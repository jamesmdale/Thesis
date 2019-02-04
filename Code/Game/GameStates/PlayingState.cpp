#include <map>
#include <string>
#include "Engine\Window\Window.hpp"
#include "Engine\Debug\DebugRender.hpp"
#include "Engine\Core\LightObject.hpp"
#include "Engine\Renderer\MeshBuilder.hpp"
#include "Engine\Debug\DebugRender.hpp"
#include "Engine\Math\IntVector2.hpp"
#include "Engine\Core\StringUtils.hpp"
#include "Engine\Core\DevConsole.hpp"
#include "Engine\File\FileHelpers.hpp"
#include "Engine\Renderer\Mesh.hpp"
#include "Engine\Math\AABB2.hpp"
#include "Engine\Time\SimpleTimer.hpp"
#include "Game\GameStates\PlayingState.hpp"
#include "Game\PointOfInterest.hpp"
#include "Game\Agent.hpp"
#include "Game\Definitions\SimulationDefinition.hpp"
#include "Game\SimulationData.hpp"
#include "Game\DebugInputBox.hpp"

float ORTHO_MAX = 0.f;
float ORTHO_MIN = 0.f;
float g_orthoZoom = 0.f;

float ZOOM_RATE = 5.f;

std::string simDataOutputDirectory = "";
bool isResetingSimulation = false;

//  =============================================================================
PlayingState::~PlayingState()
{
	m_disectedAgent = nullptr;

	delete(m_simulationTimer);
	m_simulationTimer = nullptr;

	//delete scene last
	delete(m_renderScene2D);
	m_renderScene2D = nullptr;		
}

//  =============================================================================
void PlayingState::Initialize()
{
	Window* theWindow = Window::GetInstance();
	Renderer* theRenderer = Renderer::GetInstance();

	//setup cameras
	m_uiCamera = new Camera();
	m_uiCamera->SetColorTarget(theRenderer->GetDefaultRenderTarget());
	m_uiCamera->SetOrtho(0.f, theWindow->GetClientWidth(), 0.f, theWindow->GetClientHeight(), -1.f, 1.f);
	m_uiCamera->SetView(Matrix44::IDENTITY);

	//set game camera
	m_camera = new Camera();
	m_camera->SetColorTarget(theRenderer->GetDefaultRenderTarget());
	m_camera->SetOrtho(0.f, 32.f, 0.f, 18.f, -1000.f, 1000.f);
	m_camera->SetView(Matrix44::IDENTITY);	

	g_orthoZoom = 1.f;
	ORTHO_MAX = 20.f;
	ORTHO_MIN = 0.f;

	//register commands
	//RegisterCommand("toggle_optimization", CommandRegistration(ToggleOptimized, ": Toggle blanket optimizations on and off", ""));
	RegisterCommand("pause_game", CommandRegistration(TogglePaused, ": Toggle pause on and off", ""));
	RegisterCommand("agent", CommandRegistration(DisectAgent, ": View information for given agent. (int agentId)", ""));
	RegisterCommand("toggle_ids", CommandRegistration(ToggleAgentIds, ": View agent ids", ""));
	RegisterCommand("toggle_blocks", CommandRegistration(ToggleBlockedData, ": View tile physics data", ""));

	//simulation setup
	GenerateOutputDirectory();
	g_currentSimulationDefinition = SimulationDefinition::s_simulationDefinitions[g_currentSimDefinitionIndex];

	CreateMapForSimulation(g_currentSimulationDefinition);
	InitializeSimulation(g_currentSimulationDefinition);

	//set per frame budget for 60fps
	g_perFrameHPCBudget = SecondsToPerformanceCounter(1.0 / 70.0);

	//generate debug input even if we don't really use it
	DebugInputBox::CreateInstance();

	//move camera to starting location
	m_camera->SetPosition(Vector3::ZERO);

	//cleanup
	theRenderer = nullptr;
	theWindow = nullptr;
}

