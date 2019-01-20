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
#include <map>
#include <string>
#include "Game\GameStates\PlayingState.hpp"
#include "Game\PointOfInterest.hpp"
#include "Game\Agent.hpp"
#include "Game\Definitions\SimulationDefinition.hpp"
#include "Game\SimulationData.hpp"

float ORTHO_MAX = 0.f;
float ORTHO_MIN = 0.f;
float g_orthoZoom = 0.f;

float ZOOM_RATE = 5.f;

std::string simDataOutputDirectory = "";
bool isResetingSimulation = false;

//  =============================================================================
PlayingState::~PlayingState()
{
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
	RegisterCommand("toggle_optimization", CommandRegistration(ToggleOptimized, ": Toggle blanket optimizations on and off", ""));

	std::string newFolderName = Stringf("SIMULATION_TEST_%s", GetCurrentDateTime().c_str());
	std::string newPath =  Stringf("%s%s", "Data\\ExportedSimulationData\\", newFolderName.c_str());
	
	bool success = CreateFolder(newPath.c_str());
	ASSERT_OR_DIE(success, Stringf("UNABLE TO CREATE FOLDER (%s)", newPath.c_str()).c_str());

	simDataOutputDirectory = Stringf("%s%s", newPath.c_str(), "\\");

	g_currentSimulationDefinition = SimulationDefinition::s_simulationDefinitions[g_currentSimDefinitionIndex];

	CreateMapForSimulation(g_currentSimulationDefinition);
	InitializeSimulation(g_currentSimulationDefinition);

	//set per frame budget for 60fps
	g_perFrameHPCBudget = SecondsToPerformanceCounter(1.0 / 70.0);

	//cleanup
	theRenderer = nullptr;
	theWindow = nullptr;
}

//  =============================================================================
void PlayingState::Update(float deltaSeconds)
{ 
	m_map->Update(deltaSeconds);

	if (m_simulationTimer->ResetAndDecrementIfElapsed())
	{
		isResetingSimulation = true;
	}
}

//  =============================================================================
void PlayingState::PreRender()
{
}

//  =============================================================================
void PlayingState::Render()
{
	SimpleTimer timer;
	timer.Start();

	Renderer* theRenderer = Renderer::GetInstance();

	Game::GetInstance()->m_forwardRenderingPath2D->Render(m_renderScene2D);

	RenderGame();
	RenderUI();

	timer.Stop();
	g_previousFrameRenderTime = timer.GetRunningTime();

	theRenderer = nullptr;
}

