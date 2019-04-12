#include "Game\TheApp.hpp"
#include "Game\Game.hpp"
#include "Game\GameCommon.hpp"
#include "Game\SimulationData.hpp"
#include "Game\Definitions\SpriteDefinitions\SpriteDefinition.hpp"
#include "Game\Definitions\SpriteDefinitions\IsoSpriteDefinition.hpp"
#include "Game\Definitions\SpriteDefinitions\IsoSpriteAnimDefinition.hpp"
#include "Game\Definitions\SpriteDefinitions\IsoSpriteAnimSetDefinition.hpp"
#include "Engine\Renderer\Renderer.hpp"
#include "Engine\Input\InputSystem.hpp"
#include "Engine\Time\Time.hpp"
#include "Engine\Core\EngineCommon.hpp"
#include "Engine\Core\Command.hpp"
#include "Engine\Core\ErrorWarningAssert.hpp"
#include "Engine\Window\Window.hpp"
#include "Engine\Core\DevConsole.hpp"
#include "Engine\Debug\DebugRender.hpp"
#include "Engine\File\ObjectFileLoader.hpp"
#include "Engine\Audio\AudioSystem.hpp"
#include "Engine\Profiler\Profiler.hpp"
#include "Engine\Profiler\ProfilerConsole.hpp"
#include "Game\GameStates\GameState.hpp"
#include "Engine\File\File.hpp"
#include "Engine\Core\LogSystem.hpp"
#include "Engine\File\CSVEditor.hpp"
#include <thread>
#include <fstream>

TheApp* g_theApp = nullptr;
bool g_isAppPaused = false;

//  =============================================================================
TheApp::TheApp()
{ 
	Game::CreateInstance();
}

//  =============================================================================
TheApp::~TheApp()
{
	TODO("DELETE SYSTEMS AND NULL OUT");
	//g_currentSimulationData->ExportCSV();
}

//  =============================================================================
void TheApp::RunFrame()
{
	//start profiler for frame
	Profiler::GetInstance()->MarkFrame();

	//begin frame for engine systems
	Renderer::GetInstance()->BeginFrame();	
	InputSystem::GetInstance()->BeginFrame();
	AudioSystem::GetInstance()->BeginFrame();
	MasterClockBeginFrame();

	Update();
	PreRender();
	Render();
	PostRender();

	AudioSystem::GetInstance()->EndFrame();
	DebugRender::GetInstance()->EndFrame();
	InputSystem::GetInstance()->EndFrame();
	Renderer::GetInstance()->EndFrame();

	Sleep(1);
	TODO("Need to add sleep function to release CPU cycles and reduce system demand");
}

//  =============================================================================
void TheApp::Initialize()
{
	//register app commands
	RegisterCommand("quit", CommandRegistration(Quit, ": Use to quit the program", "Quitting..."));

	//start the masterclock
	Clock* masterClock = GetMasterClock();
	masterClock->ClockSystemStartup();

	//init mouse input settings
	InputSystem::GetInstance()->GetMouse()->MouseLockToScreen(false);
	InputSystem::GetInstance()->GetMouse()->MouseShowCursor(true);
	InputSystem::GetInstance()->GetMouse()->SetMouseMode(MOUSE_ABSOLUTE_MODE);	

	Game::CreateInstance();
	Game::GetInstance()->Initialize();

	InitializeGameResources();

	WriteTestCSV();
}

//  =============================================================================
void TheApp::Update()
{
	PROFILER_PUSH();

	float deltaSeconds = GetMasterDeltaSeconds();

	float modifiedDeltaSeconds = deltaSeconds;
	
	if (g_isAppPaused)
	{
		modifiedDeltaSeconds = 0.0f;
	}

	//test for getting master delta seconds
	double unclampedFrameTime = GetUnclampedMasterDeltaSeconds();
	double unclampedFPS = GetUnclampedFPS();

	deltaSeconds = UpdateInput(deltaSeconds);

	Game::GetInstance()->Update();

	//if(DebugRender::GetInstance()->IsEnabled())
	//{
	//	DebugRender::GetInstance()->Update(modifiedDeltaSeconds);
	//}

	if (ProfilerConsole::GetInstance()->IsOpen())
	{
		ProfilerConsole::GetInstance()->UpdateFromInput();
		ProfilerConsole::GetInstance()->Update();
	}

	if(DevConsole::GetInstance()->IsOpen())
	{
		DevConsole::GetInstance()->Update(deltaSeconds);
	}
}

