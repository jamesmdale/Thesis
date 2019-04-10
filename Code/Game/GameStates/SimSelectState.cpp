#include "Game\GameStates\SimSelectState.hpp"
#include "Engine\Core\StringUtils.hpp"
#include "Engine\Window\Window.hpp"


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
	//nothing for now
}

//  =========================================================================================
void SimSelectState::Render()
{
	Renderer* theRenderer = Renderer::GetInstance();
	Window* theWindow = Window::GetInstance();

	theRenderer->SetCamera(m_camera);

	theRenderer->ClearDepth(1.f);
	theRenderer->ClearColor(Rgba::BLACK);

	theRenderer->SetTexture(*m_backGroundTexture);
	theRenderer->SetShader(theRenderer->m_defaultShader);

	theRenderer->m_defaultShader->EnableColorBlending(BLEND_OP_ADD, BLEND_SOURCE_ALPHA, BLEND_ONE_MINUS_SOURCE_ALPHA);
	theRenderer->DrawAABB(theWindow->GetClientWindow(), Rgba(0.f, 0.f, 0.f, 1.f));

	RenderInstructions();
	RenderSelectableBox();
	RenderSelectedBox();
	RenderExecuteBox();
	RenderClearBox();
	RenderReloadBox();
}

//  =========================================================================================
void SimSelectState::PostRender()
{
	//nothing for now
}

//  =========================================================================================
float SimSelectState::UpdateFromInput(float deltaSeconds)
{
	InputSystem* theInput = InputSystem::GetInstance();

	switch (m_selectedBox)
	{
	case SELECTABLE_SIMS_BOX:
		UpdateInputSelectableBox();
		break;
	case SELECTED_SIMS_BOX:
		UpdateInputSelectedBox();
		break;
	case RELOAD_OPTION_BOX:
		UpdateInputReloadBox();
		break;
	case EXECUTE_OPTION_BOX:
		UpdateInputExecuteBox();
		break;
	case CLEAR_OPTION_BOX:
		UpdateInputClearBox();
		break;
	}

	//quit to main on escape
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_ESCAPE))
		GameState::TransitionGameStates(GetGameStateFromGlobalListByType(MAIN_MENU_GAME_STATE));


	return deltaSeconds; //new deltaSeconds
}

//  =========================================================================================
void SimSelectState::Initialize()
{
	for (uint definitionIndex = 0; definitionIndex < (uint)SimulationDefinition::s_simulationDefinitions.size(); ++definitionIndex)
	{
		SimulationUIOption option = SimulationUIOption(definitionIndex, SimulationDefinition::s_simulationDefinitions[0]);
		m_selectableSimulationDefinitions.push_back(option);
	}
}

//  =========================================================================================
void SimSelectState::TransitionIn(float secondsTransitioning)
{
	m_selectedBox = SELECTABLE_SIMS_BOX;
	m_selectedOptionIndex = 0;

	s_isFinishedTransitioningIn = true;
}

//  =========================================================================================
void SimSelectState::TransitionOut(float secondsTransitioning)
{
	m_selectedBox = SELECTABLE_SIMS_BOX;
	m_selectedOptionIndex = 0;

	s_isFinishedTransitioningOut = true;
}

