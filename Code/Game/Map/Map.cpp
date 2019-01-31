#include "Game\Map\Map.hpp"
#include "Game\Definitions\MapDefinition.hpp"
#include "Game\Definitions\SimulationDefinition.hpp"
#include "Game\SimulationData.hpp"
#include "Game\SimulationData.hpp"
#include "Game\Map\MapGenStep.hpp"
#include "Game\GameCommon.hpp"
#include "Game\PointOfInterest.hpp"
#include "Game\Agent.hpp"
#include "Game\Planner.hpp"
#include "Game\Map\Tile.hpp"
#include "Game\Bombardment.hpp"
#include "Game\GameStates\PlayingState.hpp"
#include "Game\SimulationData.hpp"
#include "Engine\Window\Window.hpp"
#include "Engine\Time\Stopwatch.hpp"
#include "Engine\Math\MathUtils.hpp"
#include "Engine\Profiler\Profiler.hpp"
#include "Engine\Renderer\MeshBuilder.hpp"
#include "Engine\Renderer\Mesh.hpp"
#include "Engine\Time\SimpleTimer.hpp"

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

			newTile->m_tileCoords = Vector2(xCoordinate, yCoordinate);
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
	m_gameState = nullptr;

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

	//cleanup points of interest
	for (int poiIndex = 0; poiIndex < (int)m_armories.size(); ++poiIndex)
	{
		m_armories[poiIndex] = nullptr;
	}

	for (int poiIndex = 0; poiIndex < (int)m_lumberyards.size(); ++poiIndex)
	{
		m_lumberyards[poiIndex] = nullptr;
	}

	for (int poiIndex = 0; poiIndex < (int)m_medStations.size(); ++poiIndex)
	{
		m_medStations[poiIndex] = nullptr;
	}

	for (int poiIndex = 0; poiIndex < (int)m_pointsOfInterest.size(); ++poiIndex)
	{
		delete(m_pointsOfInterest[poiIndex]);
		m_pointsOfInterest[poiIndex] = nullptr;
	}

	//cleanup agents
	for (int agentIndex = 0; agentIndex < (int)m_agentsOrderedByXPosition.size(); ++agentIndex)
	{
		m_agentsOrderedByXPosition[agentIndex] = nullptr;
	}

	for (int agentIndex = 0; agentIndex < (int)m_agentsOrderedByYPosition.size(); ++agentIndex)
	{
		m_agentsOrderedByYPosition[agentIndex] = nullptr;
	}

	for (int agentIndex = 0; agentIndex < (int)m_agentsOrderedByPriority.size(); ++agentIndex)
	{
		delete(m_agentsOrderedByPriority[agentIndex]);
		m_agentsOrderedByPriority[agentIndex] = nullptr;
	}

	//tiles
	for (int tileIndex = 0; tileIndex < (int)m_tiles.size(); ++tileIndex)
	{
		delete(m_tiles[tileIndex]);
		m_tiles[tileIndex] = nullptr;
	}
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
		poiLocation = nullptr;
	}

	//create lumberyards
	for (int lumberyardIndex = 0; lumberyardIndex < m_activeSimulationDefinition->m_numArmories; ++lumberyardIndex)
	{
		//add random point of interest
		PointOfInterest* poiLocation = GeneratePointOfInterest(LUMBERYARD_POI_TYPE);
		m_lumberyards.push_back(poiLocation);
		poiLocation->m_map = this;

		m_pointsOfInterest.push_back(poiLocation);
		poiLocation = nullptr;
	}

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
	m_mapAsGrid = GetAsGrid();
}