//  =============================================================================
void PlayingState::Update(float deltaSeconds)
{ 
	if (!GetGameClock()->IsPaused())
	{
		m_map->Update(deltaSeconds);

		if (m_simulationTimer->ResetAndDecrementIfElapsed())
		{
			isResetingSimulation = true;
		}
	}	
}

//  =============================================================================
void PlayingState::PreRender()
{
	if (m_isCameraLockedToAgent)
	{
		//have the camera follow the agent
		m_camera->SetPosition(Vector3(m_disectedAgent->m_position) * 0.5f);
	}
}

//  =============================================================================
void PlayingState::Render()
{
	//this timer determines how much time we have for all of our agent update.
	SimpleTimer timer;
	timer.Start();

	Renderer* theRenderer = Renderer::GetInstance();

	Game::GetInstance()->m_forwardRenderingPath2D->Render(m_renderScene2D);

	RenderGame();
	RenderDebugUI();

	//this timer determines how much time we have for all of our agent update.
	timer.Stop();
	g_previousFrameRenderTime = timer.GetRunningTime();

	theRenderer = nullptr;
}

//  =============================================================================
void PlayingState::PostRender()
{
	if (g_isQuitting)
		return;

	m_map->DeleteDeadEntities();

	if (isResetingSimulation)
	{
		ExportSimulationData();
		ResetCurrentSimulationData();

		g_currentSimDefinitionIndex++;
		isResetingSimulation = false;
		m_disectedAgent = nullptr;

		if (g_currentSimDefinitionIndex < SimulationDefinition::s_simulationDefinitions.size())
		{
			SimulationDefinition* definition = SimulationDefinition::s_simulationDefinitions[g_currentSimDefinitionIndex];	

			//if we are on the same map, leave the spawn points the same.
			if(definition->m_mapDefinition == g_currentSimulationDefinition->m_mapDefinition)
			{
				g_currentSimulationDefinition = definition;
				ResetMapForSimulation(definition);
				InitializeSimulation(definition);
			}
			else
			{
				g_currentSimulationDefinition = definition;
				DeleteMap();
				CreateMapForSimulation(definition);
				InitializeSimulation(definition);
			}		
		}
		else
		{
			g_isQuitting = true;
		}	

		//reset timer regardless
		m_simulationTimer->Reset();
	}	
}

