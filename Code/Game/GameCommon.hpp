#pragma once
#include "Engine\Input\InputSystem.hpp"
#include "Engine\Renderer\Renderer.hpp"
#include "Engine\Math\Vector2.hpp"
#include "Engine\Math\Vector3.hpp"
#include "Engine\Core\Rgba.hpp"

#define ActionStackAnalysis
#define UpdatePlanAnalysis
#define AgentUpdateAnalysis
#define PathingDataAnalysis
#define CopyPathAnalysis
#define QueueActionPathingDataAnalysis
#define DistanceMemoizationDataAnalysis

//data organization switches
#define ArrayOfStructs
#define StructOfArrays

class SimulationData;
class SimulationDefinition;

bool GetIsOptimized();
bool GetIsAgentUpdateBudgeted();

//following line will go here when any engine side globals are declared and need to be used
//#include "Engine/Core/EngineCommon.hpp"

//sim data
constexpr uint16 MAX_NUM_AGENTS = UINT16_MAX;
constexpr uint8 MAX_AGENT_PATH_LENGTH = 50;
constexpr float PHYSICS_DISC_RADIUS = 0.25;

//camera data
const Vector2 g_bottomLeftOrtho = Vector2( -1.f, -1.f);
const Vector2 g_topRightOrtho = Vector2(1.f, 1.f);
const Vector2 g_center = Vector2((g_bottomLeftOrtho.x + g_topRightOrtho.x) * .5f, (g_bottomLeftOrtho.y + g_topRightOrtho.y) * .5f);
const Rgba g_backgroundColor = Rgba(1.f, 1.f, 1.f, 1.f);

const float g_tilePercentageOfWindow = 0.025f;

//game related globals
extern float g_tileSize;
extern float g_divideTileSize;

extern float g_maxCoordinateDistanceSquared;

//general globals
constexpr int g_maxHealth = 100;
constexpr float g_skewForCurrentPlan = 0.05f;
 
 //poi globals
constexpr float g_baseResourceRefillTimePerSecond = 0.5f;
constexpr int g_maxResourceCarryAmount = 1;

 //action performance globals
constexpr float g_baseRepairAmountPerPerformance = 5.f;
constexpr float g_baseShootDamageAmountPerPerformance = 5.f;
constexpr float g_baseHealAmountPerPerformance = 5.f;
constexpr float g_minActionPerformanceRatePerSecond = 0.25f;

constexpr float g_minSkillEfficiency = 0.1f;

 //optimization globals
constexpr float g_sortTimerInSeconds = 0.5f;
constexpr float g_agentCopyDestinationPositionRadius = 0.5f;

 //threat globals
constexpr float g_maxThreat = 500.f;

//bombardment globals
extern float g_bombardmentExplosionTime;
extern int g_bombardmentDamage;
extern float g_bombardmentExplosionSize;

extern SimulationDefinition* g_currentSimulationDefinition;

extern SimulationData* g_generalSimulationData;
extern SimulationData* g_processActionStackData;
extern SimulationData* g_updatePlanData;
extern SimulationData* g_agentUpdateData;
extern SimulationData* g_pathingData;
extern SimulationData* g_copyPathData;
extern SimulationData* g_queueActionPathingData;
extern SimulationData* g_distanceMemoizationData;

extern uint g_numUpdatePlanCalls;
extern uint g_numActionStackProcessCalls;
extern uint g_numAgentUpdateCalls;
extern uint g_numGetPathCalls;
extern uint g_numCopyPathCalls;
extern uint g_numQueueActionPathCalls;

extern int g_currentSimDefinitionIndex;

//debug globals
extern bool g_isDebug;
extern bool g_isQuitting;
extern bool g_isIdShown;
extern bool g_isDebugDataShown;
extern float g_halfTileSize;
extern bool g_isBlockedTileDataShown;

//time globals
extern uint64_t g_perFrameHPCBudget;
extern uint64_t g_previousFrameRenderTime;
extern uint64_t g_previousFrameNonAgentUpdateTime;
extern uint64_t g_agentUpdateBudgetThisFrame;
extern int g_agentsUpdatedThisFrame;

//convenience directions
extern IntVector2 MAP_NORTH;
extern IntVector2 MAP_SOUTH;
extern IntVector2 MAP_EAST;
extern IntVector2 MAP_WEST;

extern IntVector2 UI_UP;
extern IntVector2 UI_DOWN;
extern IntVector2 UI_RIGHT;
extern IntVector2 UI_LEFT;

//tints
extern Rgba ARMORY_TINT;
extern Rgba LUMBER_TINT;
extern Rgba MED_TINT;

//agent tints
extern Rgba GATHER_ARROWS_TINT;
extern Rgba GATHER_LUMBER_TINT;
extern Rgba GATHER_BANDAGES_TINT;

extern Rgba SHOOT_TINT;
extern Rgba REPAIR_TINT;
extern Rgba HEAL_TINT;
