#include <stdlib.h>
#include "Game\TheApp.hpp"
#include "Game\Game.hpp"
#include "Game\GameCommon.hpp"
#include "Game\GameStates\GameState.hpp"
#include "Game\GameStates\MainMenuState.hpp"
#include "Game\GameStates\LoadingState.hpp"
#include "Game\GameStates\PlayingState.hpp"
#include "Engine\Renderer\Renderer.hpp"
#include "Engine\Core\EngineCommon.hpp"
#include "Engine\Window\Window.hpp"
#include "Engine\Core\EngineCommon.hpp"
#include "Game\Definitions\SimulationDefinition.hpp"
#include <vector>
#include <string>


//game instance
static Game* g_theGame = nullptr;

bool m_isPaused = false;

//  =========================================================================================
Game::Game()
{
	m_forwardRenderingPath2D = new ForwardRenderingPath2D();
}

//  =========================================================================================
Game::~Game()
{
	// delete render members
	delete(m_forwardRenderingPath2D);
	m_forwardRenderingPath2D = nullptr;

	// delete camera members
	delete(m_gameCamera);
	m_gameCamera = nullptr;

	delete(m_gameCamera);
	m_gameCamera = nullptr;

	//cleanup global members

	//add any other data to cleanup
}

//  =========================================================================================
Game* Game::GetInstance()
{
	return g_theGame;
}

//  =========================================================================================
Game* Game::CreateInstance()
{
	if (g_theGame == nullptr)
	{
		g_theGame = new Game();
	}

	return g_theGame;
}

//  =========================================================================================
void Game::Initialize()
{
	Window* theWindow = Window::GetInstance();
	Renderer* theRenderer = Renderer::GetInstance();

	//set game common values after window has been started up
	g_tileSize = 1.f;
	g_divideTileSize = 1 / g_tileSize;
	g_halfTileSize = g_tileSize * 0.5f;
	g_bombardmentExplosionSize = g_halfTileSize * 4;

	theRenderer->SetAmbientLightIntensity(0.15f);

	g_gameClock = new Clock(GetMasterClock());

	// add cameras
	m_gameCamera = new Camera();
	m_gameCamera->SetColorTarget(theRenderer->GetDefaultRenderTarget());
	m_gameCamera->SetOrtho(0.f, theWindow->m_clientWidth, 0.f, theWindow->m_clientHeight, -1000.f, 1000.f);
	m_gameCamera->SetView(Matrix44::IDENTITY);

	// add menu states
	TODO("Add other menu states");
	GameState::AddGameState(new MainMenuState(m_gameCamera));
	GameState::AddGameState(new LoadingState(m_gameCamera));
	GameState::AddGameState(new PlayingState(m_gameCamera));

	// set to initial menu
	GameState::TransitionGameStatesImmediate(GameState::GetGameStateFromGlobalListByType(MAIN_MENU_GAME_STATE));
	GameState::UpdateGlobalGameState(0.f);

	//load definitions
	InitializeTileDefinitions();
	InitializeMapDefinitions();
	InitializeAgentDefinitions();
	InitializeSimulationDefinitions();


	// cleanup
	theRenderer = nullptr;
	theWindow = nullptr;
}

//  =========================================================================================
void Game::Update()
{
	float deltaSeconds = g_gameClock->GetDeltaSeconds();

	if (g_theApp->GetPauseState())
	{
		deltaSeconds = 0.f;
	}

	// update global menu data (handles transitions and timers) =============================================================================
	GameState::UpdateGlobalGameState(deltaSeconds);

	GameState::GetCurrentGameState()->Update(deltaSeconds);
}

//  =========================================================================================
void Game::PreRender()
{
	GameState::GetCurrentGameState()->PreRender();
}

//  =========================================================================================
void Game::Render()
{
	GameState::GetCurrentGameState()->Render();
}

//  =========================================================================================
void Game::PostRender()
{
	GameState::GetCurrentGameState()->PostRender();
}

//  =========================================================================================
float Game::UpdateInput(float deltaSeconds)
{
	deltaSeconds = GameState::GetCurrentGameState()->UpdateFromInput(deltaSeconds);

	return deltaSeconds;
}

//  =========================================================================================
void Game::InitializeTileDefinitions()
{
	TileDefinition::Initialize("Data/Definitions/TileDefinitions/TileDefinition.xml");
}

//  =========================================================================================
void Game::InitializeMapDefinitions()
{
	MapDefinition::Initialize("Data/Definitions/MapDefinitions/MapDefinition.xml");
}

//  =========================================================================================
void Game::InitializeAgentDefinitions()
{
	//unused atm 11/13/2018
}

//  =============================================================================
void Game::InitializeSimulationDefinitions()
{
	SimulationDefinition::Initialize("Data/Simulations/Simulations.xml");
}

// pause command =============================================================================
void ToggleGamePaused(Command& cmd)
{
	m_isPaused =  !m_isPaused;
}