//  =========================================================================================
void Map::Update(float deltaSeconds)
{
	PROFILER_PUSH();

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

	if (!GetIsAgentUpdateBudgeted())
	{
		for (int agentIndex = 0; agentIndex < (int)m_agentsOrderedByXPosition.size(); ++agentIndex)
		{
			m_agentsOrderedByXPosition[agentIndex]->Update(deltaSeconds);
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
	TODO("WARNING: Could possibly overflow but very unlikely");
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
		//if there is still budget left for update
		if (canUpdate)
		{
			agentUpdateTimer.Start();

			//do udpate work
			m_agentsOrderedByPriority[agentIndex]->Update(deltaSeconds);

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
		else
		{
			//update by one
			m_agentsOrderedByPriority[agentIndex]->QuickUpdate(deltaSeconds);
		}				
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

	Mesh* textMesh = CreateTextMesh();	
	if (textMesh != nullptr)
	{
		theRenderer->BindMaterial(theRenderer->CreateOrGetMaterial("text"));
		theRenderer->DrawMesh(textMesh);
	}	

	Mesh* bombardmentMesh = CreateDynamicBombardmentMesh();
	theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("Data/Images/AirStrike.png"));
	theRenderer->SetShader(theRenderer->CreateOrGetShader("agents"));
	theRenderer->DrawMesh(bombardmentMesh);

	//set back to default
	theRenderer->SetTexture(*theRenderer->m_defaultTexture);
	theRenderer->SetShader(theRenderer->m_defaultShader);

	//cleanup
	delete(agentMesh);
	agentMesh = nullptr;

	delete(bombardmentMesh);
	bombardmentMesh = nullptr;

	theRenderer = nullptr;
}

//  =============================================================================
void Map::Reload(SimulationDefinition* definition)
{
	// Cleanup Step ----------------------------------------------
	m_activeSimulationDefinition = definition;
	m_gameState = nullptr;

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
	MeshBuilder* builder = new MeshBuilder();
	
	//create mesh for static tiles and buildings
	for (int tileIndex = 0; tileIndex < (int)m_tiles.size(); ++tileIndex)
	{
		builder->CreateTexturedQuad2D(m_tiles[tileIndex]->GetBounds().GetCenter(), Vector2::ONE, m_tiles[tileIndex]->m_tileDefinition->m_baseSpriteUVCoords.mins, m_tiles[tileIndex]->m_tileDefinition->m_baseSpriteUVCoords.maxs, m_tiles[tileIndex]->m_tint );
	}

	m_mapMesh = builder->CreateMesh<VertexPCU>();


	//create debug mesh to show blocking states
	for (int tileIndex = 0; tileIndex < (int)m_tiles.size(); ++tileIndex)
	{
		int value = m_tiles[tileIndex]->m_tileDefinition->m_allowsWalking == true ? 0 : 1;
		std::string doesBlock = Stringf("%i", value);

		builder->CreateText2DInAABB2(m_tiles[tileIndex]->GetBounds().GetCenter(), Vector2::ONE, 1.f, doesBlock, m_tiles[tileIndex]->m_tint );
	}

	m_debugMapMesh = builder->CreateMesh<VertexPCU>();

	//cleanup
	delete(builder);
	builder = nullptr;
}

//  =========================================================================================
Mesh* Map::CreateDynamicAgentMesh()
{
	MeshBuilder* builder = new MeshBuilder();

	//create mesh for static tiles and buildings
	for (int agentIndex = 0; agentIndex < (int)m_agentsOrderedByYPosition.size(); ++agentIndex)
	{
		Sprite sprite = *m_agentsOrderedByXPosition[agentIndex]->m_animationSet->GetCurrentSprite(m_agentsOrderedByXPosition[agentIndex]->m_spriteDirection);

		float dimensions = 1.f;

		Vector2 spritePivot = sprite.m_definition->m_pivot;
		IntVector2 spriteDimensions = sprite.GetDimensions();

		AABB2 bounds;
		bounds.mins.x = 0.f - (spritePivot.x) * dimensions;
		bounds.maxs.x = bounds.mins.x + 1.f * dimensions;
		bounds.mins.y = 0.f - (spritePivot.y) * dimensions;
		bounds.maxs.y = bounds.mins.y + 1.f * dimensions;

		Rgba agentColor = Rgba::WHITE;
		switch (m_agentsOrderedByXPosition[agentIndex]->m_planner->m_currentPlan)
		{
		case GATHER_ARROWS_PLAN_TYPE:
			agentColor = Rgba::ORANGE;
			break;
		case GATHER_LUMBER_PLAN_TYPE:
			agentColor = Rgba::LIGHT_BLUE;
			break;
		case GATHER_BANDAGES_PLAN_TYPE:
			agentColor = Rgba::GREEN;
			break;
		case SHOOT_PLAN_TYPE:
			agentColor = Rgba::RED;
			break;
		case REPAIR_PLAN_TYPE:
			agentColor = Rgba::BLUE;
			break;
		case HEAL_PLAN_TYPE:
			agentColor = Rgba::GREEN;
			break;
		}

		builder->CreateTexturedQuad2D(m_agentsOrderedByXPosition[agentIndex]->m_position, bounds.GetDimensions(), Vector2(sprite.GetNormalizedUV().mins.x, sprite.GetNormalizedUV().maxs.y), Vector2(sprite.GetNormalizedUV().maxs.x, sprite.GetNormalizedUV().mins.y), agentColor);
	}

	 Mesh* agentMesh = builder->CreateMesh<VertexPCU>();

	//cleanup
	delete(builder);
	builder = nullptr;

	return agentMesh;
}


//  =============================================================================
Mesh* Map::CreateTextMesh()
{
	MeshBuilder* builder = new MeshBuilder();
	Mesh* textMesh = nullptr;

	//agent ids
	if (g_isIdShown)
	{
		for (int agentIndex = 0; agentIndex < (int)m_agentsOrderedByYPosition.size(); ++agentIndex)
		{
			builder->CreateText2DInAABB2(Vector2(m_agentsOrderedByXPosition[agentIndex]->m_position.x, m_agentsOrderedByXPosition[agentIndex]->m_position.y + 0.7), Vector2(0.25f, 0.25f), 1.f, Stringf("%i", m_agentsOrderedByXPosition[agentIndex]->m_id), Rgba::WHITE);
		}
	}

	//draw other things

	if (builder->m_vertices.size() > 0)
	{
		textMesh = builder->CreateMesh<VertexPCU>();
	}	

	delete(builder);
	builder = nullptr;
	return textMesh;
}

//  =========================================================================================
Mesh* Map::CreateDynamicBombardmentMesh()
{
	MeshBuilder* builder = new MeshBuilder();

	for (int bombardmentIndex = 0; bombardmentIndex < (int)m_activeBombardments.size(); ++bombardmentIndex)
	{	
		AABB2 bombardmentBox = AABB2(m_activeBombardments[bombardmentIndex]->m_disc.center, m_activeBombardments[bombardmentIndex]->m_disc.radius, m_activeBombardments[bombardmentIndex]->m_disc.radius);
		builder->CreateTexturedQuad2D(m_activeBombardments[bombardmentIndex]->m_disc.center, bombardmentBox.GetDimensions(), Vector2::ZERO, Vector2::ONE, Rgba(1.f, 1.f, 1.f, .5f));
	}

	Mesh* bombardmentMesh = builder->CreateMesh<VertexPCU>();

	//cleanup
	delete(builder);
	builder = nullptr;

	return bombardmentMesh;
}

//  =========================================================================================
void Map::DeleteDeadEntities()
{
	for (int bombardmentIndex = 0; bombardmentIndex < (int)m_activeBombardments.size(); ++bombardmentIndex)
	{
		if (m_activeBombardments[bombardmentIndex]->IsExplosionComplete())
		{
			//check collision to all characters and buildings
			DetectBombardmentToAgentCollision(m_activeBombardments[bombardmentIndex]);
			DetectBombardmentToPOICollision(m_activeBombardments[bombardmentIndex]);

			m_activeBombardments.erase(m_activeBombardments.begin() + bombardmentIndex);
			--bombardmentIndex;
		}
	}
}

// Bubble sort. Once sorted, will rarely require more than one pass =========================================================================================
void Map::SortAgentsByX()
{
	PROFILER_PUSH();

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
	PROFILER_PUSH();

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
Grid<int>* Map::GetAsGrid()
{
	Grid<int>* grid = new Grid<int>();

	grid->InitializeGrid(0, m_dimensions.x, m_dimensions.y);

	for (int tileIndex = 0; tileIndex < (int)m_tiles.size(); tileIndex++)
	{
		int value = m_tiles[tileIndex]->m_tileDefinition->m_allowsWalking ? 0 : 1;

		if(value != 0)
			grid->SetValueAtIndex(value, tileIndex);
	}

	return grid;
}

//  =========================================================================================
bool Map::IsTileBlockingAtCoordinate(const IntVector2& coordinate)
{
	bool isBlocking = !GetTileAtCoordinate(coordinate)->m_tileDefinition->m_allowsWalking;
	return isBlocking;	
}

//  =========================================================================================
Tile* Map::GetTileAtCoordinate(const IntVector2& coordinate)
{
	return m_tiles[coordinate.x + (coordinate.y * m_dimensions.x)];
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
PointOfInterest* Map::GeneratePointOfInterest(int poiType)
{
	ePointOfInterestType type = (ePointOfInterestType)poiType;

	//determine if random location is unblocked
	bool isLocationValid = false;
	IntVector2 randomCoordinate = IntVector2::ONE;
	IntVector2 accessCoordinate = IntVector2::ONE;

	while (!isLocationValid)
	{
		randomCoordinate = GetRandomNonBlockedCoordinateInMapBounds();
		
		//check tile to north, to the right, and to the northeast to see if they are even good
		if(CheckIsCoordianteValid(IntVector2(randomCoordinate.x + 1, randomCoordinate.y)) 
			&& CheckIsCoordianteValid(IntVector2(randomCoordinate.x, randomCoordinate.y + 1))
			&& CheckIsCoordianteValid(IntVector2(randomCoordinate.x + 1, randomCoordinate.y + 1)))
		{
			//check tile to north, to the right, and to the northeast to see if they are blocked
			if (!IsTileBlockingAtCoordinate(IntVector2(randomCoordinate.x + 1, randomCoordinate.y))
				&& !IsTileBlockingAtCoordinate(IntVector2(randomCoordinate.x, randomCoordinate.y + 1))
				&& !IsTileBlockingAtCoordinate(IntVector2(randomCoordinate.x + 1, randomCoordinate.y + 1)))
			{
				/*now we should get a random location for our access location for the poi
				there are potentially eight acceptable locations
				   [5][4]
				[6][X][X][3]
				[7][X][X][2]
				   [0][1]
				*/

				bool isAccessLocationValid =  false;
				int accessIterationAttemptCount = 0;

				while (!isAccessLocationValid && accessIterationAttemptCount < 7)
				{
					int randomAccessLocation = GetRandomIntInRange(0, 7);
					switch (randomAccessLocation)
					{
					case 0:
						accessCoordinate = IntVector2(randomCoordinate.x, randomCoordinate.y - 1);
						break;
					case 1:
						accessCoordinate = IntVector2(randomCoordinate.x + 2, randomCoordinate.y - 1);
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

					if (CheckIsCoordianteValid(accessCoordinate) && !IsTileBlockingAtCoordinate(accessCoordinate))
					{
						isAccessLocationValid = true;
						isLocationValid = true;
					}

					if (!isAccessLocationValid)				
						++accessIterationAttemptCount;					
				}

				if(isAccessLocationValid)
					isLocationValid = true;
			}
		}		
	}

	//Location is valid, therefore we can replace tiles with building tiles

	Rgba buildingColor = Rgba::WHITE;
	switch (poiType)
	{
	case ARMORY_POI_TYPE:
		buildingColor = Rgba::LIGHT_RED_TRANSPARENT;
		break;
	case LUMBERYARD_POI_TYPE:
		buildingColor = Rgba::LIGHT_BLUE_TRANSPARENT;
		break;
	case MED_STATION_POI_TYPE:
		buildingColor = Rgba::LIGHT_GREEN_TRANSPARENT;
		break;
	}

	//starting tile
	Tile* tile = GetTileAtCoordinate(randomCoordinate);
	tile->m_tileDefinition = TileDefinition::s_tileDefinitions.find("Building")->second;
	tile->m_tint = buildingColor;

	//tile to east
	tile = GetTileAtCoordinate(IntVector2(randomCoordinate.x + 1, randomCoordinate.y));
	tile->m_tileDefinition = TileDefinition::s_tileDefinitions.find("Building")->second;
	tile->m_tint = buildingColor;

	//tile to northeast
	tile = GetTileAtCoordinate(IntVector2(randomCoordinate.x + 1, randomCoordinate.y + 1));
	tile->m_tileDefinition = TileDefinition::s_tileDefinitions.find("Building")->second;
	tile->m_tint = buildingColor;

	//tile to north
	tile = GetTileAtCoordinate(IntVector2(randomCoordinate.x, randomCoordinate.y + 1));
	tile->m_tileDefinition = TileDefinition::s_tileDefinitions.find("Building")->second;
	tile->m_tint = buildingColor;

	//building access tile
	tile = GetTileAtCoordinate(accessCoordinate);
	tile->m_tileDefinition = TileDefinition::s_tileDefinitions.find("BuildingAccess")->second;

	PointOfInterest* poi = new PointOfInterest(type, randomCoordinate, accessCoordinate, this);

	//cleanup
	tile = nullptr;

	return poi;
}

//  =========================================================================================
IntVector2 Map::GetTileCoordinateOfPosition(const Vector2& position)
{
	float clientWidth = Window::GetInstance()->GetClientWidth();

	Vector2 positionInCoordinateSpace = position * g_divideTileSize;
	IntVector2 tileCoordinate = IntVector2(RoundToNearestInt(positionInCoordinateSpace.x), RoundToNearestInt(positionInCoordinateSpace.y));
	return tileCoordinate;
}

//  =========================================================================================
Vector2 Map::GetWorldPositionOfMapCoordinate(const IntVector2& position)
{
	Vector2 worldPosition = m_tiles[position.x + (position.y * m_dimensions.x)]->GetTileCoordinates();
	return worldPosition;
}

//  =========================================================================================
bool Map::CheckIsPositionValid(const Vector2& position)
{
	IntVector2 tileCoordinate = GetTileCoordinateOfPosition(position);
	return CheckIsCoordianteValid(tileCoordinate);
}

//  =========================================================================================
bool Map::CheckIsCoordianteValid(const IntVector2 & coordinate)
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
	Vector2 randomPoint = Vector2::NEGATIVE_ONE;

	while (isNonBlocked == false)
	{
		AABB2 validBounds = m_mapWorldBounds;
		validBounds.maxs = Vector2(validBounds.maxs.x - g_tileSize, validBounds.maxs.y - g_tileSize);

		randomPoint = validBounds.GetRandomPointInBounds();
		IntVector2 correspondingTileCoordinate = GetTileCoordinateOfPosition(randomPoint);
	
		if (m_tiles[correspondingTileCoordinate.x + (correspondingTileCoordinate.y * m_dimensions.x)]->m_tileDefinition->m_allowsWalking)
		{
			isNonBlocked = true;
		}	
	}

	return randomPoint;
}

//  =========================================================================================
IntVector2 Map::GetRandomNonBlockedCoordinateInMapBounds()
{
	bool isBlocked = true;
	IntVector2 randomPoint;

	while (isBlocked)
	{
		randomPoint = IntVector2(GetRandomIntInRange(0, m_dimensions.x - 1), GetRandomIntInRange(0, m_dimensions.y - 1));
		isBlocked = IsTileBlockingAtCoordinate(randomPoint);
	}

	return randomPoint;
}

//  =========================================================================================
IntVector2 Map::GetRandomCoordinateInMapBounds()
{
	return IntVector2(GetRandomIntInRange(0, m_dimensions.x - 1), GetRandomIntInRange(0, m_dimensions.y - 1));
}
