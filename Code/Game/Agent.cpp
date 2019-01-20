#include "Game\Agent.hpp"
#include "Game\Game.hpp"
#include "Game\GameStates\GameState.hpp"
#include "Game\Map\Map.hpp"
#include "Game\Planner.hpp";
#include "Game\GameCommon.hpp"
#include "Game\PointOfInterest.hpp"
#include "Game\GameStates\PlayingState.hpp"
#include "Engine\Core\Transform2D.hpp"
#include "Engine\Math\MathUtils.hpp"
#include "Engine\Window\Window.hpp"
#include "Engine\Utility\AStar.hpp"
#include "Engine\Time\Stopwatch.hpp"
#include "Engine\Profiler\Profiler.hpp"
#include "Engine\File\CSVWriter.hpp"

bool isFirstLoopThroughAction = true;
Stopwatch* actionTimer = nullptr;

//  =========================================================================================
Agent::Agent()
{
}

//  =========================================================================================
Agent::Agent(Vector2 startingPosition, IsoSpriteAnimSet* animationSet, Map* mapReference)
{
	m_position = startingPosition;
	m_animationSet = animationSet;
	m_planner = new Planner(mapReference, this);

	m_intermediateGoalPosition = m_position;

	m_animationSet->SetCurrentAnim("idle");

	m_id = mapReference->m_agentsOrderedByXPosition.size();

	actionTimer = new Stopwatch(GetMasterClock());

	GenerateRandomStats();
}

//  =========================================================================================
Agent::~Agent()
{
	delete(m_planner);
	m_planner = nullptr;

	m_animationSet = nullptr;
}

//  =========================================================================================
void Agent::GenerateRandomStats()
{
	m_combatBias = GetRandomFloatInRange(0.1f, 0.9f);
	m_repairBias = GetRandomFloatInRange(0.1f, 0.9f);
	m_healBias = GetRandomFloatInRange(0.1f, 0.9f);

	m_combatEfficiency = GetRandomFloatInRange(0.25f, 0.9f);
	m_repairEfficiency = GetRandomFloatInRange(0.25f, 0.9f);
	m_healEfficiency = GetRandomFloatInRange(0.25f, 0.9f);

	UpdateCombatPerformanceTime();
	UpdateRepairPerformanceTime();
	UpdateHealPerformanceTime();
}	

//  =========================================================================================
void Agent::Update(float deltaSeconds)
{
	PROFILER_PUSH();

#ifdef AgentUpdateAnalysis
	// profiling ----------------------------------------------
	uint64_t startHPC = GetPerformanceCounter();
	++g_numAgentUpdateCalls;
	//  ----------------------------------------------
#endif

	m_planner->ProcessActionStack(deltaSeconds);	

	UpdateSpriteRenderDirection();
	m_animationSet->Update(deltaSeconds);


#ifdef AgentUpdateAnalysis
	// profiling ----------------------------------------------
	uint64_t endHPC = GetPerformanceCounter();
	g_agentUpdateData->AddCell(Stringf("%f", (float)PerformanceCounterToSeconds(endHPC - startHPC)), true);
	//  ----------------------------------------------
#endif
}

//  =============================================================================
void Agent::QuickUpdate(float deltaSeconds)
{
	PROFILER_PUSH();

	int priorityIncreaseAmount = 1;

	m_animationSet->Update(deltaSeconds);

	//if we are walking at the moment, keep walking in the same direction (physics will handle the rest)
	if (m_currentPath.size() > 0)
	{
		m_forward = m_intermediateGoalPosition - m_position;
		m_forward.NormalizeAndGetLength();

		m_position += (m_forward * (m_movespeed * GetMasterDeltaSeconds() * 0.5f));
	}

	TODO("Later add more criteria to better define this data");
	if(m_planner->GetActionStackSize() == 0)
		priorityIncreaseAmount = 10; //increase a significant amount

	IncreaseUpdatePriority(priorityIncreaseAmount);
}

