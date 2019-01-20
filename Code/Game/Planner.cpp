#include "Engine\Profiler\Profiler.hpp"
#include "Game\Planner.hpp"
#include "Game\Map\Map.hpp"
#include "Game\Agent.hpp"
#include "Game\PointOfInterest.hpp"
#include "Game\Agent.hpp"
#include "Game\GameCommon.hpp"
#include "Game\GameStates\PlayingState.hpp"

UtilityStorage* Planner::m_distanceUtilityStorage = nullptr;
UtilityStorage* Planner::m_buildingHealthUtilityStorage = nullptr;
UtilityStorage* Planner::m_agentHealthUitilityStorage = nullptr;
UtilityStorage* Planner::m_agentGatherUtilityStorage = nullptr;
UtilityStorage* Planner::m_shootUtilityStorageUtility = nullptr;

int iterationsOfUpdatePlan = 0;
uint64_t averageTimeForUpdatePlan = 0.0;

int iterationsOfQueueActions = 0;
uint64_t averageTimeForQueueActions = 0.0;

//  =========================================================================================
Planner::Planner(Map* mapReference, Agent* agentReference)
{
	m_map = mapReference;
	m_agent = agentReference;

	if (GetIsOptimized())
	{
		m_distanceUtilityStorage = new UtilityStorage(0.f, 1.f, 20.f);
		m_buildingHealthUtilityStorage = new UtilityStorage(0.f, 1.f, 20.f);
		m_agentHealthUitilityStorage = new UtilityStorage(0.f, 1.f, 20.f);
		m_agentGatherUtilityStorage = new UtilityStorage(0.f, 1.f, 20.f);
		m_shootUtilityStorageUtility = new UtilityStorage(0.f, 1.f, 20.f);
	}
	
}

//  =========================================================================================
//	Queue Management
//  =========================================================================================
Planner::~Planner()
{
	m_map = nullptr;
	m_agent = nullptr;

	if (GetIsOptimized())
	{
		delete(m_distanceUtilityStorage);
		m_distanceUtilityStorage = nullptr;

		delete(m_buildingHealthUtilityStorage);
		m_buildingHealthUtilityStorage = nullptr;

		delete(m_agentHealthUitilityStorage);
		m_agentHealthUitilityStorage = nullptr;

		delete(m_agentGatherUtilityStorage);
		m_agentGatherUtilityStorage = nullptr;

		delete(m_shootUtilityStorageUtility);
		m_shootUtilityStorageUtility = nullptr;
	}
}

//  =========================================================================================
void Planner::ProcessActionStack(float deltaSeconds)
{
	PROFILER_PUSH();
	
#ifdef ActionStackAnalysis
	// profiling ----------------------------------------------
	++g_numActionStackProcessCalls;

	static int iterations = 0;
	static uint64_t timeAverage = 0.f;
	static uint64_t iterationStartHPC = GetPerformanceCounter();
	uint64_t startHPC = GetPerformanceCounter();
	++iterations;
	//  ----------------------------------------------
#endif

	if (m_actionStack.size() == 0)
	{
		UpdatePlan();
	}

	//get action at the top of the queue
	if (m_actionStack.size() > 0)
	{
		ActionData* goal = m_actionStack.top();

		//run action
		bool isComplete = goal->m_action(m_agent, goal->m_finalGoalPosition, goal->m_interactEntityId);

		if (isComplete)
		{
			m_actionStack.pop();
		}
	}

#ifdef ActionStackAnalysis
	// profiling ----------------------------------------------
	uint64_t totalHPC = GetPerformanceCounter() - startHPC;

	timeAverage = ((timeAverage * (iterations - 1)) + totalHPC) / iterations;
	if (iterations == 1)
	{
		float totalSeconds = (float)PerformanceCounterToSeconds(GetPerformanceCounter() - iterationStartHPC);
		float iterationsPerSecond = totalSeconds / iterations;
		iterationStartHPC = GetPerformanceCounter();

		float secondsAverage = (float)PerformanceCounterToSeconds(timeAverage);
		//DevConsolePrintf(Rgba::GREEN, "Average Time After 10000 iterations (Process Action Stack) %f", secondsAverage);
		//DevConsolePrintf(Rgba::GREEN, "Iterations per second %f (Process Action Stack) (total time %f)", iterationsPerSecond, totalSeconds);

		g_processActionStackData->AddCell(Stringf("%f", secondsAverage), true);
		
		//g_generalSimulationData->WriteEntryWithTimeStamp(Stringf("Iterations per second %f (Process Action Stack) (total time between: %f)", iterationsPerSecond, totalSeconds));

		//reset data
		iterationStartHPC = GetPerformanceCounter();
		iterations = 0;
		timeAverage = 0.0;
	}
	//  ---------------------------------------------
#endif
}

//  =========================================================================================
void Planner::AddActionToStack(ActionData* actionData)
{
	m_actionStack.push(actionData);
}

//  =========================================================================================
void Planner::ClearStack()
{
	while (m_actionStack.size() > 0)
	{
		m_actionStack.pop();
	}
}