//  =========================================================================================
void SimSelectState::UpdateInputSelectableBox()
{
	InputSystem* theInput = InputSystem::GetInstance();

	//if they press the up key, navigate one up in the selectable list
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_W) || theInput->WasKeyJustPressed(theInput->KEYBOARD_UP_ARROW))
	{
		if(m_selectedOptionIndex > 0)
			m_selectedOptionIndex -= 1;
	}

	//if they press the down key, navigate one down in the selectable list
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_S) || theInput->WasKeyJustPressed(theInput->KEYBOARD_DOWN_ARROW))
	{
		if (m_selectedOptionIndex < (int)m_selectableSimulationDefinitions.size())
			m_selectedOptionIndex += 1;
		else
		{
			m_selectedOptionIndex = 0;
			m_selectedBox = RELOAD_OPTION_BOX;
			return;
		}			
	}

	//if they press the right key, put them in the selected box
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_D) || theInput->WasKeyJustPressed(theInput->KEYBOARD_RIGHT_ARROW))
	{
		if (m_selectedOptionIndex < (int)m_selectableSimulationDefinitions.size())
			m_selectedOptionIndex += 1;
		else
		{
			if (m_selectedOptionIndex >= (int)m_selectedSimulationDefinitions.size())
				m_selectedOptionIndex = (int)m_selectedSimulationDefinitions.size() - 1;

			m_selectedBox = SELECTED_SIMS_BOX;
			return;
		}
	}

	//if they press the enter key, add the option to the selected list
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_ENTER))
	{
		AddSimulationToSelectedList(m_selectedOptionIndex);
	}
}

//  =========================================================================================
void SimSelectState::UpdateInputSelectedBox()
{
	//if they press the up key, navigate one up in the selected list
	InputSystem* theInput = InputSystem::GetInstance();
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_W) || theInput->WasKeyJustPressed(theInput->KEYBOARD_UP_ARROW))
	{
		if (m_selectedOptionIndex > 0)
			m_selectedOptionIndex -= 1;
	}

	//if they press the down key, navigate one down in the selected list
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_S) || theInput->WasKeyJustPressed(theInput->KEYBOARD_DOWN_ARROW))
	{
		if (m_selectedOptionIndex < (int)m_selectedSimulationDefinitions.size())
			m_selectedOptionIndex += 1;
		else
		{
			m_selectedOptionIndex = 0;
			m_selectedBox = EXECUTE_OPTION_BOX;
			return;
		}
	}

	//if they press the left key, put them in the selectable box
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_D) || theInput->WasKeyJustPressed(theInput->KEYBOARD_RIGHT_ARROW))
	{
		if (m_selectedOptionIndex < (int)m_selectedSimulationDefinitions.size())
			m_selectedOptionIndex += 1;
		else
		{
			if (m_selectedOptionIndex >= (int)m_selectableSimulationDefinitions.size())
				m_selectedOptionIndex = (int)m_selectableSimulationDefinitions.size() - 1;

			m_selectedBox = SELECTABLE_SIMS_BOX;
			return;
		}
	}

	//if they press the enter key, remove the selected option
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_ENTER))
	{
		RemoveSimulationFromSelectedList(m_selectedOptionIndex);
	}
}

//  =========================================================================================
void SimSelectState::UpdateInputExecuteBox()
{
	InputSystem* theInput = InputSystem::GetInstance();

	//if they press the up key, put them at the bottom of the selected list
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_W) || theInput->WasKeyJustPressed(theInput->KEYBOARD_UP_ARROW))
	{
		m_selectedOptionIndex = (int)m_selectedSimulationDefinitions.size() - 1;
		m_selectedBox = SELECTED_SIMS_BOX;
	}

	//if they press the left key, put them in the clear option box box
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_A) || theInput->WasKeyJustPressed(theInput->KEYBOARD_LEFT_ARROW))
	{	
		m_selectedBox = CLEAR_OPTION_BOX;
		return;
	}

	//if they press the enter key, excute the current list we have
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_ENTER))
	{
		if (m_selectedSimulationDefinitions.size() > 0)
		{
			//transition to playing with the current list
		}
		//else show popup or something telling them they must have some in the list		
	}
}