//  =============================================================================
float PlayingState::UpdateFromInput(float deltaSeconds)
{
	InputSystem* theInput = InputSystem::GetInstance();

	//movement should run on the master deltaseconds so we can move the camera even if the game is paused
	if (theInput->IsKeyPressed(theInput->KEYBOARD_DOWN_ARROW))
	{
		m_camera->Translate(Vector3(Vector2::DOWN * 5.f * GetMasterDeltaSeconds()));
		m_isCameraLockedToAgent = false;
	}
	if (theInput->IsKeyPressed(theInput->KEYBOARD_UP_ARROW))
	{
		m_camera->Translate(Vector3(Vector2::UP * 5.f * GetMasterDeltaSeconds()));
		m_isCameraLockedToAgent = false;
	}
	if (theInput->IsKeyPressed(theInput->KEYBOARD_RIGHT_ARROW))
	{
		m_camera->Translate(Vector3(Vector2::RIGHT * 5.f * GetMasterDeltaSeconds()));
		m_isCameraLockedToAgent = false;
	}
	if (theInput->IsKeyPressed(theInput->KEYBOARD_LEFT_ARROW))
	{
		m_camera->Translate(Vector3(Vector2::LEFT * 5.f * GetMasterDeltaSeconds()));
		m_isCameraLockedToAgent = false;
	}

	// cycle through agents ----------------------------------------------
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_UP_ARROW) && theInput->IsKeyPressed(theInput->KEYBOARD_CONTROL))
	{
		if (m_disectedAgent == nullptr)
		{
			m_disectedAgent = m_map->GetAgentById(0);
		}
		else
		{
			int currentAgentId = m_disectedAgent->m_id;
			m_disectedAgent = m_map->GetAgentById(currentAgentId + 1);
			if (m_disectedAgent == nullptr)
			{
				m_map->GetAgentById(0);
			}
		}
	}
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_DOWN_ARROW) && theInput->IsKeyPressed(theInput->KEYBOARD_CONTROL))
	{
		if (m_disectedAgent == nullptr)
		{
			m_disectedAgent = m_map->GetAgentById(0);
		}
		else
		{
			int currentAgentId = m_disectedAgent->m_id;
			m_disectedAgent = m_map->GetAgentById(currentAgentId - 1);
			if (m_disectedAgent == nullptr)
			{
				m_map->GetAgentById(0);
			}
		}
	}

	// reset camera ----------------------------------------------
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_R) && theInput->IsKeyPressed(theInput->KEYBOARD_CONTROL))
	{
		g_orthoZoom = Window::GetInstance()->GetClientHeight();
		m_camera->SetProjectionOrtho(g_orthoZoom, CLIENT_ASPECT, -1000.f, 1000.f);
	}

	// show tile data ----------------------------------------------
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_T) && theInput->IsKeyPressed(theInput->KEYBOARD_CONTROL))
	{
		g_isBlockedTileDataShown = !g_isBlockedTileDataShown;	
	}

	// show agentids ----------------------------------------------
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_I) && theInput->IsKeyPressed(theInput->KEYBOARD_CONTROL))
	{
		g_isIdShown = !g_isIdShown;
	}

	// toggle debug ----------------------------------------------
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_F) && theInput->IsKeyPressed(theInput->KEYBOARD_CONTROL))
	{
		g_isDebugDataShown = !g_isDebugDataShown;
		
		//also, "Open" or "close" the debug input box accordingly
		if (g_isDebugDataShown)
		{
			DebugInputBox::GetInstance()->Open();
		}
		else
		{
			DebugInputBox::GetInstance()->Close();
			m_disectedAgent = nullptr;
			m_isCameraLockedToAgent = false;
		}		
	}

	// pause ----------------------------------------------
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_P) && theInput->IsKeyPressed(theInput->KEYBOARD_CONTROL))
	{
		//toggle pause
		GetGameClock()->SetPaused(!GetGameClock()->IsPaused());
	}
	
	return deltaSeconds; //new deltaSeconds
}

//  =========================================================================================
void PlayingState::RenderGame()
{
	Renderer* theRenderer = Renderer::GetInstance();
	theRenderer->SetCamera(m_camera);
	m_map->Render();

	theRenderer = nullptr;
}

//  =========================================================================================
void PlayingState::RenderDebugUI()
{
	if (!g_isDebugDataShown)
	{
		return;
	}

	Renderer* theRenderer = Renderer::GetInstance()->GetInstance();
	theRenderer->SetCamera(m_uiCamera);

	//handle text
	Mesh* textMesh = CreateUIDebugTextMesh();
	if (textMesh != nullptr)
	{
		theRenderer->BindMaterial(theRenderer->CreateOrGetMaterial("text"));
		theRenderer->DrawMesh(textMesh);
		delete(textMesh);
	}	

	//because the path uses the world space and not screen space (ui camera), we need to set back to the main camera
	theRenderer->SetCamera(m_camera);

	textMesh = CreateWorldDebugTextMesh();
	if (textMesh != nullptr)
	{
		theRenderer->BindMaterial(theRenderer->CreateOrGetMaterial("text"));
		theRenderer->DrawMesh(textMesh);
		delete(textMesh);
	}	

	//handle path
	Mesh* pathMesh = nullptr;
	if (m_disectedAgent != nullptr)
	{
		if (m_disectedAgent->m_currentPath.size() > 0)
		{
			Mesh* pathMesh = CreateDisectedAgentPathMesh();

			if (pathMesh != nullptr)
			{
				theRenderer->BindMaterial(theRenderer->CreateOrGetMaterial("default"));
				theRenderer->DrawMesh(pathMesh);
				delete(pathMesh);
				pathMesh = nullptr;
			}				
		}	

		theRenderer->DrawDottedDisc2WithColor(m_disectedAgent->m_physicsDisc, Rgba::PINK, 10);
		//theRenderer->DrawAABB(m_disectedAgent->m, Rgba::PINK);
	}
}

