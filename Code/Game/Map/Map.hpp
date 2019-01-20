#pragma once
#include "Game\Definitions\MapDefinition.hpp"
#include "Game\Map\Tile.hpp"
#include "Game\SimulationData.hpp"
#include "Engine\Math\IntVector2.hpp"
#include "Engine\Renderer\RenderScene2D.hpp"
#include "Engine\Utility\Grid.hpp"
#include <string>
#include <vector>

//forward declarations
enum ePointOfInterestType;
class Agent;
class PointOfInterest;
class Bombardment;
class Stopwatch;
class Mesh;
class PlayingState;
class SimulationDefinition;

enum eAgentSortType
{
	X_AGENT_SORT_TYPE,
	Y_AGENT_SORT_TYPE,
	PRIORITY_AGENT_SORT_TYPE,
	NUM_SORT_TYPES
};

class Map
{
public:
	explicit Map(SimulationDefinition* definition, const std::string& mapName, RenderScene2D* renderScene);

	Map::~Map();

	void Initialize();

	void Update(float deltaSeconds);

	void UpdateAgents(float deltaSeconds);
	void UpdateAgentsBudgeted(float deltaSeconds);
	void Render();

	void Reload(SimulationDefinition* definition);
	void SetMapType(MapDefinition* newMapDefintion) { m_mapDefinition = newMapDefintion; }
	IntVector2 GetDimensions() { return m_dimensions; }
	float GetMapDistanceSquared(){ return (m_dimensions.x * m_dimensions.y) * (m_dimensions.x * m_dimensions.y);}

	//optimized mesh generation
	void CreateMapMesh();

	Mesh* CreateDynamicAgentMesh();
	Mesh* CreateDynamicBombardmentMesh();	
	Mesh* CreateTextMesh();

	//Cleanup functions
	void DeleteDeadEntities();

	//agent sorting
	void SortAgentsByX();
	void SortAgentsByY();
	void QuickSortAgentByPriority(std::vector<Agent*>& agents, int startIndex, int endIndex);
	int QuickSortPivot(std::vector<Agent*>& agents, int startIndex, int endIndex);
	void SwapAgents(int indexI, int indexJ, eAgentSortType type);

	//Conversion functions for Tile Coordinates to World Coordinates
	IntVector2 GetTileCoordinateOfPosition(const Vector2& position);
	Vector2 GetWorldPositionOfMapCoordinate(const IntVector2& coordinate);

	//helpers
	bool CheckIsPositionValid(const Vector2& position);
	bool CheckIsCoordianteValid(const IntVector2& coordinate);
	Vector2 GetRandomNonBlockedPositionInMapBounds();
	IntVector2 GetRandomNonBlockedCoordinateInMapBounds();
	IntVector2 GetRandomCoordinateInMapBounds();
	Grid<int>* GetAsGrid();
	bool IsTileBlockingAtCoordinate(const IntVector2& coordinate);
	Tile* GetTileAtCoordinate(const IntVector2& coordinate);
	Agent* GetAgentById(int agentId);

	//point of interest helpers
	PointOfInterest* GeneratePointOfInterest(int poiType);
	PointOfInterest* GetPointOfInterestById(int poiId);	

	//bombardment collision
	void DetectBombardmentToAgentCollision(Bombardment* bombardment);
	void DetectBombardmentToPOICollision(Bombardment* bombardment);

public:
	std::string m_name;
	IntVector2 m_dimensions;
	MapDefinition* m_mapDefinition = nullptr;
	SimulationDefinition* m_activeSimulationDefinition = nullptr;
	std::vector<Tile*> m_tiles;
	Grid<int>* m_mapAsGrid = nullptr;
	AABB2 m_mapWorldBounds;

	//lists
	std::vector<Agent*> m_agentsOrderedByPriority;
	std::vector<Agent*> m_agentsOrderedByXPosition;
	std::vector<Agent*> m_agentsOrderedByYPosition;

	std::vector<PointOfInterest*> m_pointsOfInterest;

	//separate lists of each type for ease of sorting
	std::vector<PointOfInterest*> m_armories;
	std::vector<PointOfInterest*> m_lumberyards;
	std::vector<PointOfInterest*> m_medStations;

	std::vector<Bombardment*> m_activeBombardments;

	//meshes for rendering
	Mesh* m_mapMesh = nullptr;
	Mesh* m_debugMapMesh = nullptr;

	Stopwatch* m_bombardmentTimer = nullptr;
	Stopwatch* m_threatTimer = nullptr;
	Stopwatch* m_sortTimer = nullptr;

	PlayingState* m_gameState = nullptr;

	float m_threat = 500.f;

private:
	bool m_isFullMapView = false;
};