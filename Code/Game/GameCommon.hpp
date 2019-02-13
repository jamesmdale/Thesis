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

//camera data
const Vector2 g_bottomLeftOrtho = Vector2( -1.f, -1.f);
const Vector2 g_topRightOrtho = Vector2(1.f, 1.f);
const Vector2 g_center = Vector2((g_bottomLeftOrtho.x + g_topRightOrtho.x) * .5f, (g_bottomLeftOrtho.y + g_topRightOrtho.y) * .5f);
const Rgba g_backgroundColor = Rgba(1.f, 1.f, 1.f, 1.f);

const float g_tilePercentageOfWindow = 0.025f;

//debug globals
extern bool g_isDebug;
extern bool g_isQuitting;
extern bool g_isIdShown;
extern bool g_isDebugDataShown;

//game related globals
extern float g_tileSize;
extern float g_divideTileSize;
extern float g_halfTileSize;
extern bool g_isBlockedTileDataShown;
extern float g_maxCoordinateDistanceSquared;

//time globals
extern uint64_t g_perFrameHPCBudget;
extern uint64_t g_previousFrameRenderTime;
extern uint64_t g_previousFrameNonAgentUpdateTime;
extern uint64_t g_agentUpdateBudgetThisFrame;
extern int g_agentsUpdatedThisFrame;

//general globals
extern uint8 g_maxHealth;
extern float g_skewForCurrentPlan;

//poi globals
extern float g_baseResourceRefillTimePerSecond;
extern uint8 g_maxResourceCarryAmount;

//action performance globals
extern float g_baseRepairAmountPerPerformance;
extern float g_baseShootDamageAmountPerPerformance;
extern float g_baseHealAmountPerPerformance;
extern float g_minActionPerformanceRatePerSecond;

extern float g_minSkillEfficiency;

//bombardment globals
extern float g_bombardmentExplosionTime;
extern float g_bombardmentExplosionSize;
extern uint8 g_bombardmentDamage;

//optimization globalks
extern float g_sortTimerInSeconds;
extern float g_agentCopyDestinationPositionRadius;

//  threat globals
extern uint16 g_maxThreat;

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