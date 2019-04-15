#include "Game\Map\Map.hpp"
#include "Game\Definitions\MapDefinition.hpp"
#include "Game\Definitions\SimulationDefinition.hpp"
#include "Game\SimulationData.hpp"
#include "Game\SimulationData.hpp"
#include "Game\Map\MapGenStep.hpp"
#include "Game\GameCommon.hpp"
#include "Game\Entities\PointOfInterest.hpp"
#include "Game\Agents\Agent.hpp"
#include "Game\Agents\Planner.hpp"
#include "Game\Map\Tile.hpp"
#include "Game\Entities\Bombardment.hpp"
#include "Game\Entities\Fire.hpp"
#include "Game\GameStates\PlayingState.hpp"
#include "Game\SimulationData.hpp"
#include "Game\Helpers\AnalysisData.hpp"
#include "Engine\Window\Window.hpp"
#include "Engine\Time\Stopwatch.hpp"
#include "Engine\Math\MathUtils.hpp"
#include "Engine\Profiler\Profiler.hpp"
#include "Engine\Renderer\MeshBuilder.hpp"
#include "Engine\Renderer\Mesh.hpp"
#include "Engine\Time\SimpleTimer.hpp"

int g_fireIdMarker = 0;

//  =========================================================================================
Map::Map(SimulationDefinition* simulationDefinition, const std::string & mapName, RenderScene2D* renderScene)
{
	m_name = mapName;
	m_mapDefinition = simulationDefinition->m_mapDefinition;
	m_activeSimulationDefinition = simulationDefinition;

	int numTilesX = GetRandomIntInRange(m_mapDefinition->m_width.min, m_mapDefinition->m_width.max);
	int numTilesY = GetRandomIntInRange(m_mapDefinition->m_height.min, m_mapDefinition->m_height.max);

	m_dimensions = IntVector2(numTilesX, numTilesY);

	for (int yCoordinate = 0; yCoordinate < numTilesY; yCoordinate++)
	{
		for (int xCoordinate = 0; xCoordinate < numTilesX; xCoordinate++)
		{		
			Tile* newTile = new Tile();

			newTile->m_tileCoords = IntVector2(xCoordinate, yCoordinate);
			newTile->m_tileDefinition = TileDefinition::s_tileDefinitions[m_mapDefinition->m_defaultTile->m_name];

			newTile->Initialize();

			m_tiles.push_back(newTile);
			newTile = nullptr;
		}
	}

	for (int genStepsIndex = 0; genStepsIndex < (int)m_mapDefinition->m_genSteps.size(); genStepsIndex++)
	{
		int iterations = GetRandomIntInRange(m_mapDefinition->m_iterations.min, m_mapDefinition->m_iterations.max);
		float chanceToRun = GetRandomFloatZeroToOne();
		if (chanceToRun <= m_mapDefinition->m_chanceToRun)
		{
			for (int iterationIndex = 0; iterationIndex < iterations; iterationIndex++)
			{
				m_mapDefinition->m_genSteps[genStepsIndex]->Run(*this);
			}
		}
	}

	m_mapWorldBounds = AABB2(0.f, 0.f, m_dimensions.x * g_tileSize, m_dimensions.y * g_tileSize);
}

//  =========================================================================================
Map::~Map()
{
	//cleanup reference pointers
	m_mapDefinition = nullptr;
	m_activeSimulationDefinition = nullptr;
	m_playingState = nullptr;

	//cleanuip timers
	delete(m_sortTimer);
	m_sortTimer = nullptr;

	delete(m_threatTimer);
	m_threatTimer = nullptr;	

	delete(m_bombardmentTimer);
	m_bombardmentTimer = nullptr;	
	
	//delete grid
	delete(m_mapAsGrid);
	m_mapAsGrid = nullptr;

	//delete mesh
	delete(m_mapMesh);
	m_mapMesh = nullptr;

	delete(m_debugMapMesh);
	m_debugMapMesh = nullptr;

	//cleanup bombardments
	for (int bombardmentIndex = 0; bombardmentIndex < (int)m_activeBombardments.size(); ++bombardmentIndex)
	{
		delete(m_activeBombardments[bombardmentIndex]);
		m_activeBombardments[bombardmentIndex] = nullptr;
	}
	m_activeBombardments.clear();

	//cleanup fires
	for (int fireIndex = 0; fireIndex < (int)m_fires.size(); ++fireIndex)
	{
		delete(m_fires[fireIndex]);
		m_fires[fireIndex] = nullptr;
	}
	m_fires.clear();

	//cleanup points of interest
	for (int poiIndex = 0; poiIndex < (int)m_armories.size(); ++poiIndex)
	{
		m_armories[poiIndex] = nullptr;
	}
	m_armories.clear();

	for (int poiIndex = 0; poiIndex < (int)m_lumberyards.size(); ++poiIndex)
	{
		m_lumberyards[poiIndex] = nullptr;
	}
	m_lumberyards.clear();

	for (int poiIndex = 0; poiIndex < (int)m_medStations.size(); ++poiIndex)
	{
		m_medStations[poiIndex] = nullptr;
	}
	m_medStations.clear();

	for (int poiIndex = 0; poiIndex < (int)m_wells.size(); ++poiIndex)
	{
		m_wells[poiIndex] = nullptr;
	}
	m_wells.clear();

	for (int poiIndex = 0; poiIndex < (int)m_pointsOfInterest.size(); ++poiIndex)
	{
		delete(m_pointsOfInterest[poiIndex]);
		m_pointsOfInterest[poiIndex] = nullptr;
	}
	m_pointsOfInterest.clear();

	//cleanup agents
	for (int agentIndex = 0; agentIndex < (int)m_agentsOrderedByXPosition.size(); ++agentIndex)
	{
		m_agentsOrderedByXPosition[agentIndex] = nullptr;
	}
	m_agentsOrderedByXPosition.clear();

	for (int agentIndex = 0; agentIndex < (int)m_agentsOrderedByYPosition.size(); ++agentIndex)
	{
		m_agentsOrderedByYPosition[agentIndex] = nullptr;
	}
	m_agentsOrderedByYPosition.clear();

	for (int agentIndex = 0; agentIndex < (int)m_agentsOrderedByPriority.size(); ++agentIndex)
	{
		delete(m_agentsOrderedByPriority[agentIndex]);
		m_agentsOrderedByPriority[agentIndex] = nullptr;
	}
	m_agentsOrderedByPriority.clear();

	//tiles
	for (int tileIndex = 0; tileIndex < (int)m_tiles.size(); ++tileIndex)
	{
		delete(m_tiles[tileIndex]);
		m_tiles[tileIndex] = nullptr;
	}
	m_tiles.clear();
}

