#include "Game\Agents\Agent.hpp"
#include "Game\Game.hpp"
#include "Game\GameStates\GameState.hpp"
#include "Game\Map\Map.hpp"
#include "Game\Agents\Planner.hpp";
#include "Game\GameCommon.hpp"
#include "Game\Entities\PointOfInterest.hpp"
#include "Game\GameStates\PlayingState.hpp"
#include "Game\Entities\Fire.hpp"
#include "Engine\Core\Transform2D.hpp"
#include "Engine\Math\MathUtils.hpp"
#include "Engine\Window\Window.hpp"
#include "Engine\Utility\AStar.hpp"
#include "Engine\Time\Stopwatch.hpp"
#include "Engine\Profiler\Profiler.hpp"
#include "Engine\File\CSVWriter.hpp"

//  =========================================================================================
Agent::Agent()
{
}

//  =========================================================================================
Agent::Agent(Vector2 startingPosition, IsoSpriteAnimSet* animationSet, Map* mapReference)
{
	//set id according to how many we've made
	m_id = mapReference->m_agentsOrderedByXPosition.size();

	//setup position and planner
	m_position = startingPosition;
	m_planner = new Planner(mapReference, this);
	m_intermediateGoalPosition = m_position;
	m_actionTimer = new Stopwatch(GetGameClock());

	//generate personality
	GenerateRandomStats();
	
	//setup physics
	m_physicsDisc.radius = 0.3f;
	m_physicsDisc.center = m_position;

	//precompute sprite data
	m_animationSet = animationSet;
	m_animationSet->SetCurrentAnim("idle");
	Sprite sprite = *m_animationSet->GetCurrentSprite(m_spriteDirection);
	Vector2 spritePivot = sprite.m_definition->m_pivot;
	IntVector2 spriteDimensions = sprite.GetDimensions();

	//m_spriteRenderBounds.mins.x = 0.f - (spritePivot.x) * 1.f;
	//m_spriteRenderBounds.maxs.x = m_spriteRenderBounds.mins.x + 1.f * 1.f;
	//m_spriteRenderBounds.mins.y = 0.f - (spritePivot.y) * 1.f;
	//m_spriteRenderBounds.maxs.y = m_spriteRenderBounds.mins.y + 1.f * 1.f;

	m_positionStuckCheckTimer = new Stopwatch(GetGameClock());
	m_positionStuckCheckTimer->SetTimer(g_agentOldPositionRefreshRate);
}

//  =========================================================================================
Agent::~Agent()
{
	delete(m_planner);
	m_planner = nullptr;

	delete(m_actionTimer);
	m_actionTimer = nullptr;

	delete(m_positionStuckCheckTimer);
	m_positionStuckCheckTimer = nullptr;

	m_animationSet = nullptr;
}

//  =========================================================================================
void Agent::GenerateRandomStats()
{
	m_combatBias = GetRandomFloatInRange(0.1f, 0.9f);
	m_repairBias = GetRandomFloatInRange(0.1f, 0.9f);
	m_healBias = GetRandomFloatInRange(0.1f, 0.9f);
	m_fireFightingBias = GetRandomFloatInRange(0.1f, 0.9f);

	m_combatEfficiency = GetRandomFloatInRange(0.25f, 0.9f);
	m_repairEfficiency = GetRandomFloatInRange(0.25f, 0.9f);
	m_healEfficiency = GetRandomFloatInRange(0.25f, 0.9f);
	m_fireFightingEfficiency = GetRandomFloatInRange(0.25f, 0.9f);

	UpdateCombatPerformanceTime();
	UpdateRepairPerformanceTime();
	UpdateHealPerformanceTime();
	UpdateFireFightingPerformanceTime();
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

		//move even slower if we can't get here
		m_position += (m_forward * (m_movespeed * GetGameClock()->GetDeltaSeconds() * 0.1f));
	}

	TODO("Later add more criteria to better define this data");
	if(m_planner->GetActionStackSize() == 0)
		priorityIncreaseAmount = 10; //increase a significant amount

	IncreaseUpdatePriority(priorityIncreaseAmount);
}