//  =========================================================================================
void Agent::Render()
{
	PROFILER_PUSH();
	Renderer* theRenderer = Renderer::GetInstance();

	Sprite sprite = *m_animationSet->GetCurrentSprite(m_spriteDirection);

	float dimensions = 1.f;

	Texture* texture = sprite.GetSpriteTexture();
	Vector2 spritePivot = sprite.m_definition->m_pivot;

	IntVector2 spriteDimensions = sprite.GetDimensions();

	AABB2 bounds;
	bounds.mins.x = 0.f - (spritePivot.x) * dimensions;
	bounds.maxs.x = bounds.mins.x + 1.f * dimensions;
	bounds.mins.y = 0.f - (spritePivot.y) * dimensions;
	bounds.maxs.y = bounds.mins.y + 1.f * dimensions;

	Rgba agentColor = Rgba::WHITE;
	switch (m_planner->m_currentPlan)
	{
	case SHOOT_PLAN_TYPE:
		agentColor = Rgba::RED;
		break;
	case REPAIR_PLAN_TYPE:
		agentColor = Rgba::BLUE;
		break;
	case HEAL_PLAN_TYPE:
		agentColor = Rgba::GREEN;
		break;
	}

	theRenderer->SetShader(theRenderer->CreateOrGetShader("agents"));
	theRenderer->DrawTexturedAABB(m_transform.GetWorldMatrix(), bounds, *theRenderer->CreateOrGetTexture(sprite.m_definition->m_diffuseSource), Vector2(sprite.GetNormalizedUV().mins.x, sprite.GetNormalizedUV().maxs.y), Vector2(sprite.GetNormalizedUV().maxs.x, sprite.GetNormalizedUV().mins.y), agentColor);
	theRenderer->SetShader(theRenderer->CreateOrGetShader("default"));

	theRenderer = nullptr;
}

//  =========================================================================================
bool Agent::GetPathToDestination(const Vector2& goalDestination)
{
	++g_numGetPathCalls;

#ifdef PathingDataAnalysis
	// profiling ----------------------------------------------
	static int iterations = 0;
	static uint64_t timeAverage = 0.f;
	static uint64_t iterationStartHPC = GetPerformanceCounter();
	uint64_t startHPC = GetPerformanceCounter();

	++iterations;
	//  ----------------------------------------------
#endif

	PROFILER_PUSH();
	m_currentPath = std::vector<Vector2>(); //clear vector

	IntVector2 startCoord = m_planner->m_map->GetTileCoordinateOfPosition(m_position);
	IntVector2 endCoord = m_planner->m_map->GetTileCoordinateOfPosition(goalDestination);
	bool isDestinationFound = false;

	m_currentPath.push_back(goalDestination);

	//add the location
	if (GetIsOptimized())
	{
		isDestinationFound = AStarSearchOnGrid(m_currentPath, startCoord, endCoord, m_planner->m_map->m_mapAsGrid, m_planner->m_map);
	}		
	else
	{
		isDestinationFound = AStarSearchOnGrid(m_currentPath, startCoord, endCoord, m_planner->m_map->GetAsGrid(), m_planner->m_map);
	}

#ifdef PathingDataAnalysis
	// profiling ----------------------------------------------
	uint64_t totalHPC = GetPerformanceCounter() - startHPC;

	timeAverage = ((timeAverage * (iterations - 1)) + totalHPC) / iterations;
	if (iterations == 1)
	{
		float totalSeconds = (float)PerformanceCounterToSeconds(GetPerformanceCounter() - iterationStartHPC);
		float iterationsPerSecond = totalSeconds/100.f;
		

		float secondsAverage = (float)PerformanceCounterToSeconds(timeAverage);
		//DevConsolePrintf(Rgba::LIGHT_BLUE, "Average Time After 100 iterations (Pathing) %f", secondsAverage);
		//DevConsolePrintf(Rgba::LIGHT_BLUE, "Iterations per second %f (Pathing) (total time between: %f)", iterationsPerSecond, totalSeconds);

		g_pathingData->AddCell(Stringf("%f", secondsAverage), true);

		//g_generalSimulationData->WriteEntryWithTimeStamp(Stringf("Iterations per second %f (Pathing) (total time between: %f)", iterationsPerSecond, totalSeconds));


		//reset data
		iterationStartHPC = GetPerformanceCounter();
		iterations = 0;
		timeAverage = 0.0;
	}
	//  ----------------------------------------------
#endif

	return isDestinationFound;
}