//  =========================================================================================
void Map::Initialize()
{
	g_maxCoordinateDistanceSquared = GetDistanceSquared(IntVector2::ZERO, GetDimensions());

	//if we don't alreadfy have an active definition, get the top one
	if (m_activeSimulationDefinition == nullptr)
	{
		m_activeSimulationDefinition = SimulationDefinition::GetSimulationByName("");
	}		

	//create armories
	for (int armoryIndex = 0; armoryIndex < m_activeSimulationDefinition->m_numArmories; ++armoryIndex)
	{
		//add random point of interest
		PointOfInterest* poiLocation = GeneratePointOfInterest(ARMORY_POI_TYPE);
		poiLocation->m_map = this;

		m_pointsOfInterest.push_back(poiLocation);
		m_armories.push_back(poiLocation);
	}

	//create lumberyards
	for (int lumberyardIndex = 0; lumberyardIndex < m_activeSimulationDefinition->m_numArmories; ++lumberyardIndex)
	{
		//add random point of interest
		PointOfInterest* poiLocation = GeneratePointOfInterest(LUMBERYARD_POI_TYPE);
		poiLocation->m_map = this;

		m_lumberyards.push_back(poiLocation);
		m_pointsOfInterest.push_back(poiLocation);
	}

	//create medstations
	for (int medStationIndex = 0; medStationIndex < m_activeSimulationDefinition->m_numMedStations; ++medStationIndex)
	{
		//add random point of interest
		PointOfInterest* poiLocation = GeneratePointOfInterest(MED_STATION_POI_TYPE);
		poiLocation->m_map = this;

		m_medStations.push_back(poiLocation);
		m_pointsOfInterest.push_back(poiLocation);
	}

	//create wells
	for (int wellIndex = 0; wellIndex < m_activeSimulationDefinition->m_numWells; ++wellIndex)
	{
		//add random point of interest
		PointOfInterest* poiLocation = GeneratePointOfInterest(WELL_POI_TYPE);
		poiLocation->m_map = this;

		m_wells.push_back(poiLocation);
		m_pointsOfInterest.push_back(poiLocation);
	}

	TODO("Add POI access points after everything is created in the map");

	IntVector2 dimensions = GetDimensions();
	AABB2 mapBounds = AABB2(Vector2::ZERO, Vector2(dimensions));

	//create agents
	for (int agentIndex = 0; agentIndex < m_activeSimulationDefinition->m_numAgents; ++agentIndex)
	{
		IsoSpriteAnimSet* animSet = nullptr;
		std::map<std::string, IsoSpriteAnimSetDefinition*>::iterator spriteDefIterator = IsoSpriteAnimSetDefinition::s_isoSpriteAnimSetDefinitions.find("agent");
		if (spriteDefIterator != IsoSpriteAnimSetDefinition::s_isoSpriteAnimSetDefinitions.end())
		{
			animSet = new IsoSpriteAnimSet(spriteDefIterator->second);
		}

		Vector2 randomStartingLocation = GetRandomNonBlockedPositionInMapBounds();
		Agent* agent = new Agent(randomStartingLocation, animSet, this);
		m_agentsOrderedByXPosition.push_back(agent);
		m_agentsOrderedByYPosition.push_back(agent);
		m_agentsOrderedByPriority.push_back(agent);

		agent->m_indexInSortedXList = agentIndex;
		agent->m_indexInSortedYList = agentIndex;

		animSet = nullptr;
		agent = nullptr;
	}

	//init other starting values
	m_threat = m_activeSimulationDefinition->m_startingThreat;

	//setup timeres
	m_bombardmentTimer = new Stopwatch(GetGameClock());
	m_bombardmentTimer->SetTimer(1.f / m_activeSimulationDefinition->m_bombardmentRatePerSecond);

	m_threatTimer = new Stopwatch(GetGameClock());
	m_threatTimer->SetTimer(1.f / m_activeSimulationDefinition->m_threatRatePerSecond);

	//sort agents for the first time
	m_sortTimer = new Stopwatch(g_sortTimerInSeconds, GetGameClock());

	SortAgentsByX();
	SortAgentsByY();

	CreateMapMesh();
	InitializeMapGrid();
	UpdateMapGrid();
}

//  =========================================================================================
void Map::Update(float deltaSeconds)
{
	PROFILER_PUSH();

	if(m_isMapGridDirty)
		UpdateMapGrid();

	//udpate timers
	if (m_threatTimer->DecrementAll() > 0)
	{
		if (m_threat != g_maxThreat)
		{
			m_threat++;
		}		
	}		

	UpdateAgents(deltaSeconds);

	//get timer excluding how long it took to update the agent
	SimpleTimer nonAgentUpdateTimer;
	nonAgentUpdateTimer.Start();

	// Update bombardments
	if (m_bombardmentTimer->DecrementAll() > 0)
	{
		Bombardment* bombardment = new Bombardment(GetWorldPositionOfMapCoordinate(GetRandomCoordinateInMapBounds()));
		m_activeBombardments.push_back(bombardment);
	}

	for (int bombardmentIndex = 0; bombardmentIndex < (int)m_activeBombardments.size(); ++bombardmentIndex)
	{
		m_activeBombardments[bombardmentIndex]->Update(deltaSeconds);
	}

	nonAgentUpdateTimer.Stop();
	g_previousFrameNonAgentUpdateTime = nonAgentUpdateTimer.GetRunningTime();
}

//  =============================================================================
void Map::UpdateAgents(float deltaSeconds)
{
	//update agents
	PROFILER_PUSH();

	if (!GetIsAgentUpdateBudgeted())
	{
		for (int agentIndex = 0; agentIndex < (int)m_agentsOrderedByXPosition.size(); ++agentIndex)
		{
			m_agentsOrderedByXPosition[agentIndex]->Update(deltaSeconds);
			DetectAgentToTileCollision(m_agentsOrderedByXPosition[agentIndex]);
		}
	}
	else
	{
		UpdateAgentsBudgeted(deltaSeconds);
	}

	//check optimization flags
	//sort for Y drawing for render AND for next frame's agent update
	if (GetIsOptimized())
	{		
		if (m_sortTimer->CheckAndReset())
		{
			SortAgentsByX();
			SortAgentsByY();
		}		
	}
}

