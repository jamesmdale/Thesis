#include "Game\GameCommon.hpp"
#include "Game\SimulationData.hpp"
#include "Game\Game.hpp"
#include "Game\Definitions\SimulationDefinition.hpp"
#include "Game\Helpers\AnalysisData.hpp"
#include "Engine\Window\Window.hpp"

IntVector2 BUILDING_DIMENSIONS = IntVector2(2, 2);

SimulationDefinition* g_currentSimulationDefinition = nullptr;

SimulationData* g_generalSimulationData = nullptr;
SimulationData* g_processActionStackData = nullptr;
SimulationData* g_updatePlanData = nullptr;
SimulationData* g_agentUpdateData = nullptr;
SimulationData* g_pathingData = nullptr;
SimulationData* g_copyPathData = nullptr;
SimulationData* g_queueActionPathingData = nullptr;
SimulationData* g_collisionData = nullptr;
SimulationData* g_memoizationData = nullptr;
SimulationData* g_testExtremeMemoizationData = nullptr;

AnalysisData* g_processActionStackAnalysisData = nullptr;
AnalysisData* g_updatePlanAnalysisData = nullptr;
AnalysisData* g_agentUpdateAnalysisData = nullptr;
AnalysisData* g_pathingAnalysisData = nullptr;
AnalysisData* g_copyPathAnalysisData = nullptr;
AnalysisData* g_queueActionPathingAnalysisData = nullptr;
AnalysisData* g_collisionAnalysisData = nullptr;
AnalysisData* g_memoizationAnalysisData = nullptr;
AnalysisData* g_testExtremeMemoizationAnalysisData = nullptr;

//debug globals
bool g_isDebug = false;
bool g_isQuitting = false;
bool g_isIdShown = false;
bool g_isBlockedTileDataShown = false;
bool g_isDebugDataShown = false;

//data set in game startup after window has been initialized
float g_tileSize = 1.f;
float g_divideTileSize = 1.f;
float g_halfTileSize = 1.f;
float g_maxCoordinateDistanceSquared = 0.f;

uint g_utilityStorageDivisions = 100;

//time globals
uint64_t g_perFrameHPCBudget = 0;
uint64_t g_perFramePrioritySort = 0;
uint64_t g_previousFrameRenderTime = 0;
uint64_t g_previousSortTime = 0;
uint64_t g_previousFrameNonAgentUpdateTime = 0;
uint64_t g_agentUpdateBudgetThisFrame = 0;
int g_agentsUpdatedThisFrame = 0;

//general globals
int g_maxHealth = 100;
int g_maxFireHealth = 10;
float g_skewForCurrentPlan = 0.05f;

//poi globals
float g_baseResourceRefillTimePerSecond = 0.5f;
int g_maxResourceCarryAmount = 3;

//action performance globals
float g_baseRepairAmountPerPerformance = 5.f;
float g_baseShootDamageAmountPerPerformance = 5.f;
float g_baseHealAmountPerPerformance = 5.f;
float g_baseFireFightingAmountPerPerformance = 5.f;
float g_minActionPerformanceRatePerSecond = 0.25f;

float g_minSkillEfficiency = 0.1f;
float g_agentOldPositionRefreshRate = 2.f;

//bombardment globals
float g_bombardmentExplosionTime = 1.f;
float g_bombardmentExplosionSize = 1.f;
int g_bombardmentDamage = 10.f;

//optimization globals
float g_sortTimerInSeconds = 0.5f;
float g_agentCopyDestinationPositionRadius = 0.5f;

int g_numMemoizationStorageAccesses = 0;
int g_numMemoizationUtilityCalls = 0;

int g_numTestMemoizationExtremeUtilityCalls = 0;
int g_numTestMemoizationStorageAccesses = 0;

//threat globals
float g_maxThreat = 500.f;

//XY
IntVector2 MAP_NORTH = IntVector2(0, 1);
IntVector2 MAP_SOUTH = IntVector2(0, -1);
IntVector2 MAP_EAST = IntVector2(1, 0);
IntVector2 MAP_WEST = IntVector2(-1, 0);

Vector2 NORTH_VEC2 = Vector2(0.f, 1.f);
Vector2 SOUTH_VEC2 = Vector2(0.f, -1.f);;
Vector2 EAST_VEC2 = Vector2(1.f, 0.f);;
Vector2 WEST_VEC2 = Vector2(-1.f, 0.f);;

IntVector2 UI_UP = IntVector2(0, 1);
IntVector2 UI_DOWN = IntVector2(0, -1);
IntVector2 UI_RIGHT = IntVector2(-1, 0);
IntVector2 UI_LEFT = IntVector2(1, 0);

//TINTS
Rgba ARMORY_TINT = Rgba(248, 124, 124, 200);
Rgba LUMBERYARD_TINT = Rgba(255, 255, 0, 255);
Rgba MED_STATION_TINT = Rgba(124, 248, 124, 200);
Rgba WELL_TINT = Rgba(124, 124, 248, 200);

Rgba GATHER_ARROWS_TINT = Rgba(255, 160, 160, 255); //red
Rgba GATHER_LUMBER_TINT= Rgba(255, 226, 170, 255); //yellow
Rgba GATHER_BANDAGES_TINT= Rgba(160, 255, 160, 255); //green
Rgba GATHER_WATER_TINT = Rgba(140, 140, 255, 255); //blue

Rgba SHOOT_TINT = Rgba(255, 0, 0, 255);//red
Rgba REPAIR_TINT = Rgba(255, 255, 0, 255); //yellow
Rgba HEAL_TINT = Rgba(0, 255, 0, 255); //green
Rgba PUT_OUT_FIRE_TINT = Rgba(0, 0, 255, 255); //blue

//  =============================================================================
// Methods =============================================================================
//  =============================================================================
bool GetIsOptimized()
{
	if (g_currentSimulationDefinition != nullptr)
	{
		return g_currentSimulationDefinition->m_isOptimized;
	}

	//else
	return false;
}

bool GetIsAgentUpdateBudgeted()
{
	if (g_currentSimulationDefinition != nullptr)
	{
		return g_currentSimulationDefinition->m_isUpdateBudgeted;
	}

	//else
	return false;
}

//  =========================================================================================
void ShuffleList(std::vector<int>& list)
{
	RNG* theRNG = Game::GetGlobalRNG();

	//no need to shuffle
	if(list.size() <= 1)
		return;

	int maxIndex = (int)list.size() - 1;

	//not totally random would need to revisit this, but will do for now.
	for (int shuffleCount = 0; shuffleCount < (int)list.size(); ++shuffleCount)
	{
		int swapVal = (int)theRNG->GetRandomUintInRange(0, maxIndex);
		int swapVal2 = (int)theRNG->GetRandomUintInRange(0, maxIndex);

		int tempVal = 0;
		// swap cards around in array
		tempVal = list[swapVal];
		list[swapVal] = list[swapVal2];
		list[swapVal2] = tempVal;
	}	
}

//  =========================================================================================
Vector2 FloorPosition(const Vector2 & position)
{
	Vector2 tempPosition = position;
	tempPosition.Floor();
	return tempPosition;
}