//  =========================================================================================
bool Agent::GetIsAtPosition(const Vector2 & goalDestination)
{
	if (m_planner->m_map->GetTileCoordinateOfPosition(m_position) != m_planner->m_map->GetTileCoordinateOfPosition(goalDestination))
		return false;
	else
		return true;
}

//  =============================================================================
AABB2 Agent::GetBounds()
{
	return AABB2();
}

//  =============================================================================
void Agent::UpdateCombatPerformanceTime()
{
	if(m_combatEfficiency == 1.f)
		m_calculatedCombatPerformancePerSecond = 1.f;
	else
		m_calculatedCombatPerformancePerSecond = g_minActionPerformanceRatePerSecond / (1.f - m_combatEfficiency);
}

//  =============================================================================
void Agent::UpdateRepairPerformanceTime()
{
	if(m_repairEfficiency == 1.f)
		m_calculatedRepairPerformancePerSecond = 1.f;
	else
	m_calculatedRepairPerformancePerSecond = g_minActionPerformanceRatePerSecond / (1.f - m_repairEfficiency);
}

//  =============================================================================
void Agent::UpdateHealPerformanceTime()
{
	if(m_healEfficiency == 1.f)
		m_calculatedHealPerformancePerSecond = 1.f;
	else
	m_calculatedHealPerformancePerSecond = g_minActionPerformanceRatePerSecond / (1.f - m_healEfficiency);
}

//  =========================================================================================
void Agent::UpdateSpriteRenderDirection()
{
	PROFILER_PUSH();

	//calculate the largest dot between facing or turned away
	float dotValue = 0;
	float northDot = DotProduct(Vector2(MAP_NORTH), Vector2(m_forward));
	float southDot = DotProduct(Vector2(MAP_SOUTH), Vector2(m_forward));
	float eastDot = DotProduct(Vector2(MAP_EAST), Vector2(m_forward));
	float westDot = DotProduct(Vector2(MAP_WEST), Vector2(m_forward));

	IntVector2 direction;

	//set the direction we are most aligned with between north and south
	float highestLongitudeDot = 0;
	if (northDot > southDot)
	{
		direction.y = 1;
		highestLongitudeDot = northDot;
	}
	else
	{
		direction.y = -1;
		highestLongitudeDot = southDot;
	}
	
	//set the direction we are most aligned with between east and west
	float highestLatitudeDot = 0;
	if (eastDot > westDot)
	{
		direction.x = 1;
		highestLatitudeDot = eastDot;
	}
	else
	{
		direction.x = -1;
		highestLatitudeDot = westDot;
	}

	//compare north and south dot to the highest of the east and west dot. Whichever is higher is the direction we are most facing.
	if (highestLongitudeDot > highestLatitudeDot)
		direction.x = 0;
	else
		direction.y = 0;

	//set the final direction.
	m_spriteDirection = direction;
}
//  =========================================================================================
void Agent::TakeDamage(int damageAmount)
{
	m_health -= damageAmount;
	m_health = ClampInt(m_health, 0, 100);
}

//  =============================================================================
void Agent::IncreaseUpdatePriority(int amount)
{
	//if we overflow past max, set to max
	int tempUpdateAmount = m_updatePriority + amount;
	if (tempUpdateAmount < m_updatePriority)
	{
		m_updatePriority = INT_MAX;
	}
	else
	{
		m_updatePriority += amount;
	}
}

//  =============================================================================
void Agent::ResetPriority()
{
	m_updatePriority = 1;
}

//  =========================================================================================
//  Actions
//  =========================================================================================

