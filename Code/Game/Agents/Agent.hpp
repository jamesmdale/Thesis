#pragma once
#include "Game\Sprites\IsoSpriteAnimSet.hpp"
#include "Engine\Math\IntVector2.hpp"
#include "Engine\Math\Vector2.hpp"
#include "Engine\Core\EngineCommon.hpp"
#include "Engine\Core\Transform2D.hpp"
#include "Engine\Math\AABB2.hpp"
#include "Engine\Math\Disc2.hpp"

//forward declarations
class Planner;
class Map;
class PlayingState;
class Stopwatch;

//typedefs
typedef bool (*ActionCallback)(Agent* agents, const uint16 agentIndex, const Vector2& goalDestination, int interactEntityId);


//container for data needed to be stored in queue  ----------------------------------------------
struct ActionCallbackData
{
	ActionCallback m_action = nullptr;
	Vector2 m_finalGoalPosition;
	uint16 m_interactEntityId = -1;
};

//  ----------------------------------------------
struct AgentInfo
{
	uint16 m_id = -1;
	uint8 m_updatePriority = 1;	
	uint8 m_health = 100;	
};

struct PositionData
{
	// position logic ----------------------------------------------
	Vector2 m_position;
	float m_movespeed = 1.f;
	Vector2 m_forward;

	Disc2 m_physicsDisc;

	// sprite data ----------------------------------------------
	IntVector2 m_spriteDirection; // = IntVector2::UP;	
};

// pathing logic  ----------------------------------------------
struct PathData
{	
	PathData(){};
	~PathData()
	{
		delete[] m_currentPath;
	}

	uint8 m_pathCount = 0;	
	uint8_t m_currentPathIndex = UINT8_MAX;
	Vector2 m_intermediateGoalPosition;	//used for temp locations while pathing	

	Vector2* m_currentPath = new Vector2[256];
};

//  ----------------------------------------------
struct Personality
{
	// bias ----------------------------------------------
	float m_combatBias = 0.5f;
	float m_repairBias = 0.5f;
	float m_healBias = 0.5f;

	// skill ----------------------------------------------
	float m_combatEfficiency = 1.f;
	float m_repairEfficiency = 1.f;
	float m_healEfficiency= 1.f;

	float m_calculatedCombatPerformancePerSecond = 1.f;
	float m_calculatedRepairPerformancePerSecond = 1.f;
	float m_calculatedHealPerformancePerSecond= 1.f;
};

//  ----------------------------------------------
struct ActionData
{	
	ActionData(){};
	~ActionData()
	{
		m_animationSet = nullptr;
	}


	bool m_isFirstLoopThroughAction = true;
	Stopwatch* m_actionTimer = nullptr;
	IsoSpriteAnimSet* m_animationSet = nullptr;

	// inventory ----------------------------------------------
	uint8 m_arrowCount = 0;
	uint8 m_bandageCount = 0;
	uint8 m_lumberCount = 0;
};

//  ----------------------------------------------
class Agent
{
public:
	Agent(Map* mapReference, uint16 numAgents);
	~Agent();

	void AddAgentData(const uint16 id, const Vector2& startingPosition, IsoSpriteAnimSet* animationSet);
	
	void Initialize();
	void InitializeAgentInfos();
	void InitializePathData();
	void InitializePersonalities();
	void InitializeActionData();
	void InitialisePlannerData();

	//overriden classes
	void Update(int agentIndex, float deltaSeconds);
	void QuickUpdate(int agentIndex, float deltaSeconds);
	void Render();

	static void ClearPathData(PathData& pathData);
	static void GenerateRandomStats(Personality& personality);

	static void TakeDamage(AgentInfo& agentInfo, int damageAmount);
	static void IncreaseUpdatePriority(AgentInfo& agentInfo, int amount = 1);

	static void ResetPriority(AgentInfo& agentInfo);

	//pathing
	static bool GetPathToDestination(PositionData& positionData, PathData& pathData, const Vector2& goalDestination);
	static bool GetIsAtPosition(PositionData& positionData, const Vector2& goalDestination);
	static void UpdatePhysicsData(PositionData& positionData);
	static void UpdateSpriteRenderDirection(PositionData& positionData);

	//stats
	static void UpdateCombatPerformanceTime(Personality& personality);
	static void UpdateRepairPerformanceTime(Personality& personality);
	static void UpdateHealPerformanceTime(Personality& personality);

	void ConstructInformationAsText(std::vector<std::string>& outStrings);

public:
	//structs organized by access ----------------------------------------------
	AgentInfo* m_agentInfo = nullptr;
	PositionData* m_positionData = nullptr;
	PathData* m_pathData = nullptr;
	Personality* m_personality = nullptr;
	Planner* m_planner = nullptr;
	ActionData* m_actionData = nullptr;
	uint16* m_indexInSortedXList = nullptr;
	uint16* m_indexInSortedYList = nullptr;
	uint16* m_indexInPriorityList = nullptr;

	static uint16 s_numAgents;
	static Map* s_mapAgentReference;
};


// actions ----------------------------------------------
bool MoveAction(Agent* agents, const uint16 agentIndex, const Vector2& goalDestination, int interactEntityId);			//walk between locations	
bool ShootAction(Agent* agents, const uint16 agentIndex, const Vector2& goalDestination, int interactEntityId);			//combat
bool RepairAction(Agent* agents, const uint16 agentIndex, const Vector2& goalDestination, int interactEntityId);		//repairs
bool HealAction(Agent* agents, const uint16 agentIndex, const Vector2& goalDestination, int interactEntityId);			//heal
bool GatherAction(Agent* agents, const uint16 agentIndex, const Vector2& goalDestination, int interactEntityId);		//acquire resource at poiLocation	





