#include "Game\GameStates\GameState.hpp"
#include "Engine\Renderer\Renderer.hpp"
#include "Engine\Profiler\Profiler.hpp"

GameState* g_currentState = nullptr;
GameState* g_transitionState = nullptr;

float GameState::s_secondsInState = 0.0f;
float GameState::s_secondsTransitioning = 0.0f;
bool GameState::s_isFinishedTransitioningOut = true;
bool GameState::s_isFinishedTransitioningIn = true;
std::vector<GameState*> GameState::s_gameStates;

GameState::GameState(Camera* camera)
{
	m_camera = camera;
	m_renderScene2D = new RenderScene2D();

	m_renderScene2D->AddCamera(m_camera);
}

GameState::~GameState()
{	
	delete(m_renderScene2D);
	m_renderScene2D = nullptr;

	//game will manage deletion of camera
	m_camera = nullptr; 

	for (int gameStateIndex = 0; gameStateIndex < (int)s_gameStates.size(); gameStateIndex++)
	{
		delete(s_gameStates[gameStateIndex]);
	}
	s_gameStates.clear();
}

void GameState::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

void GameState::PreRender()
{
	//prerender tasks here
}

void GameState::Render()
{
	Renderer* theRenderer = Renderer::GetInstance();
	theRenderer->SetCamera(m_camera);

	//always do this first at the beginning of the frame's render
	theRenderer->ClearDepth(1.f);
	theRenderer->ClearColor(Rgba::BLACK);

	//render from forward rendering path
	Game::GetInstance()->m_forwardRenderingPath2D->Render(m_renderScene2D);

	theRenderer = nullptr;
}

void GameState::PostRender()
{
	//postrender tasks here
}


float GameState::UpdateFromInput(float deltaSeconds)
{
	return deltaSeconds;
}

void GameState::TransitionIn(float secondsTransitioning)
{
	UNUSED(secondsTransitioning);
	/*	does whatever it wants in terms of transitioning. when finished,
	set s_isFinishedTransitioningIn to true
	*/
	s_isFinishedTransitioningIn = true;
}

void GameState::TransitionOut(float secondsTransitioning)
{
	UNUSED(secondsTransitioning);
	/*	does whatever it wants in terms of transitioning. when finished,
		set s_isFinishedTransitioningOut to true
	*/
	s_isFinishedTransitioningOut = true;
}

void GameState::ResetState()
{
	//reset menu options or game states here
}

void GameState::Initialize()
{
	//run initialization tasks here
	m_isInitialized = true;
}

//static methods

void GameState::UpdateGlobalGameState(float deltaSeconds)
{	
	PROFILER_PUSH();

	s_secondsInState += deltaSeconds;

	if (g_transitionState != nullptr)
	{
		s_secondsTransitioning += deltaSeconds;

		//make sure the current state and transition state complete their transitions
		if (s_isFinishedTransitioningOut == false)
		{
			g_currentState->TransitionOut(s_secondsTransitioning);
		}
		else if (s_isFinishedTransitioningIn == false)
		{
			g_transitionState->TransitionIn(s_secondsTransitioning);
		}
		else
		{
			//reset transition state
			g_currentState = g_transitionState;
			FinishTransition();
		}
	}
	
}

void GameState::FinishTransition()
{
	g_currentState = g_transitionState;
	g_transitionState = nullptr;
	s_isFinishedTransitioningIn = true;
	s_isFinishedTransitioningOut = true;
	s_secondsInState = 0.0f;
	s_secondsTransitioning = 0.0f;

	if (!g_currentState->IsInitialized())
	{
		g_currentState->Initialize();
	}	
}

void GameState::TransitionGameStates(GameState* toState)
{
	GUARANTEE_OR_DIE(toState != nullptr, "INVALID MENU STATE TRANSITION");

	g_transitionState = toState;
	s_isFinishedTransitioningOut = false;
	s_isFinishedTransitioningIn = false;
}

void GameState::TransitionGameStatesImmediate( GameState* toState)
{
	GUARANTEE_OR_DIE(toState != nullptr, "INVALID MENU STATE TRANSITION");

	g_transitionState = toState;
	s_isFinishedTransitioningOut = true;
	s_isFinishedTransitioningIn = true;
}

GameState* GameState::GetCurrentGameState()
{
	return g_currentState;
}

GameState* GameState::GetTransitionGameState()
{
	return g_transitionState;
}

//For now, we assume they will only ever have one of each possible type in the list
GameState* GameState::GetGameStateFromGlobalListByType(eGameState gameStateType)
{
	for (int gameStateIndex = 0; gameStateIndex < (int)s_gameStates.size(); gameStateIndex++)
	{
		if (s_gameStates[gameStateIndex]->m_type == gameStateType)
		{
			return s_gameStates[gameStateIndex];
		}
	}

	return nullptr;
}

//For now, we assume they will only ever have one of each possible type in the list
void GameState::AddGameState(GameState* gameState)
{
	s_gameStates.push_back(gameState);
}

float GameState::GetSecondsInCurrentState()
{
	return s_secondsInState;
}


