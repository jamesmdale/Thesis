#pragma once
#include "Engine\Input\InputSystem.hpp"
#include "Engine\Renderer\Renderer.hpp"
#include "Engine\Math\Vector2.hpp"
#include "Engine\Math\Vector3.hpp"
#include "Engine\Math\IntVector2.hpp"
#include "Engine\Core\Rgba.hpp"
#include <vector>

#define ActionStackAnalysis
#define UpdatePlanAnalysis
#define AgentUpdateAnalysis
#define PathingDataAnalysis
#define CopyPathAnalysis
#define QueueActionPathingDataAnalysis
#define MemoizationDataAnalysis
#define DistanceMemoizationDataAnalysis
#define CollisionDataAnalysis

class SimulationData;
class SimulationDefinition;
class AnalysisData;

bool GetIsOptimized();
bool GetIsAgentUpdateBudgeted();

//helper methods
void ShuffleList(std::vector<int>& list);
Vector2 FloorPosition(const Vector2& position);

constexpr float RANDOM_FIRE_THRESHOLD = 0.95f;
constexpr float UPDATE_PLAN_TIMER = 10.f;
constexpr float UPDATE_INPUT_DELAY = 0.25f;

//building globals
constexpr int OUTER_WALL_THICKNESS = 2;
extern IntVector2 BUILDING_DIMENSIONS;

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
extern SimulationData* g_collisionData;

extern AnalysisData* g_processActionStackAnalysisData;
extern AnalysisData* g_updatePlanAnalysisData;
extern AnalysisData* g_agentUpdateAnalysisData;
extern AnalysisData* g_pathingAnalysisData;
extern AnalysisData* g_copyPathAnalysisData;
extern AnalysisData* g_queueActionPathingAnalysisData;
extern AnalysisData* g_distanceMemoizationAnalysisData;
extern AnalysisData* g_collisionAnalysisData;

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
extern int g_maxHealth;
extern int g_maxFireHealth;
extern float g_skewForCurrentPlan;

//poi globals
extern float g_baseResourceRefillTimePerSecond;
extern int g_maxResourceCarryAmount;

//action performance globals
extern float g_baseRepairAmountPerPerformance;
extern float g_baseShootDamageAmountPerPerformance;
extern float g_baseHealAmountPerPerformance;
extern float g_baseFireFightingAmountPerPerformance;
extern float g_minActionPerformanceRatePerSecond;
extern float g_minSkillEfficiency;

extern float g_agentOldPositionRefreshRate;

//bombardment globals
extern float g_bombardmentExplosionTime;
extern float g_bombardmentExplosionSize;
extern int g_bombardmentDamage;

//optimization globalks
extern float g_sortTimerInSeconds;
extern float g_agentCopyDestinationPositionRadius;

extern int g_numMemoizationStorageAccesses;
extern int g_numMemoizationUtilityCalls;

//threat globals
extern float g_maxThreat;

//convenience directions
extern IntVector2 MAP_NORTH;
extern IntVector2 MAP_SOUTH;
extern IntVector2 MAP_EAST;
extern IntVector2 MAP_WEST;

extern Vector2 NORTH_VEC2;
extern Vector2 SOUTH_VEC2;
extern Vector2 EAST_VEC2;
extern Vector2 WEST_VEC2;

extern IntVector2 UI_UP;
extern IntVector2 UI_DOWN;
extern IntVector2 UI_RIGHT;
extern IntVector2 UI_LEFT;

// tints
extern Rgba ARMORY_TINT;
extern Rgba LUMBERYARD_TINT;
extern Rgba MED_STATION_TINT;
extern Rgba WELL_TINT;

extern Rgba GATHER_ARROWS_TINT;
extern Rgba GATHER_LUMBER_TINT;
extern Rgba GATHER_BANDAGES_TINT;
extern Rgba GATHER_WATER_TINT;

extern Rgba SHOOT_TINT;
extern Rgba REPAIR_TINT;
extern Rgba HEAL_TINT;
extern Rgba PUT_OUT_FIRE_TINT;

//simulation data output
constexpr char* SIM_NAME_OUTPUT_TEXT = "SimulationName";
constexpr char* NUM_AGENTS_OUTPUT_TEXT = "NumAgents";
constexpr char* NUM_ARMORIES_OUTPUT_TEXT = "NumArmories";
constexpr char* NUM_LUMBERYARDS_OUTPUT_TEXT = "NumLumberyards";
constexpr char* NUM_MED_STATIONS_OUTPUT_TEXT = "NumMedStations";
constexpr char* NUM_WELLS_OUTPUT_TEXT = "NumWells";
constexpr char* BOMBARDMENT_RATE_OUTPUT_TEXT = "BombardmentRate";
constexpr char* THREAT_RATE_OUTPUT_TEXT = "ThreatRate";
constexpr char* STARTING_THREAT_OUTPUT_TEXT = "StartingThreat";
constexpr char* MAP_DIMENSIONS_OUTPUT_TEXT = "MapDimensions";
constexpr char* MAP_NAME_OUTPUT_TEXT = "MapName";
constexpr char* STARTING_TIME_OUTPUT_TEXT = "StartingTime";
constexpr char* OPTIMIZED_OUTPUT_TEXT = "IsOptimized";
constexpr char* BUDGETED_OUTPUT_TEXT = "IsBudgeted";
constexpr char* NUM_UPDATE_PLAN_CALLS_OUTPUT_TEXT = "Num Update Plan Calls";
constexpr char* NUM_PROCESS_ACTION_STACK_CALLS_OUTPUT_TEXT = "Num Process Action Stack Calls";
constexpr char* NUM_AGENT_UPDATE_CALLS_OUTPUT_TEXT = "Num Agent Update Calls";
constexpr char* NUM_GET_PATH_CALLS_OUTPUT_TEXT = "Num Get Path Calls";
constexpr char* NUM_COPY_PATH_CALLS_OUTPUT_TEXT = "Num Copy Path Calls";
constexpr char* NUM_QUEUE_ACTION_PATH_CALLS_OUTPUT_TEXT = "Num Queue Action Path Calls";
constexpr char* NUM_MEMOIZATION_STANDARD_CALLS_OUTPUT_TEXT = "Num Memoization Standard Calls";
constexpr char* NUM_MEMOIZATION_OPTIMIZED_ACCESSES_OUTPUT_TEXT = "Num Memoization Optimized Accesses";