//  =========================================================================================
void SimSelectState::UpdateInputClearBox()
{
	InputSystem* theInput = InputSystem::GetInstance();

	//if they press the up key, put them at the bottom of the selected list
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_W) || theInput->WasKeyJustPressed(theInput->KEYBOARD_UP_ARROW))
	{
		m_selectedOptionIndex = (int)m_selectedSimulationDefinitions.size() - 1;
		m_selectedBox = SELECTED_SIMS_BOX;
	}

	//if they press the left key, put them in the reload option box box
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_A) || theInput->WasKeyJustPressed(theInput->KEYBOARD_LEFT_ARROW))
	{
		m_selectedBox = RELOAD_OPTION_BOX;
		return;
	}

	//if they press the left key, put them in the execute option box box
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_D) || theInput->WasKeyJustPressed(theInput->KEYBOARD_RIGHT_ARROW))
	{
		m_selectedBox = RELOAD_OPTION_BOX;
		return;
	}

	//if they press the enter key, excute the current list we have
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_ENTER))
	{
		RemoveAllFromList();
	}
}

//  =========================================================================================
void SimSelectState::UpdateInputReloadBox()
{

	InputSystem* theInput = InputSystem::GetInstance();

	//if they press the up key, put them at the bottom of the selected list
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_W) || theInput->WasKeyJustPressed(theInput->KEYBOARD_UP_ARROW))
	{
		m_selectedOptionIndex = (int)m_selectableSimulationDefinitions.size() - 1;
		m_selectedBox = SELECTABLE_SIMS_BOX;
	}

	//if they press the left key, put them in the reload option box box
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_A) || theInput->WasKeyJustPressed(theInput->KEYBOARD_LEFT_ARROW))
	{
		m_selectedBox = RELOAD_OPTION_BOX;
		return;
	}

	//if they press the left key, put them in the execute option box box
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_D) || theInput->WasKeyJustPressed(theInput->KEYBOARD_RIGHT_ARROW))
	{
		m_selectedBox = CLEAR_OPTION_BOX;
		return;
	}

	//if they press the enter key, excute the current list we have
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_ENTER))
	{
		ReloadSimulationDefinitions();
	}
}

//  =========================================================================================
void SimSelectState::RenderInstructions()
{
	Renderer* theRenderer = Renderer::GetInstance();
	Window* theWindow = Window::GetInstance();

	theRenderer->DrawText2DCentered(Vector2(theWindow->m_clientWidth * 0.5f, theWindow->m_clientHeight * 0.015f),
		"Press 'ESCAPE' to return to Main Menu",
		theWindow->m_clientHeight * 0.01f,
		Rgba::YELLOW,
		1.f,
		Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));
}

//  =========================================================================================
void SimSelectState::RenderSelectableBox()
{
	Renderer* theRenderer = Renderer::GetInstance();
	Window* theWindow = Window::GetInstance();

	static Rgba hoveredColor = Rgba::WHITE;
	static Rgba simHeaderColor = Rgba::YELLOW;
	static Rgba nonHoveredColor = Rgba::GRAY;

	static AABB2 selectableContentsBox = AABB2(Vector2(theWindow->m_clientWidth * 0.025f, theWindow->m_clientHeight * 0.15f),
		Vector2(theWindow->m_clientWidth * 0.45f, theWindow->m_clientHeight * 0.90f));

	// reset texture
		theRenderer->SetTexture(*m_backGroundTexture);
	theRenderer->SetShader(theRenderer->m_defaultShader);
	theRenderer->m_defaultShader->EnableColorBlending(BLEND_OP_ADD, BLEND_SOURCE_ALPHA, BLEND_ONE_MINUS_SOURCE_ALPHA);

	//draw options
	theRenderer->DrawAABB(selectableContentsBox, Rgba(0.2f, 0.2f, 0.2f, 1.f));

	//draw each type of reset
	Vector2 optionsBoxStart = selectableContentsBox.GetTopLeftPosition();
	Vector2 optionsBoxDimensions = selectableContentsBox.GetDimensions();

	int optionDisplacementCount = 0;
	int simCount = 0;

	float startHeight = 0.9f;
	float heightDecrementAmount = 0.02f;

	for (int selectableIndex = 0; selectableIndex < (int)m_selectableSimulationDefinitions.size(); ++selectableIndex)
	{
		Rgba textColor = nonHoveredColor;
		if (m_selectedBox == SELECTABLE_SIMS_BOX && m_selectedOptionIndex == selectableIndex)
			textColor = hoveredColor;

		++simCount;

		std::string simName = m_selectableSimulationDefinitions[selectableIndex].m_definition->m_name;

		//draw simulation paths
		Vector2 textPosition = Vector2(optionsBoxStart.x + (optionsBoxDimensions.x * 0.05f), ((optionsBoxStart.y * 0.95) - (optionsBoxDimensions.y * float(optionDisplacementCount) * heightDecrementAmount)));
		theRenderer->DrawText2D(textPosition,
			Stringf("%i) %s", simCount, simName.c_str()).c_str(),
			theWindow->m_clientHeight * 0.015f,
			textColor,
			1.f,
			Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));

		++optionDisplacementCount;
	}
}

