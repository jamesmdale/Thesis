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
class Agent;
class Stopwatch;

//typedefs
typedef bool (*ActionCallback)(Agent* agent, const Vector2& goalDestination, int interactEntityId);

//container for data needed to be stored in queue
struct ActionData
{
	ActionCallback m_action = nullptr;
	Vector2 m_finalGoalPosition;
	int m_interactEntityId = -1;
};

class Agent
{
public:
	Agent();

	Agent(Vector2 startingPosition, IsoSpriteAnimSet* animationSet, Map* mapReference);
	~Agent();

	void GenerateRandomStats();

	//overriden classes
	void Update(float deltaSeconds);
	void QuickUpdate(float deltaSeconds);
	void Render();

	void UpdateSpriteRenderDirection();

	void TakeDamage(int damageAmount);
	void IncreaseUpdatePriority(int amount = 1);
	void ResetPriority();

	void ResetPlannerAndPathing();
	void ClearCurrentPath();

	//pathing
	bool GetPathToDestination(const Vector2& goalDestination);
	bool GetIsAtPosition(const Vector2& goalDestination);
	void UpdatePhysicsData();

	bool IsHazardAhead();

	//stats
	void UpdateCombatPerformanceTime();
	void UpdateRepairPerformanceTime();
	void UpdateHealPerformanceTime();
	void UpdateFireFightingPerformanceTime();

	//debug tools
	void ConstructInformationAsText(std::vector<std::string>& outStrings);

public:
	int m_updatePriority = 1;

	int m_id = -1;
	int m_health = 100;
	bool m_isFirstLoopThroughAction = true;
	Stopwatch* m_actionTimer = nullptr;
	Stopwatch* m_positionStuckCheckTimer = nullptr;

	// bias ----------------------------------------------
	float m_combatBias = 0.5f;
	float m_repairBias = 0.5f;
	float m_healBias = 0.5f;
	float m_fireFightingBias = 0.5f;

	// skill ----------------------------------------------

	float m_combatEfficiency = 1.f;
	float m_repairEfficiency = 1.f;
	float m_healEfficiency = 1.f;
	float m_fireFightingEfficiency = 1.f;

	float m_calculatedCombatPerformancePerSecond = 1.f;
	float m_calculatedRepairPerformancePerSecond = 1.f;
	float m_calculatedHealPerformancePerSecond = 1.f;
	float m_calculatedFireFightingPerformancePerSecond = 1.f;

	// inventory ----------------------------------------------
	int m_arrowCount = 0;
	int m_bandageCount = 0;
	int m_lumberCount = 0;
	int m_waterCount = 0;

	// position logic ----------------------------------------------
	float m_movespeed = 1.f;

	Vector2 m_position;
	Vector2 m_forward;
	Vector2 m_intermediateGoalPosition;	//used for temp locations while pathing
	
	//used for detecting cases where the agent could get stuck
	Vector2 m_oldPosition;

	Disc2 m_physicsDisc;

	//goal logic ----------------------------------------------
	std::vector<Vector2> m_currentPath;
	uint8_t m_currentPathIndex = UINT8_MAX;

	//sprites ----------------------------------------------
	IntVector2 m_spriteDirection = IntVector2::UP;
	IsoSpriteAnimSet* m_animationSet = nullptr;			

	//helper references ----------------------------------------------
	Planner* m_planner = nullptr;

	uint16_t m_indexInSortedXList = UINT16_MAX;
	uint16_t m_indexInSortedYList = UINT16_MAX;
	uint16_t m_indexInPriorityList = UINT16_MAX;	
};


// actions ----------------------------------------------
bool MoveAction(Agent* agent, const Vector2& goalDestination, int interactEntityId);		//walk between locations	
bool ShootAction(Agent* agent, const Vector2& goalDestination, int interactEntityId);	    //combat
bool RepairAction(Agent* agent, const Vector2& goalDestination, int interactEntityId);		//repairs
bool HealAction(Agent* agent, const Vector2& goalDestination, int interactEntityId);		//heal
bool FightFireAction(Agent* agent, const Vector2& goalDestination, int interactEntityId);	//fight fire
bool GatherAction(Agent* agent, const Vector2& goalDestination, int interactEntityId);		//acquire resource at poiLocation	