//  =========================================================================================
//  Planning
//  =========================================================================================
void Planner::UpdatePlan()
{
#ifdef UpdatePlanAnalysis
	// profiling ----------------------------------------------
	++g_numUpdatePlanCalls;

	static int iterations = 0;
	static uint64_t timeAverage = 0.f;
	static uint64_t iterationStartHPC = GetPerformanceCounter();
	uint64_t startHPC = GetPerformanceCounter();
	++iterations;
	//  ----------------------------------------------
#endif

	PROFILER_PUSH();
	ClearStack();

	//set preset to 
	ePlanTypes chosenOutcome = NONE_PLAN_TYPE;
	UtilityInfo highestUtilityInfo = GetIdleUtilityInfo();
	UtilityInfo compareUtilityInfo;

	//utility for gathering arrows
	compareUtilityInfo = GetHighestGatherArrowsUtility();
	if (m_currentPlan == GATHER_ARROWS_PLAN_TYPE  && compareUtilityInfo.utility != 0.f)
	{
		SkewCurrentPlanUtilityValue(compareUtilityInfo);
	}
	if (compareUtilityInfo.utility > highestUtilityInfo.utility)
	{
		highestUtilityInfo = compareUtilityInfo;
		chosenOutcome = GATHER_ARROWS_PLAN_TYPE;
	}
		
	//utility for gathering lumber
	compareUtilityInfo = GetHighestGatherLumberUtility();
	if (m_currentPlan == GATHER_LUMBER_PLAN_TYPE && compareUtilityInfo.utility != 0.f)
	{
		SkewCurrentPlanUtilityValue(compareUtilityInfo);
	}
	if(compareUtilityInfo.utility > highestUtilityInfo.utility)
	{
		highestUtilityInfo = compareUtilityInfo;
		chosenOutcome = GATHER_LUMBER_PLAN_TYPE;
	}

	//utility for gathering bandages
	/*compareUtilityInfo = GetHighestGatherBandagesUtility();
	if (m_currentPlan == GATHER_BANDAGES_PLAN_TYPE && compareUtilityInfo.utility != 0.f)
	{
		SkewCurrentPlanUtilityValue(compareUtilityInfo);
	}
	if(compareUtilityInfo.utility > highestUtilityInfo.utility)
	{
		highestUtilityInfo = compareUtilityInfo;
		chosenOutcome = GATHER_BANDAGES_PLAN_TYPE;
	}*/

	//utility for shooting	
	compareUtilityInfo = GetHighestShootUtility();

	if(compareUtilityInfo.utility != 0.f)
		SkewUtilityForBias(compareUtilityInfo, m_agent->m_combatBias);

	if (m_currentPlan == SHOOT_PLAN_TYPE  && compareUtilityInfo.utility != 0.f)
	{
		SkewCurrentPlanUtilityValue(compareUtilityInfo);
	}
	if(compareUtilityInfo.utility > highestUtilityInfo.utility)
	{
		highestUtilityInfo = compareUtilityInfo;
		chosenOutcome = SHOOT_PLAN_TYPE;
	}

	//utility for repairing buildings
	compareUtilityInfo = GetHighestRepairUtility();

	if(compareUtilityInfo.utility != 0.f)
		SkewUtilityForBias(compareUtilityInfo, m_agent->m_repairBias);

	if (m_currentPlan == REPAIR_PLAN_TYPE  && compareUtilityInfo.utility != 0.f)
	{
		SkewCurrentPlanUtilityValue(compareUtilityInfo);
	}
	if(compareUtilityInfo.utility > highestUtilityInfo.utility)
	{
		highestUtilityInfo = compareUtilityInfo;
		chosenOutcome = REPAIR_PLAN_TYPE;
	}

	//utility for healing agents
	/*compareUtilityInfo = GetHighestHealUtility();
	SkewUtilityForBias(compareUtilityInfo, m_agent->m_healBias);
	if (m_currentPlan == HEAL_PLAN_TYPE && compareUtilityInfo.utility != 0.f)
	{
		SkewCurrentPlanUtilityValue(compareUtilityInfo);
	}
	if(compareUtilityInfo.utility > highestUtilityInfo.utility)
	{
		highestUtilityInfo = compareUtilityInfo;
		chosenOutcome = HEAL_PLAN_TYPE;
	}*/

	m_currentPlan = chosenOutcome;

	QueueActionsFromCurrentPlan(m_currentPlan, highestUtilityInfo);

#ifdef UpdatePlanAnalysis
	// profiling ----------------------------------------------
	uint64_t totalHPC = GetPerformanceCounter() - startHPC;

	timeAverage = ((timeAverage * (iterations - 1)) + totalHPC) / iterations;
	if (iterations == 1)
	{
		float totalSeconds = (float)PerformanceCounterToSeconds(GetPerformanceCounter() - iterationStartHPC);
		float iterationsPerSecond = totalSeconds / 100.f;
		iterationStartHPC = GetPerformanceCounter();

		float secondsAverage = (float)PerformanceCounterToSeconds(timeAverage);
		//DevConsolePrintf(Rgba::GREEN, "Average Time After 100 iterations (UpdatePlan) %f", secondsAverage);
		//DevConsolePrintf(Rgba::GREEN, "Iterations per second %f (UpdatePlan) (total time %f)", iterationsPerSecond, totalSeconds);

		g_updatePlanData->AddCell(Stringf("%f", secondsAverage), true);

		//g_generalSimulationData->WriteEntryWithTimeStamp(Stringf("Iterations per second %f (UpdatePlan) (total time between: %f)", iterationsPerSecond, totalSeconds));

		//reset data
		iterationStartHPC = GetPerformanceCounter();
		iterations = 0;
		timeAverage = 0.0;
	}
	//  ---------------------------------------------
#endif
}

//  =========================================================================================
void Planner::QueueActionsFromCurrentPlan(ePlanTypes planType, const UtilityInfo& info)
{
	++g_numCopyPathCalls;

#ifdef QueueActionPathingDataAnalysis
	// profiling ----------------------------------------------
	static int iterations = 0;
	static uint64_t timeAverage = 0.f;
	static uint64_t iterationStartHPC = GetPerformanceCounter();
	uint64_t startHPC = GetPerformanceCounter();
	++iterations;
	//  ----------------------------------------------
#endif

	switch (planType)
	{
	case GATHER_ARROWS_PLAN_TYPE:
		QueueGatherArrowsAction(info);
		break;
	case GATHER_LUMBER_PLAN_TYPE:
		QueueGatherLumberAction(info);
		break;
	case GATHER_BANDAGES_PLAN_TYPE:
		QueueGatherBandagesAction(info);
		break;
	case SHOOT_PLAN_TYPE:
		QueueShootActions(info);
		break;
	case REPAIR_PLAN_TYPE:
		QueueRepairActions(info);
		break;
	case HEAL_PLAN_TYPE:
		QueueShootActions(info);
		break;
	default:
		//idle QueueIdleAction(info);
		break;
	}

	//decide if we have to queue a MoveAction
	if (!m_agent->GetIsAtPosition(info.endPosition))
	{
		ActionData* data = new ActionData();
		data->m_action = MoveAction;
		data->m_finalGoalPosition = info.endPosition;

		m_agent->m_planner->AddActionToStack(data);


		//figure out if we can skip doing an A* by borrowing someone else's path
		if (GetIsOptimized())
		{
			PROFILER_PUSH();
			bool success = FindAgentAndCopyPath(info.endPosition);
		}
		else
		{
			PROFILER_PUSH();
			m_agent->GetPathToDestination(info.endPosition);
		}		

#ifdef QueueActionPathingDataAnalysis
		// profiling ----------------------------------------------
		uint64_t totalHPC = GetPerformanceCounter() - startHPC;

		timeAverage = ((timeAverage * (iterations - 1)) + totalHPC) / iterations;
		if (iterations == 1)
		{
			float totalSeconds = (float)PerformanceCounterToSeconds(GetPerformanceCounter() - iterationStartHPC);
			float iterationsPerSecond = totalSeconds / 100.f;
			iterationStartHPC = GetPerformanceCounter();

			float secondsAverage = (float)PerformanceCounterToSeconds(timeAverage);
			//DevConsolePrintf(Rgba::GREEN, "Average Time After 100 iterations (Copy path) %f", secondsAverage);
			//DevConsolePrintf(Rgba::GREEN, "Iterations per second %f (Copy Path) (total time %f)", iterationsPerSecond, totalSeconds);

			g_queueActionPathingData->AddCell(Stringf("%f", secondsAverage), true);

			//g_generalSimulationData->WriteEntryWithTimeStamp(Stringf("Iterations per second %f (Queue Action Pathing) (total time between: %f)", iterationsPerSecond, totalSeconds));

			//reset data
			iterationStartHPC = GetPerformanceCounter();
			iterations = 0;
			timeAverage = 0.0;
		}
		//  ---------------------------------------------
#endif
	}
}

//  =============================================================================
// Queue Actions Functions
//  =============================================================================
void Planner::QueueGatherArrowsAction(const UtilityInfo& info)
{
	ActionData* gatherActionData = new ActionData();
	gatherActionData->m_action = GatherAction;
	gatherActionData->m_finalGoalPosition = info.endPosition;
	gatherActionData->m_interactEntityId = info.targetEntityId;

	m_actionStack.push(gatherActionData);
}

//  =========================================================================================
void Planner::QueueGatherLumberAction(const UtilityInfo& info)
{
	ActionData* gatherActionData = new ActionData();
	gatherActionData->m_action = GatherAction;
	gatherActionData->m_finalGoalPosition = info.endPosition;
	gatherActionData->m_interactEntityId = info.targetEntityId;

	m_actionStack.push(gatherActionData);
}

//  =========================================================================================
void Planner::QueueGatherBandagesAction(const UtilityInfo& info)
{
	ActionData* gatherActionData = new ActionData();
	gatherActionData->m_action = GatherAction;
	gatherActionData->m_finalGoalPosition = info.endPosition;
	gatherActionData->m_interactEntityId = info.targetEntityId;

	m_actionStack.push(gatherActionData);
}

//  =========================================================================================
void Planner::QueueShootActions(const UtilityInfo& info)
{
	ActionData* shootActionData = new ActionData();
	shootActionData->m_action = ShootAction;
	shootActionData->m_finalGoalPosition = info.endPosition;
	shootActionData->m_interactEntityId = info.targetEntityId;

	m_actionStack.push(shootActionData);
}

//  =========================================================================================
void Planner::QueueRepairActions(const UtilityInfo& info)
{
	ActionData* repairActionData = new ActionData();
	repairActionData->m_action = RepairAction;
	repairActionData->m_finalGoalPosition = info.endPosition;
	repairActionData->m_interactEntityId = info.targetEntityId;

	m_actionStack.push(repairActionData);
}