//  =========================================================================================
void SimSelectState::RenderSelectedBox()
{
	Renderer* theRenderer = Renderer::GetInstance();
	Window* theWindow = Window::GetInstance();

	static Rgba hoveredColor = Rgba::WHITE;
	static Rgba simHeaderColor = Rgba::YELLOW;
	static Rgba nonHoveredColor = Rgba::GRAY;

	static AABB2 selectedContentsBox = AABB2(Vector2(theWindow->m_clientWidth * 0.525, theWindow->m_clientHeight * 0.15f),
		Vector2(theWindow->m_clientWidth * 0.95, theWindow->m_clientHeight * 0.90f));

	// reset texture
	theRenderer->SetTexture(*m_backGroundTexture);
	theRenderer->SetShader(theRenderer->m_defaultShader);
	theRenderer->m_defaultShader->EnableColorBlending(BLEND_OP_ADD, BLEND_SOURCE_ALPHA, BLEND_ONE_MINUS_SOURCE_ALPHA);

	//draw options
	theRenderer->DrawAABB(selectedContentsBox, Rgba(0.2f, 0.2f, 0.2f, 1.f));

	//draw each type of reset
	Vector2 optionsBoxStart = selectedContentsBox.GetTopLeftPosition();
	Vector2 optionsBoxDimensions = selectedContentsBox.GetDimensions();

	int optionDisplacementCount = 0;
	int simCount = 0;

	float startHeight = 0.9f;
	float heightDecrementAmount = 0.02f;

	for (int selectableIndex = 0; selectableIndex < (int)m_selectedSimulationDefinitions.size(); ++selectableIndex)
	{
		Rgba textColor = nonHoveredColor;
		if (m_selectedBox == SELECTED_SIMS_BOX && m_selectedOptionIndex == selectableIndex)
			textColor = hoveredColor;

		++simCount;

		std::string simName = m_selectedSimulationDefinitions[selectableIndex].m_definition->m_name;

		//draw simulation paths
		Vector2 textPosition = Vector2(optionsBoxStart.x + (optionsBoxDimensions.x * 0.05f), ((optionsBoxStart.y * 0.95) - (optionsBoxDimensions.y * float(optionDisplacementCount) * heightDecrementAmount)));
		theRenderer->DrawText2D(textPosition,
			Stringf("%i) %s", simCount, simName.c_str()).c_str(),
			theWindow->m_clientHeight * 0.015f,
			textColor,
			1.f,
			Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));

		++optionDisplacementCount;
	}
}


//  =========================================================================================
void SimSelectState::RenderReloadBox()
{
	Renderer* theRenderer = Renderer::GetInstance();
	Window* theWindow = Window::GetInstance();

	static Rgba hoveredColor = Rgba::WHITE;
	static Rgba simHeaderColor = Rgba::YELLOW;
	static Rgba nonHoveredColor = Rgba::GRAY;

	static AABB2 reloadBox = AABB2(Vector2(theWindow->m_clientWidth * 0.25f, theWindow->m_clientHeight * 0.075f),
		theWindow->m_clientWidth * 0.05f, theWindow->m_clientHeight * 0.05f);

	// reset texture
	theRenderer->SetTexture(*m_backGroundTexture);
	theRenderer->SetShader(theRenderer->m_defaultShader);
	theRenderer->m_defaultShader->EnableColorBlending(BLEND_OP_ADD, BLEND_SOURCE_ALPHA, BLEND_ONE_MINUS_SOURCE_ALPHA);

	//draw options
	theRenderer->DrawAABB(reloadBox, Rgba(0.2f, 0.2f, 0.2f, 1.f));
}