//  =============================================================================
void PlayingState::InitializeSimulation(SimulationDefinition* definition)
{
	//current sim definition
	g_currentSimulationDefinition = definition;
	InitializeSimulationData();

	//re-adjust camera center
	Vector2 mapCenter = -1.f * m_map->m_mapWorldBounds.GetCenter();
	m_camera->SetPosition(Vector3(mapCenter.x, mapCenter.y, 0.f));	

	m_simulationTimer = new Stopwatch(GetGameClock());
	m_simulationTimer->SetTimer(g_generalSimulationData->m_simulationDefinitionReference->m_totalProcessingTimeInSeconds);
}

//  =============================================================================
void PlayingState::CreateMapForSimulation(SimulationDefinition* definition)
{
	//map creation
	m_map = new Map(definition, "TestMap", m_renderScene2D);	
	m_map->Initialize();
	m_map->m_playingState = this;
}

//  =============================================================================
void PlayingState::ResetMapForSimulation(SimulationDefinition* definition)
{
	m_map->Reload(definition);
	m_map->m_playingState = this;
}

//  =============================================================================
void PlayingState::InitializeSimulationData()
{	
	g_generalSimulationData = new SimulationData();
	g_generalSimulationData->Initialize(g_currentSimulationDefinition);

#ifdef ActionStackAnalysis
	//action stack data
	g_processActionStackData = new SimulationData();
	g_processActionStackData->Initialize(g_currentSimulationDefinition);
#endif // ActionStackAnalysis

#ifdef UpdatePlanAnalysis
	//update plan data
	g_updatePlanData = new SimulationData();
	g_updatePlanData->Initialize(g_currentSimulationDefinition);
#endif // UpdatePlanAnalysis

#ifdef AgentUpdateAnalysis
	//agent update data
	g_agentUpdateData = new SimulationData();
	g_agentUpdateData->Initialize(g_currentSimulationDefinition);
#endif // AgentUpdateAnalysis

#ifdef PathingDataAnalysis
	//pathing data
	g_pathingData = new SimulationData();
	g_pathingData->Initialize(g_currentSimulationDefinition);
#endif // PathingDataAnalysis

#ifdef CopyPathAnalysis
	//copy path data
	g_copyPathData = new SimulationData();
	g_copyPathData->Initialize(g_currentSimulationDefinition);
#endif // CopyPathAnalysis

#ifdef QueueActionPathingDataAnalysis
	//queue path data
	g_queueActionPathingData = new SimulationData();
	g_queueActionPathingData->Initialize(g_currentSimulationDefinition);
#endif // QueueActionPathingDataAnalysis

#ifdef DistanceMemoizationDataAnalysis
	//queue path data
	g_distanceMemoizationData = new SimulationData();
	g_distanceMemoizationData->Initialize(g_currentSimulationDefinition);
#endif // DistanceMemoizationDataAnalysis
}

//  =============================================================================
void PlayingState::ResetCurrentSimulationData()
{
	delete(g_generalSimulationData);
	g_generalSimulationData = nullptr;

#ifdef ActionStackAnalysis
	delete(g_processActionStackData);
	g_processActionStackData = nullptr;
#endif

#ifdef UpdatePlanAnalysis
	delete(g_updatePlanData);
	g_updatePlanData = nullptr;
#endif

#ifdef AgentUpdateAnalysis
	delete(g_agentUpdateData);
	g_agentUpdateData = nullptr;
#endif

#ifdef PathingDataAnalysis
	delete(g_pathingData);
	g_pathingData = nullptr;
#endif

#ifdef CopyPathAnalysis
	delete(g_copyPathData);
	g_copyPathData = nullptr;
#endif

#ifdef QueueActionPathingDataAnalysis
	delete(g_queueActionPathingData);
	g_queueActionPathingData = nullptr;
#endif

#ifdef DistanceMemoizationDataAnalysis
	delete(g_distanceMemoizationData);
	g_distanceMemoizationData = nullptr;
#endif

	g_numUpdatePlanCalls = 0;
	g_numActionStackProcessCalls = 0;
	g_numAgentUpdateCalls = 0;
	g_numGetPathCalls = 0;
	g_numCopyPathCalls = 0;
	g_numQueueActionPathCalls = 0;
}

//  =============================================================================
void PlayingState::DeleteMap()
{
	delete(m_map);
	m_map = nullptr;
}

//  =============================================================================
void PlayingState::ExportSimulationData()
{
	//std::string newFolderName = Stringf("SIMULATION_TEST_%s", GetCurrentDateTime().c_str());
	//std::string newPath =  Stringf("%s%s", "Data\\ExportedSimulationData\\", newFolderName);
	//CreateFolder(newPath.c_str());	

	std::string newFolder = Stringf("%s%s%s", simDataOutputDirectory.c_str(), "Simulation_Definition_", g_currentSimulationDefinition->m_name.c_str());
	CreateFolder(newFolder.c_str());

	std::string finalFilePath = Stringf("%s%s", newFolder.c_str(), "\\");


	//general data
	FinalizeGeneralSimulationData();
	std::string fileName = Stringf("GeneralInfo_%s.csv", g_currentSimulationDefinition->m_name.c_str());
	bool success = g_generalSimulationData->ExportCSV(finalFilePath, fileName.c_str());
	ASSERT_OR_DIE(success, "Action data broken");

#ifdef ActionStackAnalysis
	//export action stack data
	fileName = Stringf("ActionStackAverageTimesPer_%s.csv", g_currentSimulationDefinition->m_name.c_str());
	success = g_processActionStackData->ExportCSV(finalFilePath, fileName.c_str());
	ASSERT_OR_DIE(success, "Action data broken");
#endif

#ifdef UpdatePlanAnalysis
	//export update plan data
	fileName = Stringf("AgentUpdatePlanAverageTimesPer_%s.csv", g_currentSimulationDefinition->m_name.c_str());
	success = g_updatePlanData->ExportCSV(finalFilePath, fileName.c_str());
	ASSERT_OR_DIE(success, "Update plan data broken");
#endif

#ifdef AgentUpdateAnalysis
	//export agent update data
	fileName = Stringf("AgentUpdateAverageTimes_%s.csv", g_currentSimulationDefinition->m_name.c_str());
	success = g_agentUpdateData->ExportCSV(finalFilePath, fileName.c_str());
	ASSERT_OR_DIE(success, "Update data broken");
#endif

#ifdef PathingDataAnalysis
	//export stack data
	fileName = Stringf("PathingDataAveragesTimesPer_%s.csv", g_currentSimulationDefinition->m_name.c_str());
	success = g_pathingData->ExportCSV(finalFilePath, fileName.c_str());
	ASSERT_OR_DIE(success, "Pathing data broken");
#endif

#ifdef CopyPathAnalysis
	//action stack data
	fileName = Stringf("CopyPathAverageTimersPer_%s.csv", g_currentSimulationDefinition->m_name.c_str());
	success = g_copyPathData ->ExportCSV(finalFilePath, fileName.c_str());
	ASSERT_OR_DIE(success, "Copy path data broken");
#endif

#ifdef QueueActionPathingDataAnalysis
	fileName = Stringf("QueueActionPathingTimes_%s.csv", g_currentSimulationDefinition->m_name.c_str());
	success = g_queueActionPathingData ->ExportCSV(finalFilePath, fileName.c_str());
	ASSERT_OR_DIE(success, "Copy path data broken");
#endif

#ifdef DistanceMemoizationDataAnalysis
	fileName = Stringf("DistanceMemoizationUtilityTimes_%s.csv", g_currentSimulationDefinition->m_name.c_str());
	success = g_distanceMemoizationData ->ExportCSV(finalFilePath, fileName.c_str());
	ASSERT_OR_DIE(success, "Copy path data broken");
#endif
}

