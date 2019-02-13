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
typedef bool (*ActionCallback)(Agent* agentsList, const uint16 agentIndex, const Vector2& goalDestination, uint16 interactEntityId);

//container for data needed to be stored in queue
struct ActionCallbackData
{
	ActionCallback m_action = nullptr;
	Vector2 m_finalGoalPosition;
	uint16 m_interactEntityId = -1;
};

struct PathData
{
	// position logic ----------------------------------------------
	Vector2 m_position;
	float m_movespeed = 1.f;
	Vector2 m_forward;

	Disc2 m_pathDisc;
	Disc2 m_physicsDisc;

	 //pathing logic ----------------------------------------------
	std::vector<Vector2> m_currentPath;
	Vector2 m_intermediateGoalPosition;	//used for temp locations while pathing
	uint8_t m_currentPathIndex = UINT8_MAX;
	Map* m_map = nullptr;

	// sprite data ----------------------------------------------
	IntVector2 m_spriteDirection; // = IntVector2::UP;	
};

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

struct AgentInfo
{
	uint16 m_id = -1;
	uint8 m_updatePriority = 1;	
	uint8 m_health = 100;	
};

struct ActionData
{	
	bool m_isFirstLoopThroughAction = true;
	Stopwatch* m_actionTimer = nullptr;
	IsoSpriteAnimSet* m_animationSet; // = nullptr;

	// inventory ----------------------------------------------
	uint8 m_arrowCount = 0;
	uint8 m_bandageCount = 0;
	uint8 m_lumberCount = 0;
};


class Agent
{
public:
	Agent();

	Agent(Vector2 startingPosition, IsoSpriteAnimSet* animationSet, Map* mapReference);
	~Agent();

	static void GenerateRandomStats(Personality& personality);

	//overriden classes
	void Update(int agentIndex, float deltaSeconds);
	void QuickUpdate(int agentIndex, float deltaSeconds);
	void Render();

	static void UpdateSpriteRenderDirection(PathData& pathData);

	static void TakeDamage(AgentInfo& agentInfo, int damageAmount);
	static void IncreaseUpdatePriority(AgentInfo& agentInfo, int amount = 1);

	static void ResetPriority(AgentInfo& agentInfo);

	//pathing
	static bool GetPathToDestination(PathData& pathData, const Vector2& goalDestination);
	static bool GetIsAtPosition(PathData& pathData, const Vector2& goalDestination);
	static void UpdatePhysicsData(PathData& pathData);

	//stats
	static void UpdateCombatPerformanceTime(Personality& personality);
	static void UpdateRepairPerformanceTime(Personality& personality);
	static void UpdateHealPerformanceTime(Personality& personality);

	void ConstructInformationAsText(std::vector<std::string>& outStrings);

public:
	//structs organized by access ----------------------------------------------
	std::vector<AgentInfo> m_agentInfo;	
	std::vector<PathData> m_pathData;
	std::vector<Personality> m_personality;
	std::vector<Planner> m_planner; // = nullptr;
	std::vector<ActionData> m_actionData;
	std::vector<uint16> m_indexInSortedXList; // = UINT16_MAX;
	std::vector<uint16> m_indexInSortedYList; // = UINT16_MAX;
	std::vector<uint16> m_indexInPriorityList; // = UINT16_MAX;	
};


// actions ----------------------------------------------
bool MoveAction(Agent* agentsList, const uint16 agentIndex, const Vector2& goalDestination, int interactEntityId);		//walk between locations	
bool ShootAction(Agent* agentsList, const uint16 agentIndex, const Vector2& goalDestination, int interactEntityId);	    //combat
bool RepairAction(Agent* agentsList, const uint16 agentIndex, const Vector2& goalDestination, int interactEntityId);									//repairs
bool HealAction(Agent* agentsList, const uint16 agentIndex, const Vector2& goalDestination, int interactEntityId);										//heal
bool GatherAction(Agent* agentsList, const uint16 agentIndex, const Vector2& goalDestination, int interactEntityId);		//acquire resource at poiLocation	





