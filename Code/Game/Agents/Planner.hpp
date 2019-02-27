#pragma once
#include "Engine\Math\IntVector2.hpp"
#include "Game\UtilityHelpers\UtilityStorage.hpp"
#include <stack>

struct ActionData;
class Agent;
class Fire;
class Map;
class PointOfInterest;
enum eAgentSortType;

enum ePlanTypes
{
	NONE_PLAN_TYPE,
	GATHER_ARROWS_PLAN_TYPE,
	GATHER_LUMBER_PLAN_TYPE,
	GATHER_BANDAGES_PLAN_TYPE,
	GATHER_WATER_PLAN_TYPE,
	SHOOT_PLAN_TYPE,
	REPAIR_PLAN_TYPE,
	HEAL_PLAN_TYPE,
	PUT_OUT_FIRE_PLAN_TYPE,
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
	Planner(Map* mapReference, Agent* agentReference);
	~Planner();

	//queue management
	void ProcessActionStack(float deltaSeconds);
	void AddActionToStack(ActionData* actionData);
	void ClearStack();
	inline size_t GetActionStackSize() { return m_actionStack.size(); }

	//planning
	void UpdatePlan();	
	void QueueActionsFromCurrentPlan(ePlanTypes planType, const UtilityInfo& info);

	void QueueGatherArrowsAction(const UtilityInfo& info);
	void QueueGatherLumberAction(const UtilityInfo& info);
	void QueueGatherBandagesAction(const UtilityInfo& info);
	void QueueGatherWaterAction(const UtilityInfo& info);
	void QueueShootActions(const UtilityInfo& info);
	void QueueRepairActions(const UtilityInfo& info);
	void QueueHealActions(const UtilityInfo& info);

	std::string GetPlanTypeAsText();

	//utility cost calculations
	UtilityInfo GetHighestGatherArrowsUtility();
	UtilityInfo GetHighestGatherLumberUtility();
	UtilityInfo GetHighestGatherBandagesUtility();
	UtilityInfo GetHighestGatherWaterUtility();

	UtilityInfo GetHighestShootUtility();
	UtilityInfo GetHighestRepairUtility();
	UtilityInfo GetHealSelfUtility();
	UtilityInfo GetPutOutFireUtility();

	UtilityInfo GetRepairUtilityPerBuilding(PointOfInterest* poi);
	UtilityInfo GetGatherUitlityPerBuilding(PointOfInterest* poi);
	UtilityInfo GetHealUtilityPerAgent(Agent* agent);
	UtilityInfo GetPutOutFireUtility(Fire* fire);

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
	bool FindAgentAndCopyPath(const Vector2& endPostion);
	void CopyPath(Agent* toAgent, Agent* fromAgent, uint8_t startingIndex);
	Agent* GetAgentFromSortedList(uint16_t agentIndex, eAgentSortType sortType);

	bool GetDoesHaveTopActionGoalPosition(Vector2& outPosition);

public:
	Map* m_map = nullptr;
	Agent* m_agent = nullptr;
	ePlanTypes m_currentPlan;

	UtilityHistory m_utilityHistory;

	//utility data
	static UtilityStorage* m_distanceUtilityStorage;
	static UtilityStorage* m_buildingHealthUtilityStorage;
	static UtilityStorage* m_agentHealthUitilityStorage;
	static UtilityStorage* m_agentGatherUtilityStorage;
	static UtilityStorage* m_shootUtilityStorageUtility;

private:
	std::stack<ActionData*> m_actionStack;
};