bool MoveAction(Agent* agent, const Vector2& goalDestination, int interactEntityId)
{
	PROFILER_PUSH();

	//used to handle any extra logic that must occur on first loop
	if (isFirstLoopThroughAction)
	{
		//do first pass logic
		agent->m_currentPath.clear();
		agent->m_currentPathIndex = UINT8_MAX;
		isFirstLoopThroughAction = false;
	}

	UNUSED(interactEntityId);

	bool isComplete = false;

	TODO("might optimize not doing this every frame later");
	agent->m_animationSet->SetCurrentAnim("walk");

	// early outs ----------------------------------------------
	if (agent->m_planner->m_map->GetTileCoordinateOfPosition(agent->m_position) == agent->m_planner->m_map->GetTileCoordinateOfPosition(goalDestination))
	{
		//reset first loop action
		isFirstLoopThroughAction = true;
		agent->m_currentPathIndex = UINT8_MAX;
		return true;
	}
		

	//if we don't have a path to the destination or have completed our previous path, get a new path
	if (agent->m_currentPath.size() == 0 || agent->m_currentPathIndex == UINT8_MAX)
	{
		agent->GetPathToDestination(goalDestination);
		agent->m_currentPathIndex = agent->m_currentPath.size() - 1;
	}	

	//We have a path, follow it.
	if (agent->m_planner->m_map->GetTileCoordinateOfPosition(agent->m_position) != agent->m_planner->m_map->GetTileCoordinateOfPosition(agent->m_currentPath[agent->m_currentPathIndex]))
	{
		agent->m_intermediateGoalPosition = agent->m_currentPath[agent->m_currentPathIndex];

		agent->m_forward = agent->m_intermediateGoalPosition - agent->m_position;
		agent->m_forward.NormalizeAndGetLength();

		agent->m_position += (agent->m_forward * (agent->m_movespeed * GetMasterDeltaSeconds()));
	}		
	else
	{
		//if we are down to our final destination and we are in the same tile, just snap to the location in that tile
		if (agent->m_currentPathIndex == 0)
		{
			agent->m_position = agent->m_currentPath[agent->m_currentPathIndex];
			--agent->m_currentPathIndex;

			//reset first loop action
			isFirstLoopThroughAction = true;
			return true;
		}
		else
		{
			--agent->m_currentPathIndex;

			if (agent->m_currentPath.size() != 0)
				agent->m_intermediateGoalPosition = agent-> m_currentPath[agent->m_currentPathIndex];

			agent->m_forward = agent->m_intermediateGoalPosition - agent->m_position;
			agent->m_forward.NormalizeAndGetLength();

			agent->m_position += (agent->m_forward * (agent->m_movespeed * GetMasterDeltaSeconds()));
		}
	}

	agent->m_transform.SetLocalPosition(agent->m_position);

	return isComplete;
}

//  =========================================================================================
bool ShootAction(Agent* agent, const Vector2& goalDestination, int interactEntityId)
{
	PROFILER_PUSH();

	//used to handle any extra logic that must occur on first loop
	if (isFirstLoopThroughAction)
	{
		//do first pass logic
		isFirstLoopThroughAction = false;
		actionTimer->SetTimer(agent->m_calculatedCombatPerformancePerSecond);		
	}

	agent->m_animationSet->SetCurrentAnim("shoot");

	//if we are at our destination, we are ready to shoot	
	if (actionTimer->DecrementAll() > 0)
	{
		//launch arrow in agent forward
		agent->m_planner->m_map->m_threat = ClampInt(agent->m_planner->m_map->m_threat - g_baseShootDamageAmountPerPerformance, 0, g_maxThreat);
		agent->m_arrowCount--;
		agent->m_animationSet->GetCurrentAnim()->PlayFromStart();
	}	

	if (agent->m_arrowCount == 0 || agent->m_planner->m_map->m_threat == 0)
	{
		//reset first loop action
		isFirstLoopThroughAction = true;
		return true;
	}
		
	return false;
}

//  =========================================================================================
bool RepairAction(Agent* agent, const Vector2& goalDestination, int interactEntityId)
{
	PROFILER_PUSH();

	//used to handle any extra logic that must occur on first loop
	if (isFirstLoopThroughAction)
	{
		PointOfInterest* targetPoi = agent->m_planner->m_map->GetPointOfInterestById(interactEntityId);
		agent->m_forward = targetPoi->GetWorldBounds().GetCenter() - agent->m_position;
		//do first pass logic
		actionTimer->SetTimer(agent->m_calculatedRepairPerformancePerSecond);
		isFirstLoopThroughAction = false;
	}

	agent->m_animationSet->SetCurrentAnim("cast");

	//if we are at our destination, we are ready to repair
	PointOfInterest* targetPoi = agent->m_planner->m_map->GetPointOfInterestById(interactEntityId);
	
	if (actionTimer->DecrementAll() > 0)
	{
		targetPoi->m_health = ClampInt(targetPoi->m_health + g_baseRepairAmountPerPerformance, 0, 100);
		agent->m_lumberCount--;
	}

	if (agent->m_lumberCount == 0 || targetPoi->m_health == 100)
	{
		//reset first loop action
		isFirstLoopThroughAction = true;
		return true;
	}		

	return false;
}