//  =============================================================================
void TheApp::PreRender()
{
	//PROFILER_PUSH();

	Game::GetInstance()->PreRender();

	if (ProfilerConsole::GetInstance()->IsOpen())
	{
		ProfilerConsole::GetInstance()->PreRender();
	}
}

//  =============================================================================
void TheApp::Render()
{
	PROFILER_PUSH();
	//set up screen
	Game::GetInstance()->Render();

	/*if(DebugRender::GetInstance()->IsEnabled())
	{
		DebugRender::GetInstance()->Render();
	}*/

	if (ProfilerConsole::GetInstance()->IsOpen())
	{
		ProfilerConsole::GetInstance()->Render();
	}

	if(DevConsole::GetInstance()->IsOpen())
	{
		DevConsole::GetInstance()->Render();
	}	
}

//  =============================================================================
void TheApp::PostRender()
{
	Game::GetInstance()->PostRender();
}

//  =============================================================================
float TheApp::UpdateInput(float deltaSeconds)
{
	//PROFILER_PUSH();

	if(InputSystem::GetInstance()->WasKeyJustPressed((InputSystem::GetInstance()->KEYBOARD_TILDE)))
	{
		if(!DevConsole::GetInstance()->IsOpen())
		{
			DevConsole::GetInstance()->Open();
		}
		else
		{
			DevConsole::GetInstance()->Close();
		}		
	}
	
	if (!DevConsole::GetInstance()->IsOpen())
	{
		if (InputSystem::GetInstance()->WasKeyJustPressed(InputSystem::GetInstance()->KEYBOARD_M) && !ProfilerConsole::GetInstance()->IsOpen())
		{
			AudioSystem::GetInstance()->ToggleMasterMute();
		}				

		deltaSeconds = Game::GetInstance()->UpdateInput(deltaSeconds);
	}

	return deltaSeconds;
}
//  =========================================================================================
void TheApp::InitializeGameResources()
{
	Renderer* theRenderer = Renderer::GetInstance();

	//load sprite sheet images
	theRenderer->CreateOrGetTexture("Data/Images/Agents/Female_1.png");
	theRenderer->CreateOrGetTexture("Data/Images/Agents/Female_2.png");
	theRenderer->CreateOrGetTexture("Data/Images/Agents/Female_3.png");
	theRenderer->CreateOrGetTexture("Data/Images/Agents/Female_4.png");
	theRenderer->CreateOrGetTexture("Data/Images/Agents/Male_1.png");
	theRenderer->CreateOrGetTexture("Data/Images/Agents/Male_2.png");
	theRenderer->CreateOrGetTexture("Data/Images/Agents/Male_3.png");
	theRenderer->CreateOrGetTexture("Data/Images/Agents/Male_4.png");


	//initialize agent definition
	SpriteDefinition::Initialize("Data/Definitions/AgentDefinitions/to_agent_sprite.xml");
	IsoSpriteDefinition::Initialize("Data/Definitions/AgentDefinitions/agent_isosprite.xml");
	IsoSpriteAnimDefinition::Initialize("Data/Definitions/AgentDefinitions/agent_animation.xml");
	IsoSpriteAnimSetDefinition::Initialize("Data/Definitions/AgentDefinitions/agent_animset.xml");

	theRenderer = nullptr;
}

//  =============================================================================
// command callbacks =========================================================================================
//  =============================================================================
void Quit(Command &cmd)
{
	DevConsolePrintf(cmd.m_commandInfo->m_successMessage.c_str());
	g_isQuitting = true;
}

//  =========================================================================================
void WriteTestCSV()
{
	RemoveFile("Data\\ConsoleLogs\\CSVTest.csv");

	CSVEditor writer;

	writer.AddCell("Cell A1");
	writer.AddCell("Cell A2");
	writer.AddNewLine();
	writer.AddCell("Cell B1");
	writer.AddCell("Cell B2");
	writer.AddCell("Cell B3");
	writer.AddCell("Cell B4");
	writer.AddNewLine();
	writer.AddNewLine();
	writer.AddCell("Cell D1, Test");
	writer.AddCell("Cell D2");

	bool success = writer.WriteToFile("Data\\ConsoleLogs\\CSVTest.csv");
}
