#include "Game\GameStates\PlayingState.hpp"
#include "Game\Entities\PointOfInterest.hpp"
#include "Game\Agents\Agent.hpp"
#include "Game\Definitions\SimulationDefinition.hpp"
#include "Game\SimulationData.hpp"
#include "Game\UI\DebugInputBox.hpp"
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
#include "Engine\Profiler\Profiler.hpp"
#include <map>
#include <string>



float ORTHO_MAX = 0.f;
float ORTHO_MIN = 0.f;
float g_orthoZoom = 0.f;

float g_currentFps = 0.f;
float g_minFPS = 0.f;
Stopwatch* g_minFPSResetStopwatch = nullptr;

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

	delete(m_inputDelayTimer);
	m_inputDelayTimer = nullptr;

	delete(m_simulationTimer);
	m_simulationTimer = nullptr;
}

//  =============================================================================
void PlayingState::Initialize()
{
	//PROFILER_PUSH();

	Window* theWindow = Window::GetInstance();
	Renderer* theRenderer = Renderer::GetInstance();
	Game* theGame = Game::GetInstance();

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
	g_currentSimulationDefinition = theGame->m_selectedDefinitions[theGame->m_currentSimDefinitionIndex];

	CreateMapForSimulation(g_currentSimulationDefinition);
	InitializeSimulation(g_currentSimulationDefinition);

	//set per frame budget for 60fps
	g_perFrameHPCBudget = SecondsToPerformanceCounter(1.0 / 70.0);

	//generate debug input even if we don't really use it
	DebugInputBox::CreateInstance();

	//move camera to starting location
	m_camera->SetPosition(Vector3::ZERO);

	g_minFPSResetStopwatch = new Stopwatch(GetMasterClock());
	g_minFPSResetStopwatch->SetTimer(10.f);

	m_inputDelayTimer = new Stopwatch(GetMasterClock());
	m_inputDelayTimer->SetTimer(UPDATE_INPUT_DELAY);

	//cleanup
	theRenderer = nullptr;
	theWindow = nullptr;
}

//  =============================================================================
void PlayingState::Update(float deltaSeconds)
{ 
	PROFILER_PUSH();

	if (!GetGameClock()->IsPaused())
	{
		m_map->Update(deltaSeconds);

		UpdateFPSCounters();

		if (m_simulationTimer->ResetAndDecrementIfElapsed())
		{
			isResetingSimulation = true;
		}		
	}
}

//  =============================================================================
void PlayingState::PreRender()
{
	PROFILER_PUSH();

	if (m_isCameraLockedToAgent)
	{
		//have the camera follow the agent
		m_camera->SetPosition(Vector3(m_disectedAgent->m_position) * 0.5f);
	}
}

//  =============================================================================
void PlayingState::Render()
{
	PROFILER_PUSH();

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
	PROFILER_PUSH();

	if (g_isQuitting)
		return;

	m_map->DeleteDeadEntities();

	if (isResetingSimulation)
	{
		LoadNextSim();
		m_camera->SetPosition(Vector3(m_map->GetCenter()));
	}	
}