//  =============================================================================
void Map::UpdateAgentsBudgeted(float deltaSeconds)
{
	PROFILER_PUSH();

	int64_t remainingAgentUpdateBudget = int64_t(g_perFrameHPCBudget - g_previousFrameNonAgentUpdateTime - g_previousFrameRenderTime);
	g_agentsUpdatedThisFrame = 0;

	//if this is the first frame, there is no point in sorting because we have no criteria to decide on
	if (g_previousFrameNonAgentUpdateTime != 0 && g_previousFrameRenderTime != 0)
	{
		QuickSortAgentByPriority(m_agentsOrderedByPriority, 0, (int)m_agentsOrderedByPriority.size() - 1);
	}	

	SimpleTimer agentUpdateTimer;
	bool canUpdate = true;
	for (int agentIndex = 0; agentIndex < (int)m_agentsOrderedByPriority.size(); ++agentIndex)
	{
		//if there is still budget left for update, continue..
		if (canUpdate)
		{
			agentUpdateTimer.Start();

			//do udpate work
			m_agentsOrderedByPriority[agentIndex]->Update(deltaSeconds);
			//DetectAgentToTileCollision(m_agentsOrderedByPriority[agentIndex]);

			agentUpdateTimer.Stop();
			uint64_t totalUpdateTime = agentUpdateTimer.GetRunningTime();
			agentUpdateTimer.Reset();

			remainingAgentUpdateBudget -= totalUpdateTime;
			m_agentsOrderedByPriority[agentIndex]->ResetPriority();

			if (remainingAgentUpdateBudget <= 0)
			{
				canUpdate = false;
			}	

			g_agentsUpdatedThisFrame++;
		}	
		else //we are out of budgets
		{
			m_agentsOrderedByPriority[agentIndex]->QuickUpdate(deltaSeconds);
		}		

		DetectAgentToTileCollision(m_agentsOrderedByPriority[agentIndex]);
	}

	//sort if we are optimized
	if (GetIsOptimized())
	{		
		if (m_sortTimer->CheckAndReset())
		{
			SortAgentsByX();
			SortAgentsByY();
		}		
	}
}

//  =========================================================================================
void Map::Render()
{
	if(g_isQuitting)
		return;

	PROFILER_PUSH();

	Renderer* theRenderer = Renderer::GetInstance();

	//render tile mesh
	theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("Data/Images/Terrain_8x8.png"));
	theRenderer->SetShader(theRenderer->m_defaultShader);
	theRenderer->DrawMesh(m_mapMesh);

	//render tile block data
	if (g_isBlockedTileDataShown)
	{
		theRenderer->BindMaterial(theRenderer->CreateOrGetMaterial("text"));
		theRenderer->DrawMesh(m_debugMapMesh);
	}

	//create and render agent mesh
	Mesh* agentMesh = CreateDynamicAgentMesh();
	theRenderer->SetTexture(*theRenderer->CreateOrGetTexture(m_agentsOrderedByXPosition[0]->m_animationSet->GetCurrentSprite(m_agentsOrderedByXPosition[0]->m_spriteDirection)->m_definition->m_diffuseSource));
	theRenderer->SetShader(theRenderer->CreateOrGetShader("agents"));
	theRenderer->DrawMesh(agentMesh);

	//create and render text
	Mesh* textMesh = CreateTextMesh();	
	if (textMesh != nullptr)
	{
		theRenderer->BindMaterial(theRenderer->CreateOrGetMaterial("text"));
		theRenderer->DrawMesh(textMesh);
	}	

	//create and render bombardments
	if (m_activeBombardments.size() > 0)
	{
		Mesh* bombardmentMesh = CreateDynamicBombardmentMesh();
		theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("Data/Images/AirStrike.png"));
		theRenderer->SetShader(theRenderer->CreateOrGetShader("agents"));
		theRenderer->DrawMesh(bombardmentMesh);

		delete(bombardmentMesh);
		bombardmentMesh = nullptr;
	}

	//create and render fires
	if (m_fires.size() > 0)
	{
		Mesh* fireMesh = CreateDynamicFireMesh();
		theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("Data/Images/Fire.png"));
		theRenderer->SetShader(theRenderer->CreateOrGetShader("agents"));
		theRenderer->DrawMesh(fireMesh);

		delete(fireMesh);
		fireMesh = nullptr;
	}

	//set back to default
	theRenderer->SetTexture(*theRenderer->m_defaultTexture);
	theRenderer->SetShader(theRenderer->m_defaultShader);

	//cleanup
	delete(agentMesh);
	agentMesh = nullptr;

	theRenderer = nullptr;
}

//  =============================================================================
void Map::Reload(SimulationDefinition* definition)
{
	// Cleanup Step ----------------------------------------------
	m_activeSimulationDefinition = definition;
	m_playingState = nullptr;

	//cleanuip timers
	delete(m_sortTimer);
	m_sortTimer = nullptr;

	delete(m_threatTimer);
	m_threatTimer = nullptr;	

	delete(m_bombardmentTimer);
	m_bombardmentTimer = nullptr;

	//cleanup bombardments
	for (int bombardmentIndex = 0; bombardmentIndex < (int)m_activeBombardments.size(); ++bombardmentIndex)
	{
		delete(m_activeBombardments[bombardmentIndex]);
		m_activeBombardments[bombardmentIndex] = nullptr;
	}
	m_activeBombardments.clear();

	//cleanup fires
	for (int fireIndex = 0; fireIndex < (int)m_fires.size(); ++fireIndex)
	{
		delete(m_fires[fireIndex]);
		m_fires[fireIndex] = nullptr;
	}
	m_fires.clear();

	//cleanup agents
	for (int agentIndex = 0; agentIndex < (int)m_agentsOrderedByXPosition.size(); ++agentIndex)
	{
		m_agentsOrderedByXPosition[agentIndex] = nullptr;		
	}
	m_agentsOrderedByXPosition.clear();

	for (int agentIndex = 0; agentIndex < (int)m_agentsOrderedByYPosition.size(); ++agentIndex)
	{
		delete(m_agentsOrderedByYPosition[agentIndex]);
		m_agentsOrderedByYPosition[agentIndex] = nullptr;
	}
	m_agentsOrderedByYPosition.clear();

	for (int agentIndex = 0; agentIndex < (int)m_agentsOrderedByPriority.size(); ++agentIndex)
	{
		delete(m_agentsOrderedByPriority[agentIndex]);
		m_agentsOrderedByPriority[agentIndex] = nullptr;
	}
	m_agentsOrderedByPriority.clear();

	// Reload step ----------------------------------------------

	//create agents
	for (int agentIndex = 0; agentIndex < m_activeSimulationDefinition->m_numAgents; ++agentIndex)
	{
		IsoSpriteAnimSet* animSet = nullptr;
		std::map<std::string, IsoSpriteAnimSetDefinition*>::iterator spriteDefIterator = IsoSpriteAnimSetDefinition::s_isoSpriteAnimSetDefinitions.find("agent");
		if (spriteDefIterator != IsoSpriteAnimSetDefinition::s_isoSpriteAnimSetDefinitions.end())
		{
			animSet = new IsoSpriteAnimSet(spriteDefIterator->second);
		}

		Vector2 randomStartingLocation = GetRandomNonBlockedPositionInMapBounds();
		Agent* agent = new Agent(randomStartingLocation, animSet, this);
		m_agentsOrderedByXPosition.push_back(agent);
		m_agentsOrderedByYPosition.push_back(agent);

		agent->m_indexInSortedXList = agentIndex;
		agent->m_indexInSortedYList = agentIndex;

		animSet = nullptr;
		agent = nullptr;
	}

	//init other starting values
	m_threat = m_activeSimulationDefinition->m_startingThreat;

	//setup timeres
	m_bombardmentTimer = new Stopwatch(GetGameClock());
	m_bombardmentTimer->SetTimer(1.f / m_activeSimulationDefinition->m_bombardmentRatePerSecond);

	m_threatTimer = new Stopwatch(GetGameClock());
	m_threatTimer->SetTimer(1.f / m_activeSimulationDefinition->m_threatRatePerSecond);

	//sort agents for the first time
	m_sortTimer = new Stopwatch(g_sortTimerInSeconds, GetGameClock());

	SortAgentsByX();
	SortAgentsByY();	
}

