#pragma once
#include "Engine\Math\IntVector2.hpp"
#include "Game\UtilityHelpers\UtilityStorage.hpp"
#include <stack>

struct ActionCallbackData;
class Agent;
struct AgentInfo;
struct PositionData;
struct PathData;
struct Personality;
struct ActionData;

class Map;
class PointOfInterest;
enum eAgentSortType;

enum ePlanTypes
{
	NONE_PLAN_TYPE,
	GATHER_ARROWS_PLAN_TYPE,
	GATHER_LUMBER_PLAN_TYPE,
	GATHER_BANDAGES_PLAN_TYPE,
	SHOOT_PLAN_TYPE,
	REPAIR_PLAN_TYPE,
	HEAL_PLAN_TYPE,
	NUM_PLAN_TYPE
};

struct UtilityInfo
{
	float utility = 0.0f;
	Vector2 endPosition = Vector2::ZERO;
	int targetEntityId = -1;
};

struct UtilityHistory 
{
	float m_lastGatherArrows = 0.0f;
	float m_lastGatherLumber = 0.0f;
	float m_lastGatherBandages = 0.0f;
	float m_lastShoot = 0.0f;
	float m_lastRepair = 0.0f;
	float m_lastHeal = 0.0f;
	float m_lastMove = 0.0f;	
	ePlanTypes m_chosenOutcome = NUM_PLAN_TYPE;
};

class Planner
{
public:
	Planner();
	~Planner();

	//queue management
	void ProcessActionStack(const uint16 agentIndex, float deltaSeconds);
	void AddActionToStack(ActionCallbackData* actionData);
	void ClearStack();
	inline size_t GetActionStackSize() { return m_actionStack.size(); }

	//planning
	void UpdatePlan(uint16 agentIndex);	
	void QueueActionsFromCurrentPlan(PositionData& positionData, PathData& pathData, uint16& indexInSortedXList, uint16& indexInSortedYList, ePlanTypes planType, const UtilityInfo& info);

	void QueueGatherArrowsAction(const UtilityInfo& info);
	void QueueGatherLumberAction(const UtilityInfo& info);
	void QueueGatherBandagesAction(const UtilityInfo& info);
	void QueueShootActions(const UtilityInfo& info);
	void QueueRepairActions(const UtilityInfo& info);
	void QueueHealActions(const UtilityInfo& info);

	std::string GetPlanTypeAsText();

	//utility cost calculations
	UtilityInfo GetHighestGatherArrowsUtility(PositionData& positionData, ActionData& actionData);
	UtilityInfo GetHighestGatherLumberUtility(PositionData& positionData, ActionData& actionData);
	UtilityInfo GetHighestGatherBandagesUtility(PositionData& positionData, ActionData& actionData);

	UtilityInfo GetHighestShootUtility(PositionData& positionData, ActionData& actionData);
	UtilityInfo GetHighestRepairUtility(PositionData& positionData, ActionData& actionData);
	UtilityInfo GetHealSelfUtility(AgentInfo& agentInfo, PositionData& positionData, ActionData& actionData);

	UtilityInfo GetRepairUtilityPerBuilding(PositionData& positionData, ActionData& actionData, PointOfInterest* poi);
	UtilityInfo GetGatherUitlityPerBuilding(PositionData& positionData, ActionData& actionData, PointOfInterest* poi);

	UtilityInfo GetIdleUtilityInfo();

	void SkewCurrentPlanUtilityValue(UtilityInfo& outInfo);
	void SkewUtilityForBias(UtilityInfo& outInfo, float biasValue);

	//utility functions
	float CalculateDistanceUtility(float normalizedDistance);
	float CalculateBuildingHealthUtility(float normalizedBuildingHealth);
	float CalculateAgentHealthUtility(float normalizedAgentHealth);
	float CalculateAgentGatherUtility(float normalizedGatherUtility);
	float CalculateShootUtility(float normalizedThreatUtility);
	float CalculateIdleUtility();

	//shooting helpers
	IntVector2 GetNearestTileCoordinateOfMapEdgeFromCoordinate(const IntVector2& coordinate);					//O(1)

	//optimizations
	bool FindAgentAndCopyPath(PositionData& positionData, PathData& pathData, uint16& indexInSortedXList, uint16& indexInSortedYList, const Vector2& endPostion);
	void CopyPath(PathData& toAgentPathData, PathData& fromAgentPathData, uint8_t startingIndex);
	uint16 GetAgentIndexFromSortedList(uint16_t agentIndex, eAgentSortType sortType);

	bool GetDoesHaveTopActionGoalPosition(Vector2& outPosition);

private:
	std::stack<ActionCallbackData*> m_actionStack;

public:
	ePlanTypes m_currentPlan;
	UtilityHistory m_utilityHistory;

	static Agent* s_agentsPlannerReference;
	static Map* s_mapPlannerReference;

	//utility data
	static UtilityStorage* m_distanceUtilityStorage;
	static UtilityStorage* m_buildingHealthUtilityStorage;
	static UtilityStorage* m_agentHealthUitilityStorage;
	static UtilityStorage* m_agentGatherUtilityStorage;
	static UtilityStorage* m_shootUtilityStorageUtility;
};