//  =============================================================================
void PlayingState::GenerateOutputDirectory()
{
	std::string newFolderName = Stringf("SIMULATION_TEST_%s", GetCurrentDateTime().c_str());
	std::string newPath =  Stringf("%s%s", "Data\\ExportedSimulationData\\", newFolderName.c_str());

	bool success = CreateFolder(newPath.c_str());
	ASSERT_OR_DIE(success, Stringf("UNABLE TO CREATE FOLDER (%s)", newPath.c_str()).c_str());

	simDataOutputDirectory = Stringf("%s%s", newPath.c_str(), "\\");
}

//  =============================================================================
void PlayingState::FinalizeGeneralSimulationData()
{
	std::string optimizationString = "";
	g_generalSimulationData->CreateComprehensiveDataSet();

	GetIsOptimized() ? optimizationString = "true" : optimizationString = "false";
	g_generalSimulationData->AddCell("Is Optimized?", false);
	g_generalSimulationData->AddCell(Stringf("%s", optimizationString.c_str()));
	g_generalSimulationData->AddNewLine();

#ifdef UpdatePlanAnalysis
	g_generalSimulationData->AddCell("Num update plan calls:", false);
	g_generalSimulationData->AddCell(Stringf(" %i", g_numUpdatePlanCalls));
	g_generalSimulationData->AddNewLine();
#endif

#ifdef ActionStackAnalysis
	g_generalSimulationData->AddCell("Num Process Action Stack calls:");
	g_generalSimulationData->AddCell(Stringf(" %i", g_numActionStackProcessCalls));
	g_generalSimulationData->AddNewLine();
#endif

#ifdef AgentUpdateAnalysis
	g_generalSimulationData->AddCell("Num Agent Update calls:");
	g_generalSimulationData->AddCell(Stringf(" %i", g_numAgentUpdateCalls));
	g_generalSimulationData->AddNewLine();
#endif

#ifdef PathingDataAnalysis
	g_generalSimulationData->AddCell("Num Get Path calls:");
	g_generalSimulationData->AddCell(Stringf(" %i", g_numGetPathCalls));
	g_generalSimulationData->AddNewLine();
#endif

#ifdef CopyPathAnalysis
	g_generalSimulationData->AddCell("Num Copy Path calls:");
	g_generalSimulationData->AddCell(Stringf(" %i", g_numCopyPathCalls));
	g_generalSimulationData->AddNewLine();
#endif

#ifdef QueueActionPathingDataAnalysis
	g_generalSimulationData->AddCell("Num Queue Action Path calls:");
	g_generalSimulationData->AddCell(Stringf(" %i", g_numQueueActionPathCalls));
	g_generalSimulationData->AddNewLine();
#endif
	
}

