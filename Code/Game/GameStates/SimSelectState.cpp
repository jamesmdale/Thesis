#include "Game\GameStates\SimSelectState.hpp"


//  =========================================================================================
SimSelectState::~SimSelectState()
{

}

//  =========================================================================================
void SimSelectState::Update(float deltaSeconds)
{

}

//  =========================================================================================
void SimSelectState::PreRender()
{

}

//  =========================================================================================
void SimSelectState::Render()
{

}

//  =========================================================================================
void SimSelectState::PostRender()
{

}

//  =========================================================================================
float SimSelectState::UpdateFromInput(float deltaSeconds)
{
	InputSystem* theInput = InputSystem::GetInstance();

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_W) || theInput->WasKeyJustPressed(theInput->KEYBOARD_UP_ARROW))
	{
		if (m_selectedSimMenuOption == RUN_ALL_SELECT_SIM_OPTION)
		{
			m_selectedSimMenuOption = RUN_FROM_LIST_SELECT_SIM_OPTION;
		}
		else
		{
			m_selectedSimMenuOption = RUN_ALL_SELECT_SIM_OPTION;
		}
	}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_S) || theInput->WasKeyJustPressed(theInput->KEYBOARD_DOWN_ARROW))
	{
		if (m_selectedSimMenuOption == RUN_ALL_SELECT_SIM_OPTION)
		{
			m_selectedSimMenuOption = RUN_FROM_LIST_SELECT_SIM_OPTION;
		}
		else
		{
			m_selectedSimMenuOption = RUN_ALL_SELECT_SIM_OPTION;
		}
	}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_SPACE))
	{
		switch (m_selectedSimMenuOption)
		{
		case(RUN_ALL_SELECT_SIM_OPTION):
			ResetState();
			GameState::TransitionGameStates(GetGameStateFromGlobalListByType(PLAYING_GAME_STATE));
			break;
		case(RUN_FROM_LIST_SELECT_SIM_OPTION):
			break;
		}
	}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_ESCAPE))
	{
		ResetState();
		GameState::TransitionGameStates(GetGameStateFromGlobalListByType(MAIN_MENU_GAME_STATE));
	}


	return deltaSeconds; //new deltaSeconds
}

//  =========================================================================================
void SimSelectState::ResetState()
{

}