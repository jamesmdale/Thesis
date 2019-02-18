#include "Game\Agents\Agent.hpp"
#include "Game\Game.hpp"
#include "Game\GameStates\GameState.hpp"
#include "Game\Map\Map.hpp"
#include "Game\Agents\Planner.hpp";
#include "Game\GameCommon.hpp"
#include "Game\Pathing\ThesisAStar.hpp"
#include "Game\Entities\PointOfInterest.hpp"
#include "Game\GameStates\PlayingState.hpp"
#include "Engine\Core\Transform2D.hpp"
#include "Engine\Math\MathUtils.hpp"
#include "Engine\Window\Window.hpp"
#include "Engine\Utility\AStar.hpp"
#include "Engine\Time\Stopwatch.hpp"
#include "Engine\Profiler\Profiler.hpp"
#include "Engine\File\CSVWriter.hpp"

Map* m_agentMapReference = nullptr;
uint16 m_numAgents = 0;

//  =========================================================================================
Agent::Agent(Map* mapReference, uint16 numAgents)
{
	s_mapAgentReference = mapReference;
	s_numAgents = numAgents;

	m_agentInfo = new AgentInfo[numAgents];
	m_positionData = new PositionData[numAgents];
	m_pathData = new PathData[numAgents];
	m_personality = new Personality[numAgents];

	m_planner = new Planner[numAgents];

	m_actionData = new ActionData[numAgents];
	m_indexInSortedXList = new uint16[numAgents];
	m_indexInSortedYList = new uint16[numAgents];
	m_indexInPriorityList = new uint16[numAgents];		
}

//  =========================================================================================
Agent::~Agent()
{
	s_mapAgentReference = nullptr;
	
	delete[] m_agentInfo;
	m_agentInfo = nullptr;

	delete[] m_positionData;
	m_positionData = nullptr;

	delete[] m_pathData;
	m_pathData = nullptr;

	delete[] m_personality;
	m_personality = nullptr;

	delete[] m_planner;
	m_planner = nullptr;

	delete[] m_actionData;
	m_actionData = nullptr;

	delete[] m_indexInSortedXList;
	m_indexInSortedXList = nullptr;

	delete[] m_indexInSortedYList;
	m_indexInSortedYList = nullptr;

	delete[] m_indexInPriorityList;
	m_indexInPriorityList = nullptr;
}

//  =========================================================================================
void Agent::AddAgentData(const uint16 id, const Vector2& startingPosition, IsoSpriteAnimSet* animationSet)
{

}

//  =========================================================================================
void Agent::Initialize()
{
	InitializeAgentInfos();
	InitializePathData();
	InitializePersonalities();
	InitializeActionData();
	InitialisePlannerData();
}

//  =========================================================================================
void Agent::InitializeAgentInfos()
{
}

//  =========================================================================================
void Agent::InitializePathData()
{
}

//  =========================================================================================
void Agent::InitializePersonalities()
{
}

//  =========================================================================================
void Agent::InitializeActionData()
{
}

//  =========================================================================================
void Agent::InitialisePlannerData()
{
	m_planner[0].s_agentsPlannerReference = this;
}

//  =========================================================================================
void Agent::ClearPathData(PathData& pathData)
{
	for (int pathIndex = 0; pathIndex < pathData.m_pathCount; ++pathIndex)
	{
		pathData.m_currentPath[pathData.m_pathCount] = Vector2::ZERO;
	}
}