//  =========================================================================================
Mesh* PlayingState::CreateUIDebugTextMesh()
{
	MeshBuilder builder = MeshBuilder();
	Mesh* textMesh = nullptr;
	Window* theWindow = Window::GetInstance();

	// fps counter ----------------------------------------------		
	AABB2 fpsBox = AABB2(theWindow->GetClientWindow(), Vector2(0.8f, 0.9f), Vector2(0.95f, 0.975f));
	builder.CreateText2DInAABB2( fpsBox.GetCenter(), fpsBox.GetDimensions(), 1.f, Stringf("FPS: %f", GetUnclampedFPS()), Rgba::WHITE);

	//cam position information ----------------------------------------------
	AABB2 camPosBox = AABB2(theWindow->GetClientWindow(), Vector2(0.8f, 0.85f), Vector2(0.95f, 0.9f));
	Vector3 camPosition = m_camera->m_transform->GetWorldPosition();
	builder.CreateText2DInAABB2( camPosBox.GetCenter(), camPosBox.GetDimensions(), 1.f, Stringf("Cam Pos: %f,%f", camPosition.x, camPosition.y));

	//agent update per frameinfo ----------------------------------------------
	if (GetIsAgentUpdateBudgeted())
	{
		AABB2 agentsUpdatedBox = AABB2(theWindow->GetClientWindow(), Vector2(0.8f, 0.8f), Vector2(0.95f, 0.85f));
		builder.CreateText2DInAABB2( agentsUpdatedBox.GetCenter(), agentsUpdatedBox.GetDimensions(), 1.f, Stringf("Agents Updated: %i", g_agentsUpdatedThisFrame), Rgba::WHITE);
	}
	
	//threat ----------------------------------------------
	AABB2 agentsUpdatedBox = AABB2(theWindow->GetClientWindow(), Vector2(0.8f, 0.8f), Vector2(0.95f, 0.85f));
	builder.CreateText2DInAABB2( agentsUpdatedBox.GetCenter(), agentsUpdatedBox.GetDimensions(), 1.f, Stringf("Threat: %i/%i", (int)m_map->m_threat, (int)g_maxThreat), Rgba::WHITE);


	//debug input ----------------------------------------------
	DebugInputBox* theDebugInputBox = DebugInputBox::GetInstance();
	std::string inputText = Stringf("> %s", theDebugInputBox->GetInput().c_str());
	builder.CreateText2DFromPoint( Vector2(10.f , 0.0f), theWindow->GetClientHeight() * 0.015, 1.f, inputText.c_str(), Rgba::WHITE );

	//disected agent ----------------------------------------------
	if (m_disectedAgent != nullptr)
	{
		std::vector<std::string> agentInfo;
		m_disectedAgent->ConstructInformationAsText(agentInfo);

		float printHeight = 0.5f;

		for (int agentInfoIndex = 0; agentInfoIndex < agentInfo.size(); ++agentInfoIndex)
		{
			builder.CreateText2DFromPoint( Vector2(theWindow->GetClientWidth() * 0.85f , theWindow->GetClientHeight() * printHeight), theWindow->GetClientHeight() * 0.010, 1.f, agentInfo[agentInfoIndex].c_str(), Rgba::WHITE );
			printHeight -= 0.02;
		}
	}

	// create mesh ----------------------------------------------
	if (builder.m_vertices.size() > 0)
	{
		textMesh = builder.CreateMesh<VertexPCU>();
	}

	return textMesh;
}

//  =========================================================================================
Mesh* PlayingState::CreateWorldDebugTextMesh()
{
	MeshBuilder builder = MeshBuilder();
	Mesh* textMesh = nullptr;
	Window* theWindow = Window::GetInstance();

	//building health ----------------------------------------------
	for (int armoryIndex = 0; armoryIndex < m_map->m_armories.size(); ++armoryIndex)
	{
		AABB2 armoryBounds = m_map->m_armories[armoryIndex]->GetWorldBounds();
		builder.CreateText2DInAABB2( armoryBounds.GetCenter(), armoryBounds.GetDimensions(), 1.f, Stringf("%i", (int)m_map->m_armories[armoryIndex]->m_health), Rgba::WHITE);
	}
	for (int lumberyardIndex = 0; lumberyardIndex < m_map->m_lumberyards.size(); ++lumberyardIndex)
	{
		AABB2 lumberyardBounds = m_map->m_lumberyards[lumberyardIndex]->GetWorldBounds();
		builder.CreateText2DInAABB2( lumberyardBounds.GetCenter(), lumberyardBounds.GetDimensions(), 1.f, Stringf("%i", (int)m_map->m_lumberyards[lumberyardIndex]->m_health), Rgba::WHITE);
	}

	// create mesh ----------------------------------------------
	if (builder.m_vertices.size() > 0)
	{
		textMesh = builder.CreateMesh<VertexPCU>();
	}

	return textMesh;
}