//  =========================================================================================
bool HealAction(Agent* agent, const Vector2& goalDestination, int interactEntityId)
{
	PROFILER_PUSH();

	//used to handle any extra logic that must occur on first loop
	if (isFirstLoopThroughAction)
	{
		Agent* targetAgent = agent->m_planner->m_map->GetAgentById(interactEntityId);
		agent->m_forward = targetAgent->m_position - agent->m_position;

		//do first pass logic
		actionTimer->SetTimer(1.f);
		isFirstLoopThroughAction = false;
	}

	//reset first loop action
	isFirstLoopThroughAction = true;
	return true;	
}

//  =========================================================================================
bool GatherAction(Agent* agent, const Vector2& goalDestination, int interactEntityId)
{
	PROFILER_PUSH();

	agent->m_animationSet->SetCurrentAnim("cast");

	//if we are at our destination, we are ready to gather
	PointOfInterest* targetPoi = agent->m_planner->m_map->GetPointOfInterestById(interactEntityId);

	//confirm agent is at targetPOI accessLocation
	//if (agent->m_planner->m_map->GetTileCoordinateOfPosition(agent->m_position) != targetPoi->m_accessCoordinate)
	//{	
	//	ActionData* data = new ActionData();
	//	data->m_action = MoveAction;
	//	data->m_finalGoalPosition = targetPoi->m_map->GetWorldPositionOfMapCoordinate(targetPoi->m_accessCoordinate);
	//	data->m_interactEntityId = -1; //move actions don't have a target entity to interact with

	//	agent->m_planner->AddActionToStack(data);
	//	data = nullptr;
	//	return false;
	//}

	//if we are serving another agent or no one is assigned we either need to wait or set this agent to currently serving
	if (targetPoi->m_agentCurrentlyServing != agent)
	{
		agent->m_forward = targetPoi->GetWorldBounds().GetCenter() - agent->m_position;

		//no one is being served, we can begin acquiring resources from the poi
		if (targetPoi->m_agentCurrentlyServing == nullptr)
		{
			targetPoi->m_agentCurrentlyServing = agent;
			targetPoi->m_refillTimer->Reset();
			agent->m_animationSet->SetCurrentAnim("cast");
		}
		//another agent is being served so we need to wait
		else
		{
			//cleanup and return
			targetPoi = nullptr;
			return false;
		}
	}

	// we are ready to be served by poi ----------------------------------------------
	switch (targetPoi->m_type)
	{
	case ARMORY_POI_TYPE:
		if (targetPoi->m_refillTimer->ResetAndDecrementIfElapsed())
		{
			agent->m_arrowCount++;
		}
		if (agent->m_arrowCount == g_maxResourceCarryAmount)
		{
			//agent is served and ready to move on
			targetPoi->m_agentCurrentlyServing = nullptr;

			//cleanup
			targetPoi = nullptr;
			return true;
		}
		break;
	case LUMBERYARD_POI_TYPE:
		if (targetPoi->m_refillTimer->ResetAndDecrementIfElapsed())
		{
			agent->m_lumberCount++;
		}
		if (agent->m_lumberCount == g_maxResourceCarryAmount)
		{
			//agent is served and ready to move on
			targetPoi->m_agentCurrentlyServing = nullptr;

			//cleanup
			targetPoi = nullptr;
			return true;
		}
		break;
	case MED_STATION_POI_TYPE:
		if (targetPoi->m_refillTimer->ResetAndDecrementIfElapsed())
		{
			agent->m_bandageCount++;
		}
		if (agent->m_bandageCount == g_maxResourceCarryAmount)
		{
			//agent is served and ready to move on
			targetPoi->m_agentCurrentlyServing = nullptr;

			//cleanup
			targetPoi = nullptr;
			return true;
		}
		break;
	}		
	
	targetPoi = nullptr;
	return false;
}