//  =============================================================================
float PlayingState::UpdateFromInput(float deltaSeconds)
{
	InputSystem* theInput = InputSystem::GetInstance();

	//movement should run on the master deltaseconds so we can move the camera even if the game is paused
	if (theInput->IsKeyPressed(theInput->KEYBOARD_DOWN_ARROW) && !theInput->IsKeyPressed(theInput->KEYBOARD_CONTROL))
	{
		m_camera->Translate(Vector3(Vector2::DOWN * 5.f * GetMasterDeltaSeconds()));
		m_isCameraLockedToAgent = false;
	}
	if (theInput->IsKeyPressed(theInput->KEYBOARD_UP_ARROW) && !theInput->IsKeyPressed(theInput->KEYBOARD_CONTROL))
	{
		m_camera->Translate(Vector3(Vector2::UP * 5.f * GetMasterDeltaSeconds()));
		m_isCameraLockedToAgent = false;

	}
	if (theInput->IsKeyPressed(theInput->KEYBOARD_RIGHT_ARROW) && !theInput->IsKeyPressed(theInput->KEYBOARD_CONTROL))
	{
		m_camera->Translate(Vector3(Vector2::RIGHT * 5.f * GetMasterDeltaSeconds()));
		m_isCameraLockedToAgent = false;
	}
	if (theInput->IsKeyPressed(theInput->KEYBOARD_LEFT_ARROW) && !theInput->IsKeyPressed(theInput->KEYBOARD_CONTROL))
	{
		m_camera->Translate(Vector3(Vector2::LEFT * 5.f * GetMasterDeltaSeconds()));
		m_isCameraLockedToAgent = false;
	}

	// cycle through agents ----------------------------------------------
	if (theInput->IsKeyPressed(theInput->KEYBOARD_UP_ARROW) && theInput->IsKeyPressed(theInput->KEYBOARD_CONTROL) && m_inputDelayTimer->HasElapsed())
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

		m_inputDelayTimer->Reset();
	}
	if (theInput->IsKeyPressed(theInput->KEYBOARD_DOWN_ARROW) && theInput->IsKeyPressed(theInput->KEYBOARD_CONTROL) && m_inputDelayTimer->HasElapsed())
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

		m_inputDelayTimer->Reset();
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

	//  =========================================================================================
	if (InputSystem::GetInstance()->WasKeyJustPressed(InputSystem::GetInstance()->KEYBOARD_ESCAPE))
	{
		g_isQuitting = true;
	}
	
	return deltaSeconds; //new deltaSeconds
}

//  =========================================================================================
void PlayingState::TransitionIn(float secondsTransitioning)
{
	Game* theGame = Game::GetInstance();
	g_currentSimulationDefinition = theGame->m_selectedDefinitions[theGame->m_currentSimDefinitionIndex];

	CreateMapForSimulation(g_currentSimulationDefinition);
	InitializeSimulation(g_currentSimulationDefinition);

	s_isFinishedTransitioningIn = true;
}

//  =========================================================================================
void PlayingState::TransitionOut(float secondsTransitioning)
{
	ResetCurrentSimulationData();
	ResetState();
	s_isFinishedTransitioningOut = true;
}

//  =========================================================================================
void PlayingState::ResetState()
{
	Game* theGame = Game::GetInstance();
	theGame->m_currentSimDefinitionIndex = 0;
}

//  =========================================================================================
void PlayingState::UpdateFPSCounters()
{
	g_currentFps = GetUnclampedFPS();
	if (g_currentFps < g_minFPS)
	{
		g_minFPS = g_currentFps;
	}
	if (g_minFPSResetStopwatch->ResetAndDecrementIfElapsed())
	{
		g_minFPS = g_currentFps;
	}	
}

//  =========================================================================================
void PlayingState::RenderGame()
{
	PROFILER_PUSH();

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

		theRenderer->BindMaterial(theRenderer->CreateOrGetMaterial("default"));
		theRenderer->DrawDottedDisc2WithColor(m_disectedAgent->m_physicsDisc, Rgba::PINK, 10);
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

#ifdef CollisionDataAnalysis
	//queue path data
	g_collisionData = new SimulationData();
	g_collisionData->Initialize(g_currentSimulationDefinition);
#endif // CollisionDataAnalysis
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

#ifdef CollisionDataAnalysis
	delete(g_collisionData);
	g_collisionData = nullptr;
#endif

	g_numUpdatePlanCalls = 0;
	g_numActionStackProcessCalls = 0;
	g_numAgentUpdateCalls = 0;
	g_numGetPathCalls = 0;
	g_numCopyPathCalls = 0;
	g_numQueueActionPathCalls = 0;
	g_numCollisionCalls = 0;
}