//  =========================================================================================
void Map::CreateMapMesh()
{
	PROFILER_PUSH();

	MeshBuilder builder;
	
	//create mesh for static tiles and buildings
	for (int tileIndex = 0; tileIndex < (int)m_tiles.size(); ++tileIndex)
	{
		builder.CreateTexturedQuad2D(m_tiles[tileIndex]->GetBounds().GetCenter(), Vector2::ONE, m_tiles[tileIndex]->m_tileDefinition->m_baseSpriteUVCoords.mins, m_tiles[tileIndex]->m_tileDefinition->m_baseSpriteUVCoords.maxs, m_tiles[tileIndex]->m_tint );
	}

	m_mapMesh = builder.CreateMesh<VertexPCU>();

	CreateDebugMapMesh();
}

//  =========================================================================================
void Map::CreateDebugMapMesh()
{
	PROFILER_PUSH();

	if (m_debugMapMesh != nullptr)
	{
		delete(m_debugMapMesh);
		m_debugMapMesh = nullptr;
	}

	MeshBuilder builder;

	//create debug mesh to show blocking states
	for (int tileIndex = 0; tileIndex < (int)m_tiles.size(); ++tileIndex)
	{
		int value = m_tiles[tileIndex]->m_tileDefinition->m_allowsWalking == true ? 0 : 1;
		std::string doesBlock = Stringf("%i", value);

		builder.CreateText2DInAABB2(m_tiles[tileIndex]->GetBounds().GetCenter(), Vector2::ONE, 1.f, doesBlock, m_tiles[tileIndex]->m_tint );
	}

	m_debugMapMesh = builder.CreateMesh<VertexPCU>();
}

//  =========================================================================================
Mesh* Map::CreateDynamicAgentMesh()
{
	PROFILER_PUSH();

	MeshBuilder builder = MeshBuilder();

	//create mesh for static tiles and buildings
	for (int agentIndex = 0; agentIndex < (int)m_agentsOrderedByYPosition.size(); ++agentIndex)
	{
		Sprite sprite = *m_agentsOrderedByXPosition[agentIndex]->m_animationSet->GetCurrentSprite(m_agentsOrderedByXPosition[agentIndex]->m_spriteDirection);

		Rgba agentColor = Rgba::WHITE;
		switch (m_agentsOrderedByXPosition[agentIndex]->m_planner->m_currentPlan.m_chosenPlanType)
		{
		case GATHER_ARROWS_PLAN_TYPE:
			agentColor = GATHER_ARROWS_TINT;
			break;
		case GATHER_LUMBER_PLAN_TYPE:
			agentColor = GATHER_LUMBER_TINT;
			break;
		case GATHER_BANDAGES_PLAN_TYPE:
			agentColor = GATHER_BANDAGES_TINT;
			break;
		case GATHER_WATER_PLAN_TYPE:
			agentColor = GATHER_WATER_TINT;
			break;
		case SHOOT_PLAN_TYPE:
			agentColor = SHOOT_TINT;
			break;
		case REPAIR_PLAN_TYPE:
			agentColor = REPAIR_TINT;
			break;
		case HEAL_PLAN_TYPE:
			agentColor = HEAL_TINT;
			break;
		case FIGHT_FIRE_PLAN_TYPE:
			agentColor = PUT_OUT_FIRE_TINT;
			break;
		}

		//AABB2 bounds = m_agentsOrderedByXPosition[agentIndex]->m_spriteRenderBounds;
		Vector2 agentSize = Vector2::ONE;

		if (g_isDebugDataShown)
		{
			if(m_agentsOrderedByXPosition[agentIndex] == m_playingState->m_disectedAgent)
				agentSize = Vector2(1.75f, 1.75f);
		}	

		builder.CreateTexturedQuad2D(m_agentsOrderedByXPosition[agentIndex]->m_position,
			agentSize, 
			Vector2(sprite.GetNormalizedUV().mins.x, sprite.GetNormalizedUV().maxs.y), 
			Vector2(sprite.GetNormalizedUV().maxs.x, sprite.GetNormalizedUV().mins.y), 
			agentColor);
	}

	Mesh* agentMesh = builder.CreateMesh<VertexPCU>();
	return agentMesh;
}


//  =============================================================================
Mesh* Map::CreateTextMesh()
{
	PROFILER_PUSH();

	MeshBuilder builder = MeshBuilder();
	Mesh* textMesh = nullptr;

	//agent ids
	if (g_isIdShown)
	{
		for (int agentIndex = 0; agentIndex < (int)m_agentsOrderedByYPosition.size(); ++agentIndex)
		{
			builder.CreateText2DInAABB2(Vector2(m_agentsOrderedByXPosition[agentIndex]->m_position.x, m_agentsOrderedByXPosition[agentIndex]->m_position.y + 0.7), Vector2(0.25f, 0.25f), 1.f, Stringf("%i", m_agentsOrderedByXPosition[agentIndex]->m_id), Rgba::WHITE);
		}
	}

	//  ----------------------------------------------
	//draw other things
	//  ----------------------------------------------

	if (builder.m_vertices.size() > 0)
	{
		textMesh = builder.CreateMesh<VertexPCU>();
	}	

	return textMesh;
}

//  =========================================================================================
Mesh* Map::CreateDynamicBombardmentMesh()
{
	PROFILER_PUSH();

	MeshBuilder builder;

	for (int bombardmentIndex = 0; bombardmentIndex < (int)m_activeBombardments.size(); ++bombardmentIndex)
	{	
		AABB2 bombardmentBox = AABB2(m_activeBombardments[bombardmentIndex]->m_disc.center, m_activeBombardments[bombardmentIndex]->m_disc.radius, m_activeBombardments[bombardmentIndex]->m_disc.radius);
		builder.CreateTexturedQuad2D(m_activeBombardments[bombardmentIndex]->m_disc.center, bombardmentBox.GetDimensions(), Vector2::ZERO, Vector2::ONE, Rgba(1.f, 1.f, 1.f, .5f));
	}

	Mesh* bombardmentMesh = builder.CreateMesh<VertexPCU>();

	return bombardmentMesh;
}