//  =========================================================================================
void SimSelectState::RenderClearBox()
{
	Renderer* theRenderer = Renderer::GetInstance();
	Window* theWindow = Window::GetInstance();

	static Rgba hoveredColor = Rgba::WHITE;
	static Rgba simHeaderColor = Rgba::YELLOW;
	static Rgba nonHoveredColor = Rgba::GRAY;

	static AABB2 clearBox = AABB2(Vector2(theWindow->m_clientWidth * 0.5f, theWindow->m_clientHeight * 0.075f),
		theWindow->m_clientWidth * 0.05f, theWindow->m_clientHeight * 0.05f);

	// reset texture
	theRenderer->SetTexture(*m_backGroundTexture);
	theRenderer->SetShader(theRenderer->m_defaultShader);
	theRenderer->m_defaultShader->EnableColorBlending(BLEND_OP_ADD, BLEND_SOURCE_ALPHA, BLEND_ONE_MINUS_SOURCE_ALPHA);

	//draw options
	theRenderer->DrawAABB(clearBox, Rgba(0.2f, 0.2f, 0.2f, 1.f));
}

//  =========================================================================================
void SimSelectState::RenderExecuteBox()
{
	Renderer* theRenderer = Renderer::GetInstance();
	Window* theWindow = Window::GetInstance();

	static Rgba hoveredColor = Rgba::WHITE;
	static Rgba simHeaderColor = Rgba::YELLOW;
	static Rgba nonHoveredColor = Rgba::GRAY;

	static AABB2 executeBox = AABB2(Vector2(theWindow->m_clientWidth * 0.75f, theWindow->m_clientHeight * 0.075f),
		theWindow->m_clientWidth * 0.05f, theWindow->m_clientHeight * 0.05f);

	// reset texture
	theRenderer->SetTexture(*m_backGroundTexture);
	theRenderer->SetShader(theRenderer->m_defaultShader);
	theRenderer->m_defaultShader->EnableColorBlending(BLEND_OP_ADD, BLEND_SOURCE_ALPHA, BLEND_ONE_MINUS_SOURCE_ALPHA);

	//draw options
	theRenderer->DrawAABB(executeBox, Rgba(0.2f, 0.2f, 0.2f, 1.f));
}

//  =========================================================================================
void SimSelectState::ReloadSimulationDefinitions()
{
	Game* theGame = Game::GetInstance();

	//reload the game selected list
	SimulationDefinition::s_simulationDefinitions.clear();
	theGame->InitializeSimulationDefinitions();

	RemoveAllFromList();
	Initialize();	
}

//  =========================================================================================
void SimSelectState::AddSimulationToSelectedList(int id)
{
	SimulationUIOption optionCopy = m_selectableSimulationDefinitions[id];
	optionCopy.m_id = (int)m_selectedSimulationDefinitions.size();

	m_selectedSimulationDefinitions.push_back(optionCopy);
}

//  =========================================================================================
void SimSelectState::RemoveSimulationFromSelectedList(int id)
{
	m_selectedSimulationDefinitions.erase(m_selectedSimulationDefinitions.begin() + id);
	m_selectedOptionIndex--;
}

//  =========================================================================================
void SimSelectState::Execute()
{

}

//  =========================================================================================
void SimSelectState::RemoveAllFromList()
{
	m_selectedSimulationDefinitions.clear();
	m_selectedOptionIndex = 0;
}

//  =========================================================================================
void SimSelectState::ReturnToMainMenu()
{
	GameState::TransitionGameStates(GetGameStateFromGlobalListByType(ANALYSIS_SELECT_GAME_STATE));
}