//  =========================================================================================
void Agent::Render()
{
	/*PROFILER_PUSH();
	Renderer* theRenderer = Renderer::GetInstance();

	Sprite sprite = *m_animationSet->GetCurrentSprite(m_spriteDirection);
	Texture* texture = sprite.GetSpriteTexture();

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
	theRenderer->DrawTexturedAABB(m_transform.GetWorldMatrix(), m_spriteRenderBounds, *theRenderer->CreateOrGetTexture(sprite.m_definition->m_diffuseSource), Vector2(sprite.GetNormalizedUV().mins.x, sprite.GetNormalizedUV().maxs.y), Vector2(sprite.GetNormalizedUV().maxs.x, sprite.GetNormalizedUV().mins.y), agentColor);
	theRenderer->SetShader(theRenderer->CreateOrGetShader("default"));

	theRenderer = nullptr;*/
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
		Grid<int> mapGrid;
		m_planner->m_map->GetAsGrid(mapGrid);

		isDestinationFound = AStarSearchOnGrid(m_currentPath, startCoord, endCoord, &mapGrid, m_planner->m_map);
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

////  =========================================================================================
//bool Agent::GetIsAtPosition(const Vector2 & goalDestination)
//{
//	if (m_planner->m_map->GetTileCoordinateOfPosition(m_position) != m_planner->m_map->GetTileCoordinateOfPosition(goalDestination))
//		return false;
//	else
//		return true;
//}

//  =========================================================================================
bool Agent::GetIsAtPosition(const Vector2& position)
{
	return m_physicsDisc.IsPointInside(position);
}

//  =========================================================================================
void Agent::UpdatePhysicsData()
{
	m_physicsDisc.center = m_position;
}

//  =========================================================================================
bool Agent::IsHazardAhead()
{
	Map* map = m_planner->m_map;

	Vector2 positionAhead = m_position + m_forward;
	Tile* forwardTile = map->GetTileAtWorldPosition(positionAhead);
	if(forwardTile->m_tileDefinition->m_name == "Fire")
		return true;

	return false;
}

//  =============================================================================
void Agent::UpdateCombatPerformanceTime()
{
		m_calculatedCombatPerformancePerSecond = g_minActionPerformanceRatePerSecond / (1.f - m_combatEfficiency);
}

//  =============================================================================
void Agent::UpdateRepairPerformanceTime()
{
	m_calculatedRepairPerformancePerSecond = g_minActionPerformanceRatePerSecond / (1.f - m_repairEfficiency);
}

//  =============================================================================
void Agent::UpdateHealPerformanceTime()
{
	m_calculatedHealPerformancePerSecond = g_minActionPerformanceRatePerSecond / (1.f - m_healEfficiency);
}

//  =========================================================================================
void Agent::UpdateFireFightingPerformanceTime()
{
	m_calculatedFireFightingPerformancePerSecond = g_minActionPerformanceRatePerSecond / (1.f - m_fireFightingEfficiency);
}

//  =========================================================================================
void Agent::ConstructInformationAsText(std::vector<std::string>& outStrings)
{
	//id
	outStrings.push_back(Stringf("ID: %i", m_id));
	outStrings.push_back(Stringf("Health: %i", m_health));

	//biases
	outStrings.push_back(Stringf("Biases"));
	outStrings.push_back(Stringf("CB: %f", m_combatBias));
	outStrings.push_back(Stringf("RB: %f", m_repairBias));
	outStrings.push_back(Stringf("HB: %f", m_healBias));
	outStrings.push_back(Stringf("FB: %f", m_fireFightingBias));

	//efficiencies
	outStrings.push_back(Stringf("Efficiencies"));
	outStrings.push_back(Stringf("CE: %f", m_combatEfficiency));
	outStrings.push_back(Stringf("RE: %f", m_repairEfficiency));
	outStrings.push_back(Stringf("HE: %f", m_healEfficiency));
	outStrings.push_back(Stringf("FE: %f", m_fireFightingEfficiency));

	//inventory
	outStrings.push_back(Stringf("Inventory"));
	outStrings.push_back(Stringf("Arrows: %i", m_arrowCount));
	outStrings.push_back(Stringf("Bandages: %i", m_bandageCount));
	outStrings.push_back(Stringf("Lumber: %i", m_lumberCount));
	outStrings.push_back(Stringf("Water: %i", m_waterCount));

	//planner
	outStrings.push_back(Stringf("Planner"));
	outStrings.push_back(Stringf("Plan: %s", m_planner->GetPlanTypeAsText().c_str()));
	outStrings.push_back(Stringf("Path Steps: %i", m_currentPath.size()));
	outStrings.push_back(Stringf("Update Pr.: %i", m_updatePriority));
	outStrings.push_back(Stringf("Gather Arr. Util.: %f", m_planner->m_utilityHistory.m_lastGatherArrows));
	outStrings.push_back(Stringf("Gather Lum. Util.: %f", m_planner->m_utilityHistory.m_lastGatherLumber));
	outStrings.push_back(Stringf("Gather Ban. Util.: %f", m_planner->m_utilityHistory.m_lastGatherBandages));
	outStrings.push_back(Stringf("Gather Wat. Util.: %f", m_planner->m_utilityHistory.m_lastGatherWater));
	outStrings.push_back(Stringf("Shoot. Util.: %f", m_planner->m_utilityHistory.m_lastShoot));
	outStrings.push_back(Stringf("Repair Util.: %f", m_planner->m_utilityHistory.m_lastRepair));
	outStrings.push_back(Stringf("Heal Util.: %f", m_planner->m_utilityHistory.m_lastHeal));
	outStrings.push_back(Stringf("Fire Util.: %f", m_planner->m_utilityHistory.m_lastFightFire));
	outStrings.push_back(Stringf("Chosen Outcome.: %i", (int)m_planner->m_utilityHistory.m_chosenOutcome));

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
void Agent::ResetPlannerAndPathing()
{
	m_planner->ClearActionStack();
	ClearCurrentPath();
}

//  =========================================================================================
void Agent::ClearCurrentPath()
{
	m_currentPath.clear();
	m_currentPathIndex = UINT8_MAX;
}

//  =========================================================================================
//  Actions
//  =========================================================================================

bool MoveAction(Agent* agent, const Vector2& goalDestination, int interactEntityId)
{
	PROFILER_PUSH();

	//used to handle any extra logic that must occur on first loop
	if (agent->m_isFirstLoopThroughAction)
	{
		//do first pass logic
		agent->ClearCurrentPath();
		agent->m_isFirstLoopThroughAction = false;
		agent->m_oldPosition = agent->m_position;

		agent->m_positionStuckCheckTimer->Reset();
	}

	//catch scenarios where agent is stuck here
	if (agent->m_positionStuckCheckTimer->ResetAndDecrementIfElapsed())
	{
		//if(agent->m_physicsDisc.IsPointInside(agent->m_oldPosition))
		if( FloorPosition(agent->m_oldPosition) == FloorPosition(agent->m_position))
		{
			agent->ClearCurrentPath();
		}

		agent->m_oldPosition = agent->m_position;
	}

	UNUSED(interactEntityId);

	TODO("might optimize not doing this every frame later");
	agent->m_animationSet->SetCurrentAnim("walk");

	// early out
	if (agent->GetIsAtPosition(goalDestination))
	{
		//reset first loop action
		agent->m_isFirstLoopThroughAction = true;
		agent->m_currentPathIndex = UINT8_MAX;
		agent->UpdatePhysicsData();
		return true;
	}		
	
	//look ahead a tile to make sure we aren't about to walk into fire
	if(agent->IsHazardAhead())
	{
		//by reseting loop in action, we will reset the path
		/*agent->m_isFirstLoopThroughAction = true;
		agent->UpdatePhysicsData();
		return false;*/
		agent->ClearCurrentPath();
	}

	//if we don't have a path to the destination or have completed our previous path, get a new path
	if (agent->m_currentPath.size() == 0 || agent->m_currentPathIndex == UINT8_MAX)
	{
		agent->GetPathToDestination(goalDestination);
		agent->m_currentPathIndex = (uint8)agent->m_currentPath.size() - 1;
	}	

	//We have a path, follow it.
	if(!agent->GetIsAtPosition(agent->m_currentPath[agent->m_currentPathIndex]))
	{
		agent->m_intermediateGoalPosition = agent->m_currentPath[agent->m_currentPathIndex];

		agent->m_forward = agent->m_intermediateGoalPosition - agent->m_position;
		agent->m_forward.NormalizeAndGetLength();

		agent->m_position += (agent->m_forward * (agent->m_movespeed * GetGameClock()->GetDeltaSeconds()));
		agent->UpdatePhysicsData();
	}		
	else
	{
		//if we are down to our final destination and we are in the same tile, just snap to the location in that tile
		if (agent->m_currentPathIndex == 0)
		{
			agent->m_position = agent->m_currentPath[agent->m_currentPathIndex];
			--agent->m_currentPathIndex;

			//reset first loop action
			agent->m_isFirstLoopThroughAction = true;
			agent->UpdatePhysicsData();
			return true;
		}
		else
		{
			--agent->m_currentPathIndex;

			if (agent->m_currentPath.size() != 0)
				agent->m_intermediateGoalPosition = agent-> m_currentPath[agent->m_currentPathIndex];

			agent->m_forward = agent->m_intermediateGoalPosition - agent->m_position;
			agent->m_forward.NormalizeAndGetLength();

			agent->m_position += (agent->m_forward * (agent->m_movespeed * GetGameClock()->GetDeltaSeconds()));
			agent->UpdatePhysicsData();
		}
	}

	//we aren't finish moving
	return false;
}

//  =========================================================================================
bool ShootAction(Agent* agent, const Vector2& goalDestination, int interactEntityId)
{
	PROFILER_PUSH();

	//used to handle any extra logic that must occur on first loop
	if (agent->m_isFirstLoopThroughAction)
	{
		//do first pass logic
		agent->m_isFirstLoopThroughAction = false;
		agent->m_actionTimer->SetTimer(agent->m_calculatedCombatPerformancePerSecond);		
	}

	agent->m_animationSet->SetCurrentAnim("shoot");

	//if we are at our destination, we are ready to shoot	
	if (agent->m_actionTimer->DecrementAll() > 0)
	{
		//launch arrow in agent forward
		agent->m_planner->m_map->m_threat = ClampInt(agent->m_planner->m_map->m_threat - g_baseShootDamageAmountPerPerformance, 0, g_maxThreat);
		agent->m_arrowCount--;

		ASSERT_OR_DIE(agent->m_arrowCount >= 0, "AGENT ARROW COUNT NEGATIVE!!");

		agent->m_animationSet->GetCurrentAnim()->PlayFromStart();
	}	

	if (agent->m_arrowCount == 0 || agent->m_planner->m_map->m_threat == 0)
	{
		//reset first loop action
		agent->m_isFirstLoopThroughAction = true;
		return true;
	}
		
	return false;
}

//  =========================================================================================
bool RepairAction(Agent* agent, const Vector2& goalDestination, int interactEntityId)
{
	PROFILER_PUSH();

	//used to handle any extra logic that must occur on first loop
	if (agent->m_isFirstLoopThroughAction)
	{
		PointOfInterest* targetPoi = agent->m_planner->m_map->GetPointOfInterestById(interactEntityId);
		agent->m_forward = targetPoi->GetWorldBounds().GetCenter() - agent->m_position;
		//do first pass logic
		agent->m_actionTimer->SetTimer(agent->m_calculatedRepairPerformancePerSecond);
		agent->m_isFirstLoopThroughAction = false;
	}

	agent->m_animationSet->SetCurrentAnim("cast");

	//if we are at our destination, we are ready to repair
	PointOfInterest* targetPoi = agent->m_planner->m_map->GetPointOfInterestById(interactEntityId);
	
	if (agent->m_actionTimer->DecrementAll() > 0)
	{
		targetPoi->m_health = ClampInt(targetPoi->m_health + g_baseRepairAmountPerPerformance, 0, 100);
		agent->m_lumberCount--;

		ASSERT_OR_DIE(agent->m_lumberCount >= 0, "AGENT LUMBER COUNT NEGATIVE!!");
	}

	if (agent->m_lumberCount == 0 || targetPoi->m_health == 100)
	{
		//reset first loop action
		agent->m_isFirstLoopThroughAction = true;
		return true;
	}		

	return false;
}

//  =========================================================================================
bool HealAction(Agent* agent, const Vector2& goalDestination, int interactEntityId)
{
	PROFILER_PUSH();

	//used to handle any extra logic that must occur on first loop
	if (agent->m_isFirstLoopThroughAction)
	{
		//do first pass logic
		agent->m_actionTimer->SetTimer(agent->m_calculatedHealPerformancePerSecond);
		agent->m_isFirstLoopThroughAction = false;
	}

	agent->m_animationSet->SetCurrentAnim("heal");

	if (agent->m_actionTimer->DecrementAll() > 0)
	{
		agent->m_health = ClampInt(agent->m_health + g_baseHealAmountPerPerformance, 0, 100);
		agent->m_bandageCount--;

		ASSERT_OR_DIE(agent->m_bandageCount >= 0, "AGENT BANDAGE COUNT NEGATIVE!!");
	}

	if (agent->m_bandageCount == 0 || agent->m_health == 100)
	{
		//reset first loop action
		agent->m_isFirstLoopThroughAction = true;
		return true;
	}		

	//reset first loop action
	agent->m_isFirstLoopThroughAction = true;
	return true;	
}

//  =========================================================================================
bool FightFireAction(Agent* agent, const Vector2& goalDestination, int interactEntityId)
{
	PROFILER_PUSH();

	Fire* targetFire = agent->m_planner->m_map->GetFireById(interactEntityId);
	if (targetFire == nullptr)
	{
		agent->m_isFirstLoopThroughAction = true;
		return true;
	}

	//used to handle any extra logic that must occur on first loop
	if (agent->m_isFirstLoopThroughAction)
	{
		//do first pass logic
		agent->m_forward = targetFire->m_worldPosition - agent->m_position;	
		agent->m_actionTimer->SetTimer(agent->m_calculatedRepairPerformancePerSecond);
		agent->m_isFirstLoopThroughAction = false;
	}

	agent->m_animationSet->SetCurrentAnim("heal");

	//if we are at our destination, we are ready to fight the fire
	if (agent->m_actionTimer->DecrementAll() > 0)
	{
		targetFire->m_health = ClampInt(targetFire->m_health - g_baseFireFightingAmountPerPerformance, 0, g_maxFireHealth);
		agent->m_waterCount--;

		ASSERT_OR_DIE(agent->m_waterCount >= 0, "AGENT WATER COUNT NEGATIVE!!");
	}

	if (agent->m_waterCount == 0 || targetFire->m_health == 0)
	{
		//reset first loop action
		agent->m_isFirstLoopThroughAction = true;
		return true;
	}		

	return false;
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

		case WELL_POI_TYPE:
			if (targetPoi->m_refillTimer->ResetAndDecrementIfElapsed())
			{
				agent->m_waterCount++;
			}
			if (agent->m_waterCount == g_maxResourceCarryAmount)
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