//  =========================================================================================
Mesh* PlayingState::CreateDisectedAgentPathMesh()
{
	MeshBuilder builder = MeshBuilder();
	Mesh* pathMesh = nullptr;

	//we know the agent is set so we don't have to check for nullptr case
	for (int pathIndex = 0; pathIndex < m_disectedAgent->m_currentPath.size() - 1; ++pathIndex)
	{
		IntVector2 tileCoords = IntVector2(m_disectedAgent->m_currentPath[pathIndex]);
		IntVector2 nextTileCoords = IntVector2(m_disectedAgent->m_currentPath[pathIndex + 1]);

		Vector2 tileCenter = m_map->GetTileAtCoordinate(tileCoords)->GetBounds().GetCenter();
		Vector2 nextTileCenter = m_map->GetTileAtCoordinate(nextTileCoords)->GetBounds().GetCenter();

		builder.CreateLine2D(m_map->GetTileAtCoordinate(tileCoords)->GetBounds().GetCenter(), m_map->GetTileAtCoordinate(nextTileCoords)->GetBounds().GetCenter(), Rgba::PINK);
	}

	if (builder.m_vertices.size() > 0)
	{
		pathMesh = builder.CreateMesh<VertexPCU>();
	}

	return pathMesh;
}

//  =========================================================================================
void PlayingState::ClearDisectedAgent()
{
	if (m_disectedAgent != nullptr)
	{
		m_disectedAgent = nullptr;
	}

	m_isCameraLockedToAgent = false;
}

// Commands =============================================================================
//void ToggleOptimized(Command& cmd)
//{
//	Game* theGame = Game::GetInstance();
//
//	//theGame->m_isOptimized = !theGame->m_isOptimized;
//
//	//if (theGame->m_isOptimized)
//	//{
//	//	DevConsolePrintf(Rgba::YELLOW, "Game is optimized!");
//	//}
//	//else
//	//{
//	//	DevConsolePrintf(Rgba::YELLOW, "Game is NOT optimized!");
//	//}
//
//	theGame = nullptr;
//}

//  =========================================================================================
void TogglePaused(Command& cmd)
{
	GetGameClock()->SetPaused(!GetGameClock()->IsPaused());
	
	std::string pauseString = "";

	GetGameClock()->IsPaused() ? pauseString = "Game paused!" : pauseString = "Game unpaused!";

	DevConsolePrintf(Rgba::GREEN, pauseString.c_str());
}

//  =========================================================================================
void DisectAgent(Command& cmd)
{
	GameState* currentState = GameState::GetCurrentGameState();
	if (currentState->m_type != PLAYING_GAME_STATE)
	{
		DevConsolePrintf(Rgba::RED, "NO AGENTS INITIALIZED!");
		return;
	}

	PlayingState* playingState = (PlayingState*)currentState;

	int agentId = cmd.GetNextInt();
	if (agentId == INT_MAX)
	{
		DevConsolePrintf(Rgba::RED, "INVALID AGENT INDEX!");
		playingState->ClearDisectedAgent();
		return;
	}

	playingState->m_disectedAgent = playingState->m_map->GetAgentById(agentId);

	if (playingState->m_disectedAgent == nullptr)
	{
		DevConsolePrintf(Rgba::RED, "INVALID AGENT INDEX!");
		playingState->ClearDisectedAgent();
		return;
	}

	playingState->m_isCameraLockedToAgent = true;
	DevConsolePrintf(Rgba::GREEN, "Disecting agent index %i", agentId);
}

//  =========================================================================================
void ToggleBlockedData(Command & cmd)
{
	g_isBlockedTileDataShown = !g_isBlockedTileDataShown;	
}

//  =========================================================================================
void ToggleAgentIds(Command& cmd)
{
	g_isIdShown = !g_isIdShown;
}