//  =========================================================================================
void Agent::GenerateRandomStats(Personality& personality)
{
	personality.m_combatBias = GetRandomFloatInRange(0.1f, 0.9f);
	personality.m_repairBias = GetRandomFloatInRange(0.1f, 0.9f);
	personality.m_healBias = GetRandomFloatInRange(0.1f, 0.9f);

	personality.m_combatEfficiency = GetRandomFloatInRange(0.25f, 0.9f);
	personality.m_repairEfficiency = GetRandomFloatInRange(0.25f, 0.9f);
	personality.m_healEfficiency = GetRandomFloatInRange(0.25f, 0.9f);

	UpdateCombatPerformanceTime(personality);
	UpdateRepairPerformanceTime(personality);
	UpdateHealPerformanceTime(personality);
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
bool Agent::GetPathToDestination(PositionData& positionData, PathData& pathData, const Vector2& goalDestination)
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
	ClearPathData(pathData); //clear current path

	IntVector2 startCoord = s_mapAgentReference->GetTileCoordinateOfPosition(positionData.m_position);
	IntVector2 endCoord = s_mapAgentReference->GetTileCoordinateOfPosition(goalDestination);
	bool isDestinationFound = false;

	//add final position to current path list
	pathData.m_currentPath[pathData.m_pathCount] = goalDestination;
	++pathData.m_pathCount;

	//add the location
	if (GetIsOptimized())
	{
		isDestinationFound = ThesisAStarSearchOnGrid(pathData, startCoord, endCoord, s_mapAgentReference->m_mapAsGrid);
	}		
	else
	{
		isDestinationFound = ThesisAStarSearchOnGrid(pathData, startCoord, endCoord, s_mapAgentReference->GetAsGrid());
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
bool Agent::GetIsAtPosition(PositionData& positionData, const Vector2& position)
{
	return positionData.m_physicsDisc.IsPointInside(position);
}

//  =========================================================================================
void Agent::UpdatePhysicsData(PositionData& positionData)
{
	positionData.m_physicsDisc.center = positionData.m_position;
}

//  =============================================================================
void Agent::UpdateCombatPerformanceTime(Personality& personality)
{
	personality.m_calculatedCombatPerformancePerSecond = g_minActionPerformanceRatePerSecond / (1.f - personality.m_combatEfficiency);
}

//  =============================================================================
void Agent::UpdateRepairPerformanceTime(Personality& personality)
{
	personality.m_calculatedRepairPerformancePerSecond = g_minActionPerformanceRatePerSecond / (1.f - personality.m_repairEfficiency);
}

//  =============================================================================
void Agent::UpdateHealPerformanceTime(Personality& personality)
{
	personality.m_calculatedHealPerformancePerSecond = g_minActionPerformanceRatePerSecond / (1.f - personality.m_healEfficiency);
}

//  =========================================================================================
void Agent::ConstructInformationAsText(std::vector<std::string>& outStrings)
{
	//id
	//outStrings.push_back(Stringf("ID: %i", m_id));
	//outStrings.push_back(Stringf("Health: %i", m_health));

	////biases
	//outStrings.push_back(Stringf("Biases"));
	//outStrings.push_back(Stringf("CB: %f", m_combatBias));
	//outStrings.push_back(Stringf("RB: %f", m_repairBias));
	//outStrings.push_back(Stringf("HB: %f", m_healBias));

	////efficiencies
	//outStrings.push_back(Stringf("Efficiencies"));
	//outStrings.push_back(Stringf("CE: %f", m_combatEfficiency));
	//outStrings.push_back(Stringf("RE: %f", m_repairEfficiency));
	//outStrings.push_back(Stringf("HE: %f", m_healEfficiency));

	////inventory
	//outStrings.push_back(Stringf("Inventory"));
	//outStrings.push_back(Stringf("Arrows: %i", m_arrowCount));
	//outStrings.push_back(Stringf("Bandages: %i", m_bandageCount));
	//outStrings.push_back(Stringf("Lumber: %i", m_lumberCount));

	////planner
	//outStrings.push_back(Stringf("Planner"));
	//outStrings.push_back(Stringf("Plan: %s", m_planner->GetPlanTypeAsText().c_str()));
	//outStrings.push_back(Stringf("Path Steps: %i", m_currentPath.size()));
	//outStrings.push_back(Stringf("Update Pr.: %i", m_updatePriority));
	//outStrings.push_back(Stringf("Gather Arr. Util.: %f", m_planner->m_utilityHistory.m_lastGatherArrows));
	//outStrings.push_back(Stringf("Gather Lum. Util.: %f", m_planner->m_utilityHistory.m_lastGatherLumber));
	//outStrings.push_back(Stringf("Shoot. Util.: %f", m_planner->m_utilityHistory.m_lastShoot));
	//outStrings.push_back(Stringf("Repair Util.: %f", m_planner->m_utilityHistory.m_lastGatherLumber));
	//outStrings.push_back(Stringf("Chosen Outcome.: %i", (int)m_planner->m_utilityHistory.m_chosenOutcome));
}

//  =========================================================================================
void Agent::UpdateSpriteRenderDirection(PositionData& positionData)
{
	PROFILER_PUSH();

	//calculate the largest dot between facing or turned away
	float dotValue = 0;
	float northDot = DotProduct(Vector2(MAP_NORTH), Vector2(positionData.m_forward));
	float southDot = DotProduct(Vector2(MAP_SOUTH), Vector2(positionData.m_forward));
	float eastDot = DotProduct(Vector2(MAP_EAST), Vector2(positionData.m_forward));
	float westDot = DotProduct(Vector2(MAP_WEST), Vector2(positionData.m_forward));

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
	positionData.m_spriteDirection = direction;
}
//  =========================================================================================
void Agent::TakeDamage(AgentInfo& agentInfo, int damageAmount)
{
	agentInfo.m_health -= damageAmount;
	agentInfo.m_health = ClampInt(agentInfo.m_health, 0, 100);
}

//  =============================================================================
void Agent::IncreaseUpdatePriority(AgentInfo& agentInfo, int amount)
{
	//if we overflow past max, set to max
	int tempUpdateAmount = agentInfo.m_updatePriority + amount;
	if (tempUpdateAmount < agentInfo.m_updatePriority)
	{
		agentInfo.m_updatePriority = INT_MAX;
	}
	else
	{
		agentInfo.m_updatePriority += amount;
	}
}

//  =============================================================================
void Agent::ResetPriority(AgentInfo& agentInfo)
{
	agentInfo.m_updatePriority = 1;
}

//  =========================================================================================
//  Actions
//  =========================================================================================

bool MoveAction(Agent* agentsList, const uint16 agentIndex, const Vector2& goalDestination, int interactEntityId)
{
	PROFILER_PUSH();
	
	//get references to the necessary data
	PositionData& positionData = agentsList->m_positionData[agentIndex];
	PathData& pathData = agentsList->m_pathData[agentIndex];
	ActionData& actionData = agentsList->m_actionData[agentIndex];

	//used to handle any extra logic that must occur on first loop
	if (actionData.m_isFirstLoopThroughAction)
	{
		//do first pass logic
		Agent::ClearPathData(pathData);
		pathData.m_currentPathIndex = UINT8_MAX;
		actionData.m_isFirstLoopThroughAction = false;
	}

	UNUSED(interactEntityId);

	TODO("might optimize not doing this every frame later");
	actionData.m_animationSet->SetCurrentAnim("walk");

	// early out
	if (Agent::GetIsAtPosition(positionData, goalDestination))
	{
		//reset first loop action
		actionData.m_isFirstLoopThroughAction = true;
		pathData.m_currentPathIndex = UINT8_MAX;
		Agent::UpdatePhysicsData(positionData);
		return true;
	}		

	//if we don't have a path to the destination or have completed our previous path, get a new path
	if (pathData.m_pathCount == 0 || pathData.m_currentPathIndex == UINT8_MAX)
	{
		Agent::GetPathToDestination(positionData, pathData, goalDestination);
		pathData.m_currentPathIndex = pathData.m_pathCount - 1;
	}	

	//We have a path, follow it.
	if(!Agent::GetIsAtPosition(positionData, pathData.m_currentPath[pathData.m_currentPathIndex]))
	{
		pathData.m_intermediateGoalPosition = pathData.m_currentPath[pathData.m_currentPathIndex];

		positionData.m_forward = pathData.m_intermediateGoalPosition - positionData.m_position;
		positionData.m_forward.NormalizeAndGetLength();

		positionData.m_position += (positionData.m_forward * (positionData.m_movespeed * GetGameClock()->GetDeltaSeconds()));
		
	}		
	else
	{
		//if we are down to our final destination and we are in the same tile, just snap to the location in that tile
		if (pathData.m_currentPathIndex == 0)
		{
			positionData.m_position = pathData.m_currentPath[pathData.m_currentPathIndex];
			--pathData.m_currentPathIndex;

			//reset first loop action
			actionData.m_isFirstLoopThroughAction = true;
			Agent::UpdatePhysicsData(positionData);
			return true;
		}
		else
		{
			--pathData.m_currentPathIndex;

			if (pathData.m_pathCount != 0)
				pathData.m_intermediateGoalPosition = pathData.m_currentPath[pathData.m_currentPathIndex];

			positionData.m_forward = pathData.m_intermediateGoalPosition - positionData.m_position;
			positionData.m_forward.NormalizeAndGetLength();

			positionData.m_position += (positionData.m_forward * (positionData.m_movespeed * GetGameClock()->GetDeltaSeconds()));
			Agent::UpdatePhysicsData(positionData);
		}
	}

	//we aren't finish moving
	return false;
}

//  =========================================================================================
bool ShootAction(Agent* agentsList, const uint16 agentIndex, const Vector2& goalDestination, int interactEntityId)
{
	PROFILER_PUSH();

	PathData& pathData = agentsList->m_pathData[agentIndex];
	Personality& personality = agentsList->m_personality[agentIndex];
	ActionData& actionData = agentsList->m_actionData[agentIndex];

	//used to handle any extra logic that must occur on first loop
	if (actionData.m_isFirstLoopThroughAction)
	{
		//do first pass logic
		actionData.m_isFirstLoopThroughAction = false;
		actionData.m_actionTimer->SetTimer(personality.m_calculatedCombatPerformancePerSecond);		
	}

	actionData.m_animationSet->SetCurrentAnim("shoot");

	//if we are at our destination, we are ready to shoot	
	if (actionData.m_actionTimer->DecrementAll() > 0)
	{
		//launch arrow in agent forward
		m_agentMapReference->m_threat = ClampInt(m_agentMapReference->m_threat - g_baseShootDamageAmountPerPerformance, 0, g_maxThreat);
		actionData.m_arrowCount--;

		ASSERT_OR_DIE(actionData.m_arrowCount >= 0, "AGENT ARROW COUNT NEGATIVE!!");

		actionData.m_animationSet->GetCurrentAnim()->PlayFromStart();
	}	

	if (actionData.m_arrowCount == 0 || m_agentMapReference->m_threat == 0)
	{
		//reset first loop action
		actionData.m_isFirstLoopThroughAction = true;
		return true;
	}
		
	return false;
}

//  =========================================================================================
bool RepairAction(Agent* agentsList, const uint16 agentIndex, const Vector2& goalDestination, int interactEntityId)
{
	PROFILER_PUSH();

	PositionData& positionData = agentsList->m_positionData[agentIndex];
	Personality& personality = agentsList->m_personality[agentIndex];
	ActionData& actionData = agentsList->m_actionData[agentIndex];

	//used to handle any extra logic that must occur on first loop
	if (actionData.m_isFirstLoopThroughAction)
	{
		PointOfInterest* targetPoi = m_agentMapReference->GetPointOfInterestById(interactEntityId);
		positionData.m_forward = targetPoi->GetWorldBounds().GetCenter() - positionData.m_position;
		//do first pass logic
		actionData.m_actionTimer->SetTimer(personality.m_calculatedRepairPerformancePerSecond);
		actionData.m_isFirstLoopThroughAction = false;
	}

	actionData.m_animationSet->SetCurrentAnim("cast");

	//if we are at our destination, we are ready to repair
	PointOfInterest* targetPoi = m_agentMapReference->GetPointOfInterestById(interactEntityId);
	
	if (actionData.m_actionTimer->DecrementAll() > 0)
	{
		targetPoi->m_health = ClampInt(targetPoi->m_health + g_baseRepairAmountPerPerformance, 0, 100);
		actionData.m_lumberCount--;

		ASSERT_OR_DIE(actionData.m_lumberCount >= 0, "AGENT LUMBER COUNT NEGATIVE!!");
	}

	if (actionData.m_lumberCount == 0 || targetPoi->m_health == 100)
	{
		//reset first loop action
		actionData.m_isFirstLoopThroughAction = true;
		return true;
	}		

	return false;
}

//  =========================================================================================
bool HealAction(Agent* agentsList, const uint16 agentIndex, const Vector2& goalDestination, int interactEntityId)
{
	PROFILER_PUSH();

	AgentInfo& agentInfo = agentsList->m_agentInfo[agentIndex];
	PositionData& positionData = agentsList->m_positionData[agentIndex];
	Personality& personality = agentsList->m_personality[agentIndex];
	ActionData& actionData = agentsList->m_actionData[agentIndex];

	//used to handle any extra logic that must occur on first loop
	if (actionData.m_isFirstLoopThroughAction)
	{
		//do first pass logic
		actionData.m_actionTimer->SetTimer(personality.m_calculatedHealPerformancePerSecond);
		actionData.m_isFirstLoopThroughAction = false;
	}

	actionData.m_animationSet->SetCurrentAnim("heal");

	if (actionData.m_actionTimer->DecrementAll() > 0)
	{
		agentInfo.m_health = ClampInt(agentInfo.m_health + g_baseHealAmountPerPerformance, 0, 100);
		actionData.m_bandageCount--;

		ASSERT_OR_DIE(actionData.m_bandageCount >= 0, "AGENT BANDAGE COUNT NEGATIVE!!");
	}

	if (actionData.m_bandageCount == 0 || agentInfo.m_health == 100)
	{
		//reset first loop action
		actionData.m_isFirstLoopThroughAction = true;
		return true;
	}		

	//reset first loop action
	actionData.m_isFirstLoopThroughAction = true;
	return true;	
}

//  =========================================================================================
bool GatherAction(Agent* agentsList, const uint16 agentIndex, const Vector2& goalDestination, int interactEntityId)
{
	PROFILER_PUSH();

	AgentInfo& agentInfo = agentsList->m_agentInfo[agentIndex];
	PositionData& positionData = agentsList->m_positionData[agentIndex];
	Personality& personality = agentsList->m_personality[agentIndex];
	ActionData& actionData = agentsList->m_actionData[agentIndex];

	actionData.m_animationSet->SetCurrentAnim("cast");

	//if we are at our destination, we are ready to gather
	PointOfInterest* targetPoi = m_agentMapReference->GetPointOfInterestById(interactEntityId);

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
	if (targetPoi->m_agentCurrentlyServingIndex != agentIndex)
	{
		positionData.m_forward = targetPoi->GetWorldBounds().GetCenter() - positionData.m_position;

		//no one is being served, we can begin acquiring resources from the poi
		if (targetPoi->m_agentCurrentlyServingIndex == UINT16_MAX)
		{
			targetPoi->m_agentCurrentlyServingIndex = agentIndex;
			targetPoi->m_refillTimer->Reset();
			actionData.m_animationSet->SetCurrentAnim("cast");
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
			actionData.m_arrowCount++;
		}
		if (actionData.m_arrowCount == g_maxResourceCarryAmount)
		{
			//agent is served and ready to move on
			targetPoi->m_agentCurrentlyServingIndex = UINT16_MAX;

			//cleanup
			targetPoi = nullptr;
			return true;
		}
		break;
	case LUMBERYARD_POI_TYPE:
		if (targetPoi->m_refillTimer->ResetAndDecrementIfElapsed())
		{
			actionData.m_lumberCount++;
		}
		if (actionData.m_lumberCount == g_maxResourceCarryAmount)
		{
			//agent is served and ready to move on
			targetPoi->m_agentCurrentlyServingIndex = UINT16_MAX;

			//cleanup
			targetPoi = nullptr;
			return true;
		}
		break;
	case MED_STATION_POI_TYPE:
		if (targetPoi->m_refillTimer->ResetAndDecrementIfElapsed())
		{
			actionData.m_bandageCount++;
		}
		if (actionData.m_bandageCount == g_maxResourceCarryAmount)
		{
			//agent is served and ready to move on
			targetPoi->m_agentCurrentlyServingIndex = UINT16_MAX;

			//cleanup
			targetPoi = nullptr;
			return true;
		}
		break;
	}		
	
	targetPoi = nullptr;
	return false;
}