//  =========================================================================================
Mesh* Map::CreateDynamicFireMesh()
{
	PROFILER_PUSH();

	MeshBuilder builder;;

	for (int fireIndex = 0; fireIndex < (int)m_fires.size(); ++fireIndex)
	{	
		Vector2 fireCenter = Vector2(m_fires[fireIndex]->m_coordinate) + Vector2(0.5f, 0.6f);
		AABB2 fireBox = AABB2(fireCenter, 0.5f, 0.5f);
		builder.CreateTexturedQuad2D(fireBox.GetCenter(), fireBox.GetDimensions(), Vector2::ZERO, Vector2::ONE, Rgba(1.f, 1.f, 1.f, 0.7f));
	}

	Mesh* fireMesh = builder.CreateMesh<VertexPCU>();

	//cleanup
	return fireMesh;
}

//  =========================================================================================
void Map::DeleteDeadEntities()
{
	PROFILER_PUSH();

	DeleteDeadFires();
	DeleteDeadBombardmentsAndRandomlyStartFire();
}

//  =========================================================================================
void Map::DeleteDeadFires()
{
	Game* theGame = Game::GetInstance();

	for (int fireIndex = 0; fireIndex < (int)m_fires.size(); ++fireIndex)
	{
		if (m_fires[fireIndex]->IsDead())
		{
			Tile* fireTile = GetTileAtWorldPosition(m_fires[fireIndex]->m_worldPosition);

			ASSERT_OR_DIE(fireTile != nullptr, "FIRE TILE IS INVALID ON DELETION");

			fireTile->m_tileDefinition = TileDefinition::s_tileDefinitions.find("Ground")->second;
			m_fires.erase(m_fires.begin() + fireIndex);
			--fireIndex;
		}			
	}
}

//  =========================================================================================
void Map::DeleteDeadBombardmentsAndRandomlyStartFire()
{
	Game* theGame = Game::GetInstance();
	for (int bombardmentIndex = 0; bombardmentIndex < (int)m_activeBombardments.size(); ++bombardmentIndex)
	{
		if (m_activeBombardments[bombardmentIndex]->IsExplosionComplete())
		{
			//check collision to all characters and buildings
			DetectBombardmentToAgentCollision(m_activeBombardments[bombardmentIndex]);
			DetectBombardmentToPOICollision(m_activeBombardments[bombardmentIndex]);

			if (DoesBombardmentStartFire())
			{
				IntVector2 tileCoordinate = GetTileCoordinateOfPosition(m_activeBombardments[bombardmentIndex]->m_disc.center);
				Tile* tile = GetTileAtCoordinate(tileCoordinate);
				
				if (tile->m_tileDefinition->m_allowsWalking && tile->m_tileDefinition->m_allowsBuilding)
				{
					SpawnFire(tile);
				}				
			}

			m_activeBombardments.erase(m_activeBombardments.begin() + bombardmentIndex);
			--bombardmentIndex;
		}
	}
}

// Bubble sort. Once sorted, will rarely require more than one pass =========================================================================================
void Map::SortAgentsByX()
{
	//PROFILER_PUSH();

	int i = 0;
	int j = 0;
	bool swapped = false;

	for (int i = 0; i < (int)m_agentsOrderedByXPosition.size() - 1; ++i)
	{
		swapped = false;
		for (int j = 0; j < (int)m_agentsOrderedByXPosition.size() - i - 1; ++j)
		{
			if(m_agentsOrderedByXPosition[j]->m_position.x > m_agentsOrderedByXPosition[j+1]->m_position.x)
			{
				SwapAgents(j, j+1, X_AGENT_SORT_TYPE);
				swapped = true;
			}
		}

		//if none were swapped on the inner loop, we are sorted.
		if(swapped == false)
			break;
	}
}

// Bubble sort. Once sorted, will rarely require more than one pass =========================================================================================
void Map::SortAgentsByY()
{
	//PROFILER_PUSH();

	int i = 0;
	int j = 0;
	bool swapped = false;

	for (int i = 0; i < (int)m_agentsOrderedByYPosition.size(); ++i)
	{
		swapped = false;
		for (int j = 0; j < (int)m_agentsOrderedByYPosition.size() - i - 1; ++j)
		{
			if (m_agentsOrderedByYPosition[j]->m_position.y > m_agentsOrderedByYPosition[j + 1]->m_position.y)
			{
				SwapAgents(j, j + 1, Y_AGENT_SORT_TYPE);
				swapped = true;
			}
		}

		//if none were swappe don the inner loop, we are sorted.
		if (swapped == false)
			break;
	}
}

//  =============================================================================
void Map::QuickSortAgentByPriority(std::vector<Agent*>& agents, int startIndex, int endIndex)
{
	if (startIndex < endIndex)
	{
		int divisorIndex = QuickSortPivot(agents, startIndex, endIndex);

		QuickSortAgentByPriority(agents, startIndex, divisorIndex - 1);  //sort everything BEFORE the split
		QuickSortAgentByPriority(agents, divisorIndex + 1, endIndex);	   //sort everything AFTER the split
	}
}

//  =============================================================================
int Map::QuickSortPivot(std::vector<Agent*>& agents, int startIndex, int endIndex)
{
	int pivotValue = agents[endIndex]->m_updatePriority;

	int lessIndex = (startIndex - 1);

	for (int agentIndex = startIndex; agentIndex <= endIndex - 1; ++agentIndex)
	{
		//only swap if agent priority is higher than pivot
		if (agents[agentIndex]->m_updatePriority > pivotValue)
		{
			++lessIndex;

			SwapAgents(lessIndex, agentIndex, PRIORITY_AGENT_SORT_TYPE);						
		}
	}

	//final swap for pivot
	SwapAgents(lessIndex + 1, endIndex, PRIORITY_AGENT_SORT_TYPE);
	return lessIndex + 1;
}

//  =========================================================================================
void Map::SwapAgents(int indexI, int indexJ, eAgentSortType type)
{
	Agent* tempAgent = nullptr;

	//swap stored indices
	switch (type)
	{
	case X_AGENT_SORT_TYPE:
		tempAgent = m_agentsOrderedByXPosition[indexI];

		//swap first
		m_agentsOrderedByXPosition[indexI] = m_agentsOrderedByXPosition[indexJ];
		m_agentsOrderedByXPosition[indexJ] = tempAgent;

		//set new indexes
		m_agentsOrderedByXPosition[indexI]->m_indexInSortedXList = indexI;
		m_agentsOrderedByXPosition[indexJ]->m_indexInSortedXList = indexJ;
		break;
	case Y_AGENT_SORT_TYPE:
		tempAgent = m_agentsOrderedByYPosition[indexI];

		//swap first
		m_agentsOrderedByYPosition[indexI] = m_agentsOrderedByYPosition[indexJ];
		m_agentsOrderedByYPosition[indexJ] = tempAgent;

		//set new indexes
		m_agentsOrderedByYPosition[indexI]->m_indexInSortedYList = indexI;
		m_agentsOrderedByYPosition[indexJ]->m_indexInSortedYList = indexJ;
		break;
	case PRIORITY_AGENT_SORT_TYPE:
		tempAgent = m_agentsOrderedByPriority[indexI];

		//swap first
		m_agentsOrderedByPriority[indexI] = m_agentsOrderedByPriority[indexJ];
		m_agentsOrderedByPriority[indexJ] = tempAgent;

		//set new indexes
		m_agentsOrderedByPriority[indexI]->m_indexInPriorityList = indexI;
		m_agentsOrderedByPriority[indexJ]->m_indexInPriorityList = indexJ;
		break;
	}

	tempAgent = nullptr;
}