//  =============================================================================
void PlayingState::PostRender()
{
	if(g_isQuitting)
		return;

	m_map->DeleteDeadEntities();

	if (isResetingSimulation)
	{
		ExportSimulationData();
		ResetCurrentSimulationData();

		g_currentSimDefinitionIndex++;
		isResetingSimulation = false;

		if (g_currentSimDefinitionIndex < SimulationDefinition::s_simulationDefinitions.size())
		{
			SimulationDefinition* definition = SimulationDefinition::s_simulationDefinitions[g_currentSimDefinitionIndex];	

			//if we are on the same map, leave the spawn points the same.
			if (definition->m_mapDefinition == g_currentSimulationDefinition->m_mapDefinition)
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

	if (theInput->IsKeyPressed(theInput->KEYBOARD_DOWN_ARROW))
	{
		m_camera->Translate(Vector3(Vector2::DOWN * 5.f * deltaSeconds));
	}
	if (theInput->IsKeyPressed(theInput->KEYBOARD_UP_ARROW))
	{
		m_camera->Translate(Vector3(Vector2::UP * 5.f * deltaSeconds));
	}
	if (theInput->IsKeyPressed(theInput->KEYBOARD_RIGHT_ARROW))
	{
		m_camera->Translate(Vector3(Vector2::RIGHT * 5.f * deltaSeconds));
	}
	if (theInput->IsKeyPressed(theInput->KEYBOARD_LEFT_ARROW))
	{
		m_camera->Translate(Vector3(Vector2::LEFT * 5.f * deltaSeconds));
	}

	//if (theInput->IsKeyPressed(theInput->KEYBOARD_PAGEUP))
	//{
	//	g_orthoZoom -= deltaSeconds * ZOOM_RATE;
	//	if(g_orthoZoom < ORTHO_MIN)
	//	{
	//		g_orthoZoom = ORTHO_MIN;
	//	}
	//	m_camera->SetProjectionOrtho(g_orthoZoom, CLIENT_ASPECT, -1000.f, 1000.f);
	//}

	//if (theInput->IsKeyPressed(theInput->KEYBOARD_PAGEDOWN))
	//{
	//	g_orthoZoom += deltaSeconds * ZOOM_RATE;
	//	if(g_orthoZoom > ORTHO_MAX)
	//	{
	//		g_orthoZoom = ORTHO_MAX;
	//	}

	//	m_camera->SetProjectionOrtho(g_orthoZoom, CLIENT_ASPECT, -1000.f, 1000.f);
	//}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_R))
	{
		g_orthoZoom = Window::GetInstance()->GetClientHeight();
		m_camera->SetProjectionOrtho(g_orthoZoom, CLIENT_ASPECT, -1000.f, 1000.f);
	}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_T))
	{
		g_isBlockedTileDataShown = !g_isBlockedTileDataShown;	
	}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_I))
	{
		g_isIdShown = !g_isIdShown;
	}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_F))
	{
		g_isDebugDataShown = !g_isDebugDataShown;
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
void PlayingState::RenderUI()
{
	//TODO("Render UI");
	Renderer* theRenderer = Renderer::GetInstance()->GetInstance();
	theRenderer->SetCamera(m_uiCamera);

	Mesh* textMesh = CreateTextMesh();

	if (textMesh != nullptr)
	{
		theRenderer->BindMaterial(theRenderer->CreateOrGetMaterial("text"));
		theRenderer->DrawMesh(textMesh);
	}	

	theRenderer->SetCamera(m_camera);

	delete(textMesh);
	textMesh = nullptr;
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

	m_simulationTimer = new Stopwatch(GetMasterClock());
	m_simulationTimer->SetTimer(g_generalSimulationData->m_simulationDefinitionReference->m_totalProcessingTimeInSeconds);
}

//  =============================================================================
void PlayingState::CreateMapForSimulation(SimulationDefinition* definition)
{
	//map creation
	m_map = new Map(definition, "TestMap", m_renderScene2D);	
	m_map->Initialize();
	m_map->m_gameState = this;
}

//  =============================================================================
void PlayingState::ResetMapForSimulation(SimulationDefinition* definition)
{
	m_map->Reload(definition);
	m_map->m_gameState = this;
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

Mesh* PlayingState::CreateTextMesh()
{
	MeshBuilder* builder = new MeshBuilder();
	Mesh* textMesh = nullptr;
	Window* theWindow = Window::GetInstance();

	//fps counter
	if (g_isDebugDataShown)
	{
		AABB2 fpsBox = AABB2(theWindow->GetClientWindow(), Vector2(0.8f, 0.9f), Vector2(0.95f, 0.975f));

		builder->CreateText2DInAABB2( fpsBox.GetCenter(), fpsBox.GetDimensions(), 1.f, Stringf("FPS: %f", GetUnclampedFPS()), Rgba::WHITE);
	}

	if (GetIsAgentUpdateBudgeted() && g_isDebugDataShown)
	{
		AABB2 agentsUpdatedBox = AABB2(theWindow->GetClientWindow(), Vector2(0.8f, 0.8f), Vector2(0.95f, 0.875f));

		builder->CreateText2DInAABB2( agentsUpdatedBox.GetCenter(), agentsUpdatedBox.GetDimensions(), 1.f, Stringf("Agents Updated: %i", g_agentsUpdatedThisFrame), Rgba::WHITE);
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

// Commands =============================================================================
void ToggleOptimized(Command& cmd)
{
	Game* theGame = Game::GetInstance();

	//theGame->m_isOptimized = !theGame->m_isOptimized;

	//if (theGame->m_isOptimized)
	//{
	//	DevConsolePrintf(Rgba::YELLOW, "Game is optimized!");
	//}
	//else
	//{
	//	DevConsolePrintf(Rgba::YELLOW, "Game is NOT optimized!");
	//}

	theGame = nullptr;
}
