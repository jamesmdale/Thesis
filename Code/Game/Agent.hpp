#pragma once
#include "Game\Sprites\IsoSpriteAnimSet.hpp"
#include "Engine\Math\IntVector2.hpp"
#include "Engine\Math\Vector2.hpp"
#include "Engine\Core\EngineCommon.hpp"
#include "Engine\Core\Transform2D.hpp"
#include "Engine\Math\AABB2.hpp"

//forward declarations
class Planner;
class Map;
class PlayingState;
class Agent;

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

	//pathing
	bool GetPathToDestination(const Vector2& goalDestination);
	bool GetIsAtPosition(const Vector2& goalDestination);

	AABB2 GetBounds();

	//stats
	void UpdateCombatPerformanceTime();
	void UpdateRepairPerformanceTime();
	void UpdateHealPerformanceTime();

	void ConstructInformationAsText(std::vector<std::string>& outStrings);

public:
	int m_updatePriority = 1;

	int m_id = -1;
	int m_health = 100;

	// bias ----------------------------------------------
	float m_combatBias = 0.5f;
	float m_repairBias = 0.5f;
	float m_healBias = 0.5f;

	// skill ----------------------------------------------

	float m_combatEfficiency = 1.f;
	float m_repairEfficiency = 1.f;
	float m_healEfficiency = 1.f;

	float m_calculatedCombatPerformancePerSecond = 1.f;
	float m_calculatedRepairPerformancePerSecond = 1.f;
	float m_calculatedHealPerformancePerSecond = 1.f;

	// inventory ----------------------------------------------
	int m_arrowCount = 0;
	int m_bandageCount = 0;
	int m_lumberCount = 0;

	// position logic ----------------------------------------------
	Vector2 m_position;
	Vector2 m_forward;
	Vector2 m_intermediateGoalPosition;	//used for temp locations while pathing
	Vector2 m_currentActionGoalPosition;
	Transform2D m_transform;
	float m_movespeed = 1.f;

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

	//optimization members

	//std::vector<Agent*> m_subordinateAgents;
	//Agent* m_commandingAgent = nullptr;
};


// actions ----------------------------------------------
bool MoveAction(Agent* agent, const Vector2& goalDestination, int interactEntityId);		//walk between locations	
bool ShootAction(Agent* agent, const Vector2& goalDestination, int interactEntityId);	    //combat
bool RepairAction(Agent* agent, const Vector2& goalDestination, int interactEntityId);									//repairs
bool HealAction(Agent* agent, const Vector2& goalDestination, int interactEntityId);										//heal
bool GatherAction(Agent* agent, const Vector2& goalDestination, int interactEntityId);		//acquire resource at poiLocation	