//  =========================================================================================
void Planner::QueueHealActions(const UtilityInfo& info)
{
	ActionData* healActionData = new ActionData();
	healActionData->m_action = HealAction;
	healActionData->m_finalGoalPosition = info.endPosition;
	healActionData->m_interactEntityId = info.targetEntityId;

	m_actionStack.push(healActionData);
}

//  =========================================================================================
// Get utilities
//  =========================================================================================
UtilityInfo Planner::GetHighestGatherArrowsUtility()
{
	UtilityInfo highestGatherArrowsUtility;

	if (GetIsOptimized())
	{
		if (m_agent->m_arrowCount == g_maxResourceCarryAmount)
		{
			return highestGatherArrowsUtility;
		}
	}

	if (m_map->m_armories.size() > 0)
	{
		for (int armoryIndex = 0; armoryIndex < (int)m_map->m_armories.size(); ++armoryIndex)
		{
			UtilityInfo infoForBuilding = GetGatherUitlityPerBuilding(m_map->m_armories[armoryIndex]);
			if (infoForBuilding.utility > highestGatherArrowsUtility.utility)
			{
				highestGatherArrowsUtility = infoForBuilding;
			}
		}
	}

	return highestGatherArrowsUtility;
}

//  =========================================================================================
UtilityInfo Planner::GetHighestGatherLumberUtility()
{
	UtilityInfo highestGatherLumberUtility;

	if (GetIsOptimized())
	{
		if (m_agent->m_lumberCount == g_maxResourceCarryAmount)
		{
			return highestGatherLumberUtility;
		}
	}

	if (m_map->m_lumberyards.size() > 0)
	{
		for (int lumberyardIndex = 0; lumberyardIndex < (int)m_map->m_lumberyards.size(); ++lumberyardIndex)
		{
			UtilityInfo infoForBuilding = GetGatherUitlityPerBuilding(m_map->m_lumberyards[lumberyardIndex]);
			if (infoForBuilding.utility > highestGatherLumberUtility.utility)
			{
				highestGatherLumberUtility = infoForBuilding;
			}
		}
	}

	return highestGatherLumberUtility;
}

//  =========================================================================================
UtilityInfo Planner::GetHighestGatherBandagesUtility()
{
	UtilityInfo highestGatherBandagesUtility;

	if (GetIsOptimized())
	{
		if (m_agent->m_bandageCount == g_maxResourceCarryAmount)
		{
			return highestGatherBandagesUtility;
		}
	}

	if (m_map->m_medStations.size() > 0)
	{
		for (int medStationIndex = 0; medStationIndex < (int)m_map->m_medStations.size(); ++medStationIndex)
		{
			UtilityInfo infoForBuilding = GetGatherUitlityPerBuilding(m_map->m_medStations[medStationIndex]);
			if (infoForBuilding.utility > highestGatherBandagesUtility.utility)
			{
				highestGatherBandagesUtility = infoForBuilding;
			}
		}
	}

	return highestGatherBandagesUtility;
}

//  =========================================================================================
UtilityInfo Planner::GetHighestShootUtility()
{
	UtilityInfo info;

	if (GetIsOptimized())
	{
		if (m_map->m_threat == 0 || m_agent->m_arrowCount == 0)
		{
			return info;
		}
	}

	Vector2 nearestWallPosition = (Vector2)GetNearestTileCoordinateOfMapEdgeFromCoordinate((IntVector2)m_agent->m_position);

	// distance to nearest wall squared ----------------------------------------------
	float distanceToBuildingSquared = GetDistanceSquared(m_agent->m_position, nearestWallPosition);

	//get max distance
	float maxDistanceSquared = GetDistanceSquared(Vector2::ZERO, Vector2(m_map->GetDimensions()));

	//normalize distance
	float normalizedDistance = distanceToBuildingSquared/maxDistanceSquared;

	//apply distance utility formula for utility value
	float distanceUtility = CalculateDistanceUtility(normalizedDistance);


	//normalized threat ----------------------------------------------
	float normalizedThreat = m_map->m_threat/g_maxThreat;

	//apply shoot utility formula for utility value
	float threatUtility = CalculateShootUtility(normalizedThreat);


	//combine distance and health utilities for final utility ----------------------------------------------
	float adjustedShootUtility = distanceUtility * threatUtility;
	info.utility = adjustedShootUtility;
	info.endPosition = nearestWallPosition;

	return info;
}

//  =========================================================================================
UtilityInfo Planner::GetHighestRepairUtility()
{
	UtilityInfo highestRepairUtility;

	if (GetIsOptimized())
	{
		if (m_agent->m_lumberCount == 0)
		{
			return highestRepairUtility;
		}
	}

	for (int buildingIndex = 0; buildingIndex < (int)m_map->m_pointsOfInterest.size(); ++buildingIndex)
	{
		UtilityInfo utilityInfoForBuilding = GetRepairUtilityPerBuilding(m_map->m_pointsOfInterest[buildingIndex]);
		if (utilityInfoForBuilding.utility > highestRepairUtility.utility)
		{
			highestRepairUtility = utilityInfoForBuilding;
		}
	}

	return highestRepairUtility;
}

//  =========================================================================================
UtilityInfo Planner::GetHighestHealUtility()
{
	UtilityInfo info;
	return info;
}