//  =============================================================================
void PlayingState::DeleteMap()
{
	delete(m_map);
	m_map = nullptr;
}

//  =========================================================================================
void PlayingState::LoadNextSim()
{
	Game* theGame = Game::GetInstance();
	ExportSimulationData();
	ResetCurrentSimulationData();

	theGame->m_currentSimDefinitionIndex++;
	isResetingSimulation = false;
	m_disectedAgent = nullptr;

	if (theGame->m_currentSimDefinitionIndex < theGame->m_selectedDefinitions.size())
	{
		SimulationDefinition* definition = theGame->m_selectedDefinitions[theGame->m_currentSimDefinitionIndex];

		//if we are on the same map, leave the spawn points the same.
	/*	if(definition->m_mapDefinition == g_currentSimulationDefinition->m_mapDefinition)
		{
			g_currentSimulationDefinition = definition;
			ResetMapForSimulation(definition);
			InitializeSimulation(definition);
		}
		else
		{*/
		g_currentSimulationDefinition = definition;
		DeleteMap();
		CreateMapForSimulation(definition);
		InitializeSimulation(definition);
		//}		
	}
	else
	{
		ResetState();
		GameState::TransitionGameStates(GetGameStateFromGlobalListByType(MAIN_MENU_GAME_STATE));
	}	

	//reset timer regardless
	m_simulationTimer->Reset();
}