//  =========================================================================================
void Map::GetAsGrid(Grid<int>& outMapGrid)
{
	outMapGrid.InitializeGrid(0, m_dimensions.x, m_dimensions.y);

	for (int tileIndex = 0; tileIndex < (int)m_tiles.size(); tileIndex++)
	{
		int value = m_tiles[tileIndex]->m_tileDefinition->m_allowsWalking ? 0 : 1;

		if(value != 0)
			outMapGrid.SetValueAtIndex(value, tileIndex);
	}
}

//  =========================================================================================
void Map::InitializeMapGrid()
{
	m_mapAsGrid = new Grid<int>();
	m_mapAsGrid->InitializeGrid(0, m_dimensions.x, m_dimensions.y);
}

//  =========================================================================================
void Map::UpdateMapGrid()
{
	PROFILER_PUSH();

	for (int tileIndex = 0; tileIndex < (int)m_tiles.size(); tileIndex++)
	{
		int value = m_tiles[tileIndex]->m_tileDefinition->m_allowsWalking ? 0 : 1;

		if(value != 0)
			m_mapAsGrid->SetValueAtIndex(value, tileIndex);
	}

	//redraw debug mesh
	if(g_isDebugDataShown)
		CreateDebugMapMesh();

	m_isMapGridDirty = false;
}

//  =========================================================================================
bool Map::IsTileBlockingAtCoordinate(const IntVector2& coordinate)
{
	bool isBlocking = !GetTileAtCoordinate(coordinate)->m_tileDefinition->m_allowsWalking;
	return isBlocking;	
}

//  =========================================================================================
bool Map::DoesTilePreventBuilding(const IntVector2& coordinate)
{
	bool doesAllowBuilding = !GetTileAtCoordinate(coordinate)->m_tileDefinition->m_allowsBuilding;
	return doesAllowBuilding;	
}

//  =========================================================================================
Tile* Map::GetTileAtCoordinate(const IntVector2& coordinate)
{
	//handle bad coordinates
	if(coordinate.x >= m_dimensions.x || coordinate.x < 0 || coordinate.y >= m_dimensions.y || coordinate.y < 0)
		return nullptr;

	return m_tiles[coordinate.x + (coordinate.y * m_dimensions.x)];
}

//  =========================================================================================
Tile* Map::GetTileAtWorldPosition(const Vector2& position)
{
	IntVector2 cooordinateOfPosition = GetTileCoordinateOfPosition(position);

	return GetTileAtCoordinate(cooordinateOfPosition);
}

//  =========================================================================================
Vector2 Map::GetCenter()
{
	return Vector2(m_dimensions) * 0.5f;
}

//  =========================================================================================
Agent* Map::GetAgentById(int agentId)
{
	//invalid input
	if (agentId > m_agentsOrderedByPriority.size() - 1 || agentId < 0)
	{
		return nullptr;
	}

	//we know the agent exists. Search for them
	for(int agentIndex = 0; agentIndex < m_agentsOrderedByPriority.size(); ++agentIndex)
	{
		if (m_agentsOrderedByPriority[agentIndex]->m_id == agentId)
		{
			return m_agentsOrderedByPriority[agentIndex];
		}
	}
}

//  =========================================================================================
PointOfInterest* Map::GetPointOfInterestById(int poiId)
{
	for (int poiIndex = 0; poiIndex < (int)m_pointsOfInterest.size(); ++poiIndex)
	{
		if(poiId == m_pointsOfInterest[poiIndex]->m_id)
			return m_pointsOfInterest[poiIndex];
	}

	//if we never found the poi, return nullptr;
	return nullptr;
}

//  =========================================================================================
void Map::DetectBombardmentToAgentCollision(Bombardment* bombardment)
{
	for (int agentIndex = 0; agentIndex < (int)m_agentsOrderedByXPosition.size(); ++agentIndex)
	{
		if (bombardment->m_disc.IsPointInside(m_agentsOrderedByXPosition[agentIndex]->m_position))
		{
			m_agentsOrderedByXPosition[agentIndex]->TakeDamage(g_bombardmentDamage);
		}
	}
}

//  =========================================================================================
void Map::DetectBombardmentToPOICollision(Bombardment* bombardment)
{
	for (int poiIndex = 0; poiIndex < (int)m_pointsOfInterest.size(); ++poiIndex)
	{
		AABB2 poiBounds = m_pointsOfInterest[poiIndex]->GetWorldBounds();
		if (DoesDiscOverlapWithAABB2(bombardment->m_disc, poiBounds))
		{
			m_pointsOfInterest[poiIndex]->TakeDamage(g_bombardmentDamage);
		}
	}
}

//  =========================================================================================
bool Map::DoesBombardmentStartFire()
{
	//Game::GetGlobalRNG()->GetRandomZeroToOne()
	return GetRandomFloatInRange(0.f, 1.f) >= RANDOM_FIRE_THRESHOLD ? true : false;
}


//  =========================================================================================
Fire* Map::GetFireById(int entityId)
{
	for (int fireIndex = 0; fireIndex < (int)m_fires.size(); ++fireIndex)
	{
		if(entityId == m_fires[fireIndex]->m_id)
			return m_fires[fireIndex];
	}

	//if we never found the fire, return nullptr;
	return nullptr;
}

//  =========================================================================================
void Map::SpawnFire(Tile* spawnTile)
{
	spawnTile->m_tileDefinition = TileDefinition::s_tileDefinitions.find("Fire")->second;
	Fire* fire = new Fire(spawnTile->m_tileCoords, this);
	m_fires.push_back(fire);

	m_isMapGridDirty = true;
}