//  =============================================================================
UtilityInfo Planner::GetRepairUtilityPerBuilding(PointOfInterest* poi)
{
	UtilityInfo info;
	info.utility = 0.f;
	info.endPosition = (Vector2)poi->m_accessCoordinate;
	info.targetEntityId = poi->m_id;

	//easy out if building is at full health
	if (poi->m_health == g_maxHealth || m_agent->m_lumberCount == 0)
	{
		return info;
	}

	// distance to building squared ----------------------------------------------
	float distanceToBuildingSquared = GetDistanceSquared(m_agent->m_position, (Vector2)poi->m_accessCoordinate);
	
	//get max distance
	float maxDistanceSquared = GetDistanceSquared(Vector2::ZERO, Vector2(m_map->GetDimensions()));

	//normalize distance
	float normalizedDistance = distanceToBuildingSquared/maxDistanceSquared;

	//apply distance utility formula for utility value
	float distanceUtility = CalculateDistanceUtility(normalizedDistance);


	//building health ----------------------------------------------
	float normalizedHealth = poi->m_health/g_maxHealth;

	//apply health utility formula for utility value
	float healthUtility = CalculateBuildingHealthUtility(normalizedHealth);


	// combine distance and health utilities for final utility ----------------------------------------------
	float adjustedUtility = distanceUtility * healthUtility;
	info.utility = adjustedUtility;

	return info;
}

//  =============================================================================
UtilityInfo Planner::GetGatherUitlityPerBuilding(PointOfInterest* poi)
{
	UtilityInfo info;
	info.utility = 0.f;
	info.endPosition = (Vector2)poi->m_accessCoordinate;
	info.targetEntityId = poi->m_id;

	int inventoryCountPerType = 0;
	switch (poi->m_type)
	{
	case ARMORY_POI_TYPE:
		inventoryCountPerType = m_agent->m_arrowCount;
		break;
	case LUMBERYARD_POI_TYPE:
		inventoryCountPerType = m_agent->m_lumberCount;
		break;
	case MED_STATION_POI_TYPE:
		inventoryCountPerType = m_agent->m_bandageCount;
		break;
	}

	//easy out if we don't need to gather for this building type
	if (inventoryCountPerType == g_maxResourceCarryAmount)
	{
		return info;
	}

	// distance to building squared ----------------------------------------------
	float distanceToBuildingSquared = GetDistanceSquared(m_agent->m_position, (Vector2)poi->m_accessCoordinate);

	//get max distance
	float maxDistanceSquared = GetDistanceSquared(Vector2::ZERO, Vector2(m_map->GetDimensions()));

	//normalize distance
	float normalizedDistance = distanceToBuildingSquared/maxDistanceSquared;

	//apply distance utility formula for utility value
	float distanceUtility = CalculateDistanceUtility(normalizedDistance);


	//building health ----------------------------------------------
	float normalizedResourceAmount = inventoryCountPerType/g_maxResourceCarryAmount;

	//apply health utility formula for utility value
	float healthUtility = CalculateAgentGatherUtility(normalizedResourceAmount);


	// combine distance and health utilities for final utility ----------------------------------------------
	float adjustedUtility = distanceUtility * healthUtility;
	info.utility = adjustedUtility;

	return info;
}

//  =========================================================================================
UtilityInfo Planner::GetHealUtilityPerAgent(Agent * agent)
{
	UtilityInfo info;
	return info;
}

//  =============================================================================
UtilityInfo Planner::GetIdleUtilityInfo()
{
	UtilityInfo info;
	info.utility = CalculateIdleUtility();
	return info;
}

//  =========================================================================================
void Planner::SkewCurrentPlanUtilityValue(UtilityInfo& outInfo)
{
	outInfo.utility += g_skewForCurrentPlan;
}

//  =========================================================================================
void Planner::SkewUtilityForBias(UtilityInfo& outInfo, float biasValue)
{
	float value = RangeMapFloat(biasValue, 0.f, 1.f, 0.f, 0.05f);
	outInfo.utility += value;
}

//  =========================================================================================
//  Utility Function Calculations
//  =========================================================================================
float Planner::CalculateDistanceUtility(float normalizedDistance)
{

#ifdef DistanceMemoizationDataAnalysis
	// profiling ----------------------------------------------
	static int iterations = 0;
	static uint64_t timeAverage = 0.f;
	static uint64_t iterationStartHPC = GetPerformanceCounter();
	uint64_t startHPC = GetPerformanceCounter();
	++iterations;
	//  ----------------------------------------------
#endif

	// dynamic programming solution ----------------------------------------------
	int outIndex = 0;
	if (GetIsOptimized())
	{
		float outValue;
		if (m_distanceUtilityStorage->DoesValueExistForInput(normalizedDistance, outValue, outIndex))
		{
			return outValue;
		}			
	}
	//  ----------------------------------------------
	
	//  UTILITY FORMULA: ((1-x)^3 * 0.4f) + 0.1f = y 
	float oneMinusNormalizedDistance = 1 - normalizedDistance;
	float utility = ((oneMinusNormalizedDistance * oneMinusNormalizedDistance * oneMinusNormalizedDistance) * 0.4f) + 0.1f;


	// dynamic programming solution ----------------------------------------------
	if (GetIsOptimized())
	{
		m_distanceUtilityStorage->StoreValueForInputAtIndex(utility, outIndex);
	}
	//  ----------------------------------------------



#ifdef DistanceMemoizationDataAnalysis
	// profiling ----------------------------------------------
	uint64_t totalHPC = GetPerformanceCounter() - startHPC;

	timeAverage = ((timeAverage * (iterations - 1)) + totalHPC) / iterations;
	if (iterations == 1)
	{
		float totalSeconds = (float)PerformanceCounterToSeconds(GetPerformanceCounter() - iterationStartHPC);
		float iterationsPerSecond = totalSeconds / 100.f;
		iterationStartHPC = GetPerformanceCounter();

		float secondsAverage = (float)PerformanceCounterToSeconds(timeAverage);
		//DevConsolePrintf(Rgba::GREEN, "Average Time After 100 iterations (Copy path) %f", secondsAverage);
		//DevConsolePrintf(Rgba::GREEN, "Iterations per second %f (Copy Path) (total time %f)", iterationsPerSecond, totalSeconds);

		g_distanceMemoizationData->AddCell(Stringf("%f", secondsAverage), true);

		//g_generalSimulationData->WriteEntryWithTimeStamp(Stringf("Iterations per second %f (Distance Utility Pathing) (total time between: %f)", iterationsPerSecond, totalSeconds));

		//reset data
		iterationStartHPC = GetPerformanceCounter();
		iterations = 0;
		timeAverage = 0.0;
	}
	//  ---------------------------------------------
#endif

	return utility;
}

//  =========================================================================================
float Planner::CalculateBuildingHealthUtility(float normalizedBuildingHealth)
{
	// dynamic programming solution ----------------------------------------------
	int outIndex = 0;
	if (GetIsOptimized())
	{
		float outValue;
		if (m_buildingHealthUtilityStorage->DoesValueExistForInput(normalizedBuildingHealth, outValue, outIndex))
		{
			return outValue;
		}			
	}
	//  ----------------------------------------------

	//  UTILITY FORMULA: ((1 - x)^2x * 0.8) = y
	float oneMinusNormalizedBuildingHealth = 1.f - normalizedBuildingHealth;
	float poweredHealth = std::pow(oneMinusNormalizedBuildingHealth, 2.f * normalizedBuildingHealth);
	float utility = poweredHealth * 0.8f;

	// dynamic programming solution ----------------------------------------------
	if (GetIsOptimized())
	{
		m_buildingHealthUtilityStorage->StoreValueForInputAtIndex(utility, outIndex);
	}
	//  ----------------------------------------------

	return utility;
}

//  =============================================================================
float Planner::CalculateAgentHealthUtility(float normalizedAgentHealth)
{
	// dynamic programming solution ----------------------------------------------
	int outIndex = 0;
	if (GetIsOptimized())
	{
		float outValue;
		if (m_agentHealthUitilityStorage->DoesValueExistForInput(normalizedAgentHealth, outValue, outIndex))
		{
			return outValue;
		}			
	}
	//  ----------------------------------------------

	//  UTILITY FORMULA: ((1 - x)^2x * 0.8) = y
	float oneMinusNormalizedAgentHealth= 1.f - normalizedAgentHealth;

	float poweredHealth = std::pow(oneMinusNormalizedAgentHealth, 2.f * normalizedAgentHealth);
	float utility = poweredHealth * 0.8f;

	// dynamic programming solution ----------------------------------------------
	if (GetIsOptimized())
	{
		m_agentHealthUitilityStorage->StoreValueForInputAtIndex(utility, outIndex);
	}
	//  ----------------------------------------------

	return utility;
}

//  =============================================================================
float Planner::CalculateAgentGatherUtility(float normalizedResourceCarryAmount)
{
	// dynamic programming solution ----------------------------------------------
	int outIndex = 0;
	if (GetIsOptimized())
	{
		float outValue;
		if (m_agentGatherUtilityStorage->DoesValueExistForInput(normalizedResourceCarryAmount, outValue, outIndex))
		{
			return outValue;
		}			
	}
	//  ----------------------------------------------

	//  UTILITY FORMULA: ((1-x)^8x * 0.8) = y
	float oneMinusNormalizedGatherUtility = 1.f - normalizedResourceCarryAmount;
	
	float poweredGather = std::pow(oneMinusNormalizedGatherUtility, 8.f * normalizedResourceCarryAmount);
	float utility = poweredGather * 0.8f;

	// dynamic programming solution ----------------------------------------------
	if (GetIsOptimized())
	{
		m_agentGatherUtilityStorage->StoreValueForInputAtIndex(utility, outIndex);
	}
	//  ----------------------------------------------

	return utility;
}

//  =============================================================================
float Planner::CalculateShootUtility(float normalizedThreatUtility)
{
	// dynamic programming solution ----------------------------------------------
	int outIndex = 0;
	if (GetIsOptimized())
	{
		float outValue;
		if (m_shootUtilityStorageUtility->DoesValueExistForInput(normalizedThreatUtility, outValue, outIndex))
		{
			return outValue;
		}			
	}
	//  ----------------------------------------------

	//  UTILITY FORMULA: ((1-(1-x)^2x) * 0.8 = y
	float oneMinusNormalizedThreatUtility = 1.f - normalizedThreatUtility;

	float poweredThreat = 1.f - std::pow(oneMinusNormalizedThreatUtility, 2.f * normalizedThreatUtility);
	float utility = poweredThreat * 0.8f;

	// dynamic programming solution ----------------------------------------------
	if (GetIsOptimized())
	{
		m_shootUtilityStorageUtility->StoreValueForInputAtIndex(utility, outIndex);
	}
	//  ----------------------------------------------

	return utility;
}