//  =============================================================================
void PlayingState::ExportSimulationData()
{
	std::string newFolder = "";
	bool isFolderValid = false;
	int iterationCount = 0;

	newFolder = Stringf("%s%s%s", simDataOutputDirectory.c_str(), "Simulation_Definition_", g_currentSimulationDefinition->m_name.c_str());
	while (!isFolderValid)
	{
		if (DoesFileExist(newFolder))
		{
			if (iterationCount > 0)
				newFolder.erase(newFolder.end() - 2, newFolder.end());

			++iterationCount;
			
			newFolder.append(Stringf("_%i", iterationCount).c_str());
		}
		else
		{
			isFolderValid = true;
		}
			
	}

	CreateFolder(newFolder.c_str());
	std::string finalFilePath = Stringf("%s%s", newFolder.c_str(), "\\");

	//general data
	std::string generalInfoFolder = Stringf("%s%s", finalFilePath.c_str(), "GeneralInfo");
	std::string generalInfoFilePath = Stringf("%s%s", finalFilePath.c_str(), "GeneralInfo\\");

	CreateFolder(generalInfoFolder.c_str());
	FinalizeGeneralSimulationData();

	std::string fileName = Stringf("GeneralInfo_%s.csv", g_currentSimulationDefinition->m_name.c_str());
	bool success = g_generalSimulationData->ExportCSV(generalInfoFilePath, fileName.c_str());
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
	ASSERT_OR_DIE(success, "Que action pathing broken");
#endif

#ifdef DistanceMemoizationDataAnalysis
	fileName = Stringf("DistanceMemoizationUtilityTimes_%s.csv", g_currentSimulationDefinition->m_name.c_str());
	success = g_distanceMemoizationData ->ExportCSV(finalFilePath, fileName.c_str());
	ASSERT_OR_DIE(success, "Distance memoization broken");
#endif

#ifdef CollisionDataAnalysis
	fileName = Stringf("CollisionCalculationTimes_%s.csv", g_currentSimulationDefinition->m_name.c_str());
	success = g_collisionData ->ExportCSV(finalFilePath, fileName.c_str());
	ASSERT_OR_DIE(success, "Collision data broken");
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
Mesh* PlayingState::CreateUITextMesh()
{
	return nullptr;
}

//  =========================================================================================
Mesh* PlayingState::CreateUIDebugTextMesh()
{
	MeshBuilder builder = MeshBuilder();
	Mesh* textMesh = nullptr;
	Window* theWindow = Window::GetInstance();

	// fps counter ----------------------------------------------		
	AABB2 fpsBox = AABB2(theWindow->GetClientWindow(), Vector2(0.8f, 0.9f), Vector2(0.95f, 0.975f));
	builder.CreateText2DInAABB2( fpsBox.GetCenter(), fpsBox.GetDimensions(), 1.f, Stringf("FPS: %f", g_currentFps), Rgba::WHITE);

	fpsBox = AABB2(theWindow->GetClientWindow(), Vector2(0.8f, 0.85f), Vector2(0.95f, 0.9f));
	builder.CreateText2DInAABB2( fpsBox.GetCenter(), fpsBox.GetDimensions(), 1.f, Stringf("MIN FPS: %f", g_minFPS), Rgba::WHITE);

	//cam position information ----------------------------------------------
	AABB2 camPosBox = AABB2(theWindow->GetClientWindow(), Vector2(0.8f, 0.8f), Vector2(0.95f, 0.85f));
	Vector3 camPosition = m_camera->m_transform->GetWorldPosition();
	builder.CreateText2DInAABB2( camPosBox.GetCenter(), camPosBox.GetDimensions(), 1.f, Stringf("Cam Pos: %f,%f", camPosition.x, camPosition.y));

	//agent update per frameinfo ----------------------------------------------
	if (GetIsAgentUpdateBudgeted())
	{
		AABB2 agentsUpdatedBox = AABB2(theWindow->GetClientWindow(), Vector2(0.8f, 0.75f), Vector2(0.95f, 0.8f));
		builder.CreateText2DInAABB2( agentsUpdatedBox.GetCenter(), agentsUpdatedBox.GetDimensions(), 1.f, Stringf("Agents Updated: %i", g_agentsUpdatedThisFrame), Rgba::WHITE);
	}
	
	//threat ----------------------------------------------
	AABB2 agentsUpdatedBox = AABB2(theWindow->GetClientWindow(), Vector2(0.8f, 0.7f), Vector2(0.95f, 0.8f));
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

		float printHeight = 0.65f;

		for (int agentInfoIndex = 0; agentInfoIndex < agentInfo.size(); ++agentInfoIndex)
		{
			builder.CreateText2DFromPoint( Vector2(theWindow->GetClientWidth() * 0.8f , theWindow->GetClientHeight() * printHeight), theWindow->GetClientHeight() * 0.01, 1.f, agentInfo[agentInfoIndex].c_str(), Rgba::WHITE );
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
	for (int poiIndex = 0; poiIndex < m_map->m_pointsOfInterest.size(); ++poiIndex)
	{
		AABB2 poiBounds = m_map->m_pointsOfInterest[poiIndex]->GetWorldBounds();
		builder.CreateText2DInAABB2( poiBounds.GetCenter(), poiBounds.GetDimensions(), 1.f, Stringf("%i", (int)m_map->m_pointsOfInterest[poiIndex]->m_health), Rgba::WHITE);
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

	//get player to first position
	Vector2 agentPosition = m_disectedAgent->m_position;

	//get current index
	uint8_t pathIndex = m_disectedAgent->m_currentPathIndex;
	if(pathIndex == UINT8_MAX)
		pathIndex = m_disectedAgent->m_currentPath.size() -1 ;

	//build player to next position in path index
	Vector2 nextTileCenter = m_disectedAgent->m_currentPath[pathIndex];
	builder.CreateLine2D(agentPosition, nextTileCenter, Rgba::PINK);

	if (m_disectedAgent->m_currentPath.size() != 1)
	{
		//we know the agent is set so we don't have to check for nullptr case
		for (int pathIndex = 1; pathIndex < m_disectedAgent->m_currentPath.size() - 1; ++pathIndex)
		{
			builder.CreateLine2D(m_disectedAgent->m_currentPath[pathIndex], m_disectedAgent->m_currentPath[pathIndex + 1], Rgba::PINK);
		}
	}	

	pathMesh = builder.CreateMesh<VertexPCU>();	

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