//  =========================================================================================
void Map::DetectAgentToTileCollision(Agent* agent)
{
	

#ifdef CollisionDataAnalysis
	// profiling ----------------------------------------------
	g_collisionAnalysisData->Start();
	//  ----------------------------------------------
#endif

	IntVector2 agentTileCoord = GetTileCoordinateOfPosition(agent->m_position);
	Vector2 agentPosition = agent->m_position;
	bool didPushOutCorner = false;
	bool didPushOutAxis = false;

	//handle northeast
	IntVector2 tileDirection = IntVector2(1,1);
	IntVector2 tileCoord = agentTileCoord + tileDirection;
	didPushOutCorner = PushAgentOutOfTile(agent, tileCoord, NORTHEAST_TILE_DIRECTION);

	if (!didPushOutCorner)
	{
		//handle southeast
		tileDirection = IntVector2(1,-1);
		tileCoord = agentTileCoord + tileDirection;
		didPushOutCorner = PushAgentOutOfTile(agent, tileCoord, SOUTHEAST_TILE_DIRECTION);
	}

	if (!didPushOutCorner)
	{
		//handle northwest
		tileDirection = IntVector2(-1,1);
		tileCoord = agentTileCoord + tileDirection;
		didPushOutCorner = PushAgentOutOfTile(agent, tileCoord, NORTHWEST_TILE_DIRECTION);
	}

	if (!didPushOutCorner)
	{
		//handle southwest
		tileDirection = IntVector2(-1,-1);
		tileCoord = agentTileCoord + tileDirection;
		didPushOutCorner = PushAgentOutOfTile(agent, tileCoord, SOUTHWEST_TILE_DIRECTION);
	}
 
	// handle up,down,left,right push out (because we've done corners, we can skip scenarios where we have both) ----------------------------------------------
	//handle north
	tileDirection = IntVector2(0,1);
	tileCoord = agentTileCoord + tileDirection;
	didPushOutAxis = PushAgentOutOfTile(agent, tileCoord, NORTH_TILE_DIRECTION);

	//handle south
	if (!didPushOutAxis)
	{
		//handle southeast
		tileDirection = IntVector2(0,-1);
		tileCoord = agentTileCoord + tileDirection;
		didPushOutAxis = PushAgentOutOfTile(agent, tileCoord, SOUTH_TILE_DIRECTION);
	}

	//handle east
	if (!didPushOutAxis)
	{
		//handle southeast
		tileDirection = IntVector2(1,0);
		tileCoord = agentTileCoord + tileDirection;
		didPushOutAxis = PushAgentOutOfTile(agent, tileCoord, EAST_TILE_DIRECTION);
	}

	//handle west
	if (!didPushOutAxis)
	{
		//handle southeast
		tileDirection = IntVector2(-1,0);
		tileCoord = agentTileCoord + tileDirection;
		didPushOutAxis = PushAgentOutOfTile(agent, tileCoord, WEST_TILE_DIRECTION);
	}

	if (didPushOutAxis || didPushOutCorner)
	{
		int i = 0;
	}

#ifdef CollisionDataAnalysis
	// profiling ----------------------------------------------
	g_collisionAnalysisData->End();
	//  ----------------------------------------------
#endif
}

//  =========================================================================================
bool Map::PushAgentOutOfTile(Agent* agent, const IntVector2& tileCoordinate, int tileDirection)
{
	//early outs
	Tile* tile = GetTileAtCoordinate(tileCoordinate);
	if(tile == nullptr)
		return false;

	if(tile->m_tileDefinition->m_allowsWalking)
		return false;

	Vector2 agentCenter = agent->m_position;

		Vector2 collisionPoint;
		switch(tileDirection)
		{
		case EAST_TILE_DIRECTION:
			collisionPoint = Vector2((float)tileCoordinate.x, agentCenter.y);
			break;
		case WEST_TILE_DIRECTION:
			collisionPoint = Vector2((float)tileCoordinate.x + 1, agentCenter.y);
			break;
		case NORTH_TILE_DIRECTION:
			collisionPoint = Vector2(agentCenter.x, (float)tileCoordinate.y);
			break;
		case SOUTH_TILE_DIRECTION:
			collisionPoint = Vector2(agentCenter.x, (float)tileCoordinate.y + 1.f);
			break;
		case NORTHEAST_TILE_DIRECTION:
			collisionPoint = Vector2((float)tileCoordinate.x, (float)tileCoordinate.y);
			break;
		case NORTHWEST_TILE_DIRECTION:
			collisionPoint = Vector2((float)tileCoordinate.x + 1.f, (float)tileCoordinate.y);
			break;
		case SOUTHEAST_TILE_DIRECTION:
			collisionPoint = Vector2((float)tileCoordinate.x, (float)tileCoordinate.y + 1.f);
			break;
		case SOUTHWEST_TILE_DIRECTION:
			collisionPoint = Vector2((float)tileCoordinate.x + 1.f, (float)tileCoordinate.y + 1.f);
			break;
		}

		bool isColliding =  agent->m_physicsDisc.IsPointInside(collisionPoint);
		if(isColliding)
		{
			float displacement = GetDistance(agentCenter, collisionPoint);

			float amountToPush = agent->m_physicsDisc.radius - displacement;
			Vector2 pushOutDirection = (agentCenter - collisionPoint).GetNormalized();

			agent->m_position += pushOutDirection * (amountToPush);
			agent->UpdatePhysicsData();
		}	

		return isColliding;
}