//  =============================================================================
float Planner::CalculateIdleUtility()
{
	//flat rate for idle.  (very small but will always trump anything of value 0)
	return 0.001f;
}

//  =========================================================================================
//  Helpers
//  =========================================================================================
IntVector2 Planner::GetNearestTileCoordinateOfMapEdgeFromCoordinate(const IntVector2& coordinate)
{
	IntVector2 closestCoordinate = coordinate;

	//maxs of map (walkable is -2).  (assumed mins are 0,0)
	IntVector2 maxTileCoordinates = IntVector2(m_map->m_dimensions.x - 2.f, m_map->m_dimensions.y - 2.f);

	/*ex:
	the coordinate (3,6) on a map the size of (8,8) will return (3,7);
	*/

	//find the shortest distance on the X plane
	int distanceFromMaxX = maxTileCoordinates.x - coordinate.x;

	bool isDistanceFromMaxXCloser = false;
	int shortestDistanceX = coordinate.x;

	if(distanceFromMaxX < coordinate.x)
	{
		shortestDistanceX = distanceFromMaxX;
		isDistanceFromMaxXCloser = true;
	}

	//find the shortest distance on the Y plane
	int distanceFromMaxY = maxTileCoordinates.y - coordinate.y;
	bool isDistanceFromMaxYCloser = false;
	int shortestDistanceY = coordinate.y;

	if (distanceFromMaxY < coordinate.y)
	{
		shortestDistanceY = distanceFromMaxY;
		isDistanceFromMaxYCloser = true;
	}

	//compare the shortest distances on each coordinate. The one that is the closest we will move to either min or max
	//(if equal, just choose X)
	if(shortestDistanceY < shortestDistanceX)
	{
		if(isDistanceFromMaxYCloser)
			closestCoordinate.y = maxTileCoordinates.y;
		else
			closestCoordinate.y = 1;
	}
	else
	{
		if (isDistanceFromMaxXCloser)
			closestCoordinate.x = maxTileCoordinates.x;
		else
			closestCoordinate.x = 1;	
	}

	return closestCoordinate;
}