//  =========================================================================================
PointOfInterest* Map::GeneratePointOfInterest(int poiType)
{
	ePointOfInterestType type = (ePointOfInterestType)poiType;

	//determine if random location is unblocked
	bool isLocationValid = false;
	IntVector2 randomCoordinate = IntVector2::ONE;
	IntVector2 accessCoordinate = IntVector2::NEGATIVE_ONE;

	std::vector<int> potentialAccessPoints = {0, 1, 2, 3, 4, 5, 6, 7, 8};

	int locationPlacementAttemptCount = 0;
	while (!isLocationValid)
	{
		ASSERT_OR_DIE(locationPlacementAttemptCount != 100, "NO SPACE TO PLACE POI!!!");

		randomCoordinate = IntVector2::ONE;
		accessCoordinate = IntVector2::NEGATIVE_ONE;
		randomCoordinate = GetRandomNonBlockedCoordinateInMapBounds();
		
		//check tile to north, to the right, and to the northeast to see if they are even good
		if(CheckIsCoordinateValid(IntVector2(randomCoordinate.x + 1, randomCoordinate.y)) 
			&& CheckIsCoordinateValid(IntVector2(randomCoordinate.x, randomCoordinate.y + 1))
			&& CheckIsCoordinateValid(IntVector2(randomCoordinate.x + 1, randomCoordinate.y + 1)))
		{
			//check tile to north, to the right, and to the northeast to see if they are blocked
			if (!DoesTilePreventBuilding(IntVector2(randomCoordinate.x + 1, randomCoordinate.y))
				&& !DoesTilePreventBuilding(IntVector2(randomCoordinate.x, randomCoordinate.y + 1))
				&& !DoesTilePreventBuilding(IntVector2(randomCoordinate.x + 1, randomCoordinate.y + 1)))
			{
				
				bool isAccessLocationValid =  false;
				int accessIterationAttemptCount = 0;

				ShuffleList(potentialAccessPoints);

				int accessIndex = 0;
				while (!isAccessLocationValid && accessIndex < (int)potentialAccessPoints.size())
				{
					switch (potentialAccessPoints[accessIndex])
					{
					case 0:
						accessCoordinate = IntVector2(randomCoordinate.x, randomCoordinate.y - 1);
						break;
					case 1:
						accessCoordinate = IntVector2(randomCoordinate.x + 1, randomCoordinate.y - 1);
						break;
					case 2:
						accessCoordinate = IntVector2(randomCoordinate.x + 2, randomCoordinate.y);
						break;
					case 3:
						accessCoordinate = IntVector2(randomCoordinate.x + 2, randomCoordinate.y + 1);
						break;
					case 4:
						accessCoordinate = IntVector2(randomCoordinate.x + 1, randomCoordinate.y + 2);
						break;
					case 5:
						accessCoordinate = IntVector2(randomCoordinate.x, randomCoordinate.y + 2);
						break;
					case 6:
						accessCoordinate = IntVector2(randomCoordinate.x - 1, randomCoordinate.y + 1);
						break;
					case 7:
						accessCoordinate = IntVector2(randomCoordinate.x - 1, randomCoordinate.y);
						break;
					}

					if (CheckIsCoordinateValid(accessCoordinate) && !DoesTilePreventBuilding(accessCoordinate))
					{
						isAccessLocationValid = true;
						isLocationValid = true;
					}

					if (!isAccessLocationValid)
					{
						++accessIndex;
					}											
				}

				if (isAccessLocationValid)
				{
					isLocationValid = true;
				}

				locationPlacementAttemptCount++;
			}
		}		
	}

	//Location is valid, therefore we can replace tiles with building tiles
	Rgba buildingColor = Rgba::WHITE;
	std::string spriteName = "Building";
	switch (poiType)
	{
	case ARMORY_POI_TYPE:
		buildingColor = Rgba::LIGHT_RED_TRANSPARENT;
		spriteName = "Armory";
		break;
	case LUMBERYARD_POI_TYPE:
		buildingColor = Rgba::LIGHT_YELLOW_TRANSPARENT;
		spriteName = "Lumberyard";
		break;
	case MED_STATION_POI_TYPE:
		buildingColor = Rgba::LIGHT_GREEN_TRANSPARENT;
		spriteName = "MedStation";
		break;
	case WELL_POI_TYPE:
		buildingColor = Rgba::LIGHT_BLUE_TRANSPARENT;
		spriteName = "Well";
		break;
	}

	//starting tile
	Tile* tile = GetTileAtCoordinate(randomCoordinate);
	tile->m_tileDefinition = TileDefinition::s_tileDefinitions.find(spriteName.c_str())->second;
	tile->m_tint = buildingColor;

	//tile to east
	tile = GetTileAtCoordinate(IntVector2(randomCoordinate.x + 1, randomCoordinate.y));
	tile->m_tileDefinition = TileDefinition::s_tileDefinitions.find(spriteName.c_str())->second;
	tile->m_tint = buildingColor;

	//tile to northeast
	tile = GetTileAtCoordinate(IntVector2(randomCoordinate.x + 1, randomCoordinate.y + 1));
	tile->m_tileDefinition = TileDefinition::s_tileDefinitions.find(spriteName.c_str())->second;
	tile->m_tint = buildingColor;

	//tile to north
	tile = GetTileAtCoordinate(IntVector2(randomCoordinate.x, randomCoordinate.y + 1));
	tile->m_tileDefinition = TileDefinition::s_tileDefinitions.find(spriteName.c_str())->second;
	tile->m_tint = buildingColor;

	tile = GetTileAtCoordinate(IntVector2(accessCoordinate.x, accessCoordinate.y));
	tile->m_tileDefinition = TileDefinition::s_tileDefinitions.find("BuildingAccess")->second;
	tile->m_tint = buildingColor;

	PointOfInterest* poi = new PointOfInterest(type, randomCoordinate, accessCoordinate, this);

	return poi;
}

//  =========================================================================================
IntVector2 Map::GetTileCoordinateOfPosition(const Vector2& position)
{
	float clientWidth = Window::GetInstance()->GetClientWidth();

	Vector2 positionInCoordinateSpace = position * g_divideTileSize;
	IntVector2 tileCoordinate = IntVector2(RoundDownToInt(positionInCoordinateSpace.x), RoundDownToInt(positionInCoordinateSpace.y));
	return tileCoordinate;
}

//  =========================================================================================
Vector2 Map::GetWorldPositionOfMapCoordinate(const IntVector2& position)
{
	Vector2 worldPosition = m_tiles[position.x + (position.y * m_dimensions.x)]->GetTileWorldPosition();
	return worldPosition;
}

//  =========================================================================================
bool Map::CheckIsPositionValid(const Vector2& position)
{
	IntVector2 tileCoordinate = GetTileCoordinateOfPosition(position);
	return CheckIsCoordinateValid(tileCoordinate);
}

//  =========================================================================================
bool Map::CheckIsCoordinateValid(const IntVector2 & coordinate)
{
	bool isCoordinateValid = false;

	if(coordinate.x >= 0 && coordinate.x < m_dimensions.x && coordinate.y >= 0 && coordinate.y < m_dimensions.y)
		isCoordinateValid = true;

	return isCoordinateValid;
}

//  =========================================================================================
Vector2 Map::GetRandomNonBlockedPositionInMapBounds()
{
	bool isNonBlocked = false;
	IntVector2 randomCoord;

	while (isNonBlocked == false)
	{
		//we know outer edges are no good so might as well skip
		randomCoord = IntVector2(GetRandomIntInRange(1.f, m_dimensions.x - 1), GetRandomIntInRange(1.f, m_dimensions.y - 1));

		Tile* correspondingTileCoordinate = GetTileAtCoordinate(randomCoord);
	
		if (correspondingTileCoordinate->m_tileDefinition->m_allowsWalking)
		{
			isNonBlocked = true;
		}	
	}

	return Vector2(0.5f, 0.5f) + Vector2(randomCoord);
}

//  =========================================================================================
IntVector2 Map::GetRandomNonBlockedCoordinateInMapBounds()
{
	bool isBlocked = true;
	IntVector2 randomPoint;

	while (isBlocked)
	{
		randomPoint = IntVector2(GetRandomIntInRange(OUTER_WALL_THICKNESS, m_dimensions.x - OUTER_WALL_THICKNESS - BUILDING_DIMENSIONS.x), GetRandomIntInRange(OUTER_WALL_THICKNESS, m_dimensions.y - OUTER_WALL_THICKNESS - BUILDING_DIMENSIONS.y));
		isBlocked = IsTileBlockingAtCoordinate(randomPoint) || DoesTilePreventBuilding(randomPoint);
	}

	return randomPoint;
}

//  =========================================================================================
IntVector2 Map::GetRandomCoordinateInMapBounds()
{
	return IntVector2(GetRandomIntInRange(0, m_dimensions.x - 1), GetRandomIntInRange(0, m_dimensions.y - 1));
}