//  =========================================================================================
// Optimizations
//  =========================================================================================
bool Planner::FindAgentAndCopyPath(const Vector2& endPosition)
{
	++g_numCopyPathCalls;

	static Disc2 compareDisc = Disc2(0.f, 0.f, g_agentCopyDestinationPositionRadius);

	bool didSuccessfullyCopyMatchingAgent = false;

#ifdef CopyPathAnalysis
	// profiling ----------------------------------------------
	static int iterations = 0;
	static uint64_t timeAverage = 0.f;
	static uint64_t iterationStartHPC = GetPerformanceCounter();
	uint64_t startHPC = GetPerformanceCounter();
	++iterations;
	//  ----------------------------------------------
#endif

	Vector2 goalPosition = Vector2::ZERO;
	if (GetDoesHaveTopActionGoalPosition(goalPosition))
	{
		//search surrounding agents for similarities (most likely to be similar)
		Agent* matchingAgents[12]; //get 3 on each side from each list

		//get the 6 closest on the X plane
		matchingAgents[0] = GetAgentFromSortedList(m_agent->m_indexInSortedXList - 1, X_AGENT_SORT_TYPE);
		matchingAgents[1] = GetAgentFromSortedList(m_agent->m_indexInSortedXList - 2, X_AGENT_SORT_TYPE);
		matchingAgents[2] = GetAgentFromSortedList(m_agent->m_indexInSortedXList - 3, X_AGENT_SORT_TYPE);

		matchingAgents[3] = GetAgentFromSortedList(m_agent->m_indexInSortedXList + 1, X_AGENT_SORT_TYPE);
		matchingAgents[4] = GetAgentFromSortedList(m_agent->m_indexInSortedXList + 2, X_AGENT_SORT_TYPE);
		matchingAgents[5] = GetAgentFromSortedList(m_agent->m_indexInSortedXList + 3, X_AGENT_SORT_TYPE);

		//get 6 closest on the Y Plane
		matchingAgents[6] = GetAgentFromSortedList(m_agent->m_indexInSortedXList - 1, Y_AGENT_SORT_TYPE);
		matchingAgents[7] = GetAgentFromSortedList(m_agent->m_indexInSortedXList - 2, Y_AGENT_SORT_TYPE);
		matchingAgents[8] = GetAgentFromSortedList(m_agent->m_indexInSortedXList - 3, Y_AGENT_SORT_TYPE);

		matchingAgents[9] = GetAgentFromSortedList(m_agent->m_indexInSortedXList + 1, Y_AGENT_SORT_TYPE);
		matchingAgents[10] = GetAgentFromSortedList(m_agent->m_indexInSortedXList + 2, Y_AGENT_SORT_TYPE);
		matchingAgents[11] = GetAgentFromSortedList(m_agent->m_indexInSortedXList + 3, Y_AGENT_SORT_TYPE);


		//--------IN ORDER OF PRIORITY-----------
		//we care about their goal location
		//we care about their current task
		//we care about the distance to their current path
	
		Agent* mostResembledAgent = nullptr;
		float minDistanceSquared = m_map->GetMapDistanceSquared();
		uint8_t indexIntoMostResembledAgentsPath = UINT8_MAX;

		compareDisc.center = goalPosition;

		for (int agentIndex = 0; agentIndex < 12; ++agentIndex)
		{
			//early out if agent index is out of range OR if we've already looked over this agent
			if (matchingAgents[agentIndex] == nullptr || mostResembledAgent == matchingAgents[agentIndex])
			{
				continue;
			}				
		
			if (matchingAgents[agentIndex]->m_currentPath.size() > 0)
			{	
				Vector2 matchingAgentFinalDestinationPosition = Vector2::ZERO;
				if (!matchingAgents[agentIndex]->m_planner->GetDoesHaveTopActionGoalPosition(matchingAgentFinalDestinationPosition))
				{
					continue;
				}

				if (compareDisc.IsPointInside(matchingAgentFinalDestinationPosition))
				{
					//this agent matches our current goal location
					for (int agentPathIndex = 0; agentPathIndex < (int)matchingAgents[agentIndex]->m_currentPath.size(); ++agentPathIndex)
					{
						float distanceSquared = GetDistanceSquared(matchingAgents[agentIndex]->m_currentPath[agentPathIndex], m_agent->m_position);
						if (distanceSquared < minDistanceSquared)
						{
							mostResembledAgent = matchingAgents[agentIndex];
							minDistanceSquared = distanceSquared;
							indexIntoMostResembledAgentsPath = (uint8_t)agentPathIndex;
						}
					}
				}
			}

			//cleanup this agent so we can simply call delete at the end
			matchingAgents[agentIndex] = nullptr;
		}

		//we didn't find someone else's path to borrow
		if (mostResembledAgent != nullptr)
		{
			//copy path into this agent's path
			CopyPath(m_agent, mostResembledAgent, indexIntoMostResembledAgentsPath);
			didSuccessfullyCopyMatchingAgent = true;		
		}

		// cleanup ----------------------------------------------
		//should all be marked as nullptr by the end
		mostResembledAgent = nullptr;
	}

	//Generate our own path and queue move action
	if (!didSuccessfullyCopyMatchingAgent)
	{
		m_agent->GetPathToDestination(endPosition);
	}

#ifdef CopyPathAnalysis
	// profiling ----------------------------------------------
	uint64_t totalHPC = GetPerformanceCounter() - startHPC;

	timeAverage = ((timeAverage * (iterations - 1)) + totalHPC) / iterations;
	if (iterations == 1)
	{
		float totalSeconds = (float)PerformanceCounterToSeconds(GetPerformanceCounter() - iterationStartHPC);
		float iterationsPerSecond = totalSeconds / 100.f;
		iterationStartHPC = GetPerformanceCounter();

		float secondsAverage = (float)PerformanceCounterToSeconds(timeAverage);
		//DevConsolePrintf(Rgba::GREEN, "Average Time After 100 iterations (Copy path) %f", secondsAverage);
		//DevConsolePrintf(Rgba::GREEN, "Iterations per second %f (Copy Path) (total time %f)", iterationsPerSecond, totalSeconds);

		g_copyPathData->AddCell(Stringf("%f", secondsAverage), true);

		//g_generalSimulationData->WriteEntryWithTimeStamp(Stringf("Iterations per second %f (Copy Path) (total time between: %f)", iterationsPerSecond, totalSeconds));
		
		//reset data
		iterationStartHPC = GetPerformanceCounter();
		iterations = 0;
		timeAverage = 0.0;
	}
	//  ---------------------------------------------
#endif

	return didSuccessfullyCopyMatchingAgent;
}

//  =========================================================================================
void Planner::CopyPath(Agent* toAgent, Agent* fromAgent, uint8_t startingIndex)
{
	toAgent->m_currentPath.clear();
	toAgent->m_currentPathIndex = startingIndex;
	toAgent->m_currentPath.resize(startingIndex + 1);

	for (int pathIndex = startingIndex; pathIndex >= 0; --pathIndex)
	{
		toAgent->m_currentPath[pathIndex] = fromAgent->m_currentPath[pathIndex];
	}
}

//  =========================================================================================
Agent* Planner::GetAgentFromSortedList(uint16_t agentIndex, eAgentSortType sortType)
{
	switch (sortType)
	{
	case X_AGENT_SORT_TYPE:
		if (agentIndex < 0 || agentIndex >= (uint16_t)m_map->m_agentsOrderedByXPosition.size())
		{
			return nullptr;
		}
		return m_map->m_agentsOrderedByXPosition[agentIndex];
		break;

	case Y_AGENT_SORT_TYPE:
		if (agentIndex < 0 || agentIndex >= (uint16_t)m_map->m_agentsOrderedByYPosition.size())
		{
			return nullptr;
		}
		return m_map->m_agentsOrderedByYPosition[agentIndex];
		break;
	}
}

//  =============================================================================
bool Planner::GetDoesHaveTopActionGoalPosition(Vector2& positionOut)
{
	if (m_actionStack.size() > 0)
	{
		positionOut = m_actionStack.top()->m_finalGoalPosition;
		return true;
	}
	else
	{
		//if we don't have an action, return negative one
		return false;
	}	
}
