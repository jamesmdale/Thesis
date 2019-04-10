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
		SimulationUIOption option = SimulationUIOption(definitionIndex, SimulationDefinition::s_simulationDefinitions[definitionIndex]);
		m_selectableSimulationDefinitions.push_back(option);

		m_executionSimulationDefinitions.push_back(option);
	}

	m_isInitialized = true;
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
		if (m_selectedOptionIndex < (int)m_selectableSimulationDefinitions.size() - 1)
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
		m_selectedOptionIndex = 0;
		m_selectedBox = SELECTED_SIMS_BOX;
		return;
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
		if (m_selectedOptionIndex < (int)m_executionSimulationDefinitions.size() - 1)
			m_selectedOptionIndex += 1;
		else
		{
			m_selectedOptionIndex = 0;
			m_selectedBox = EXECUTE_OPTION_BOX;
			return;
		}
	}

	//if they press the left key, put them in the selectable box
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_A) || theInput->WasKeyJustPressed(theInput->KEYBOARD_LEFT_ARROW))
	{
		m_selectedOptionIndex = 0;
		m_selectedBox = SELECTABLE_SIMS_BOX;
		return;
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
		m_selectedOptionIndex = (int)m_executionSimulationDefinitions.size() - 1;
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
		if (m_executionSimulationDefinitions.size() > 0)
		{
			//else show popup or something telling them they must have some in the list		
			Execute();
		}		
	}
}

//  =========================================================================================
void SimSelectState::UpdateInputClearBox()
{
	InputSystem* theInput = InputSystem::GetInstance();

	//if they press the up key, put them at the bottom of the selected list
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_W) || theInput->WasKeyJustPressed(theInput->KEYBOARD_UP_ARROW))
	{
		m_selectedOptionIndex = (int)m_executionSimulationDefinitions.size() - 1;
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
		m_selectedBox = EXECUTE_OPTION_BOX;
		return;
	}

	//if they press the enter key, excute the current list we have
	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_ENTER))
	{
		RemoveAllFromSelectedList();
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

	/*static AABB2 selectableContentsBox = AABB2(Vector2(theWindow->m_clientWidth * 0.025f, theWindow->m_clientHeight * 0.15f),
		Vector2(theWindow->m_clientWidth * 0.45f, theWindow->m_clientHeight * 0.90f));*/

	AABB2 selectableContentsBox;
	Vector2 selectableContentsBoxCenter = Vector2(theWindow->m_clientWidth * 0.25, theWindow->m_clientHeight * 0.55f);
	selectableContentsBox.SetCenter(selectableContentsBoxCenter);
	selectableContentsBox.SetDimensions(theWindow->m_clientWidth * 0.2f, theWindow->m_clientHeight * 0.33f);

	// reset texture
	theRenderer->SetTexture(*m_backGroundTexture);
	theRenderer->SetShader(theRenderer->m_defaultShader);
	theRenderer->m_defaultShader->EnableColorBlending(BLEND_OP_ADD, BLEND_SOURCE_ALPHA, BLEND_ONE_MINUS_SOURCE_ALPHA);

	//draw options
	if (m_selectedBox == SELECTABLE_SIMS_BOX)
		theRenderer->DrawAABB(selectableContentsBox * 0.015f, Rgba::YELLOW);

	theRenderer->DrawAABB(selectableContentsBox, Rgba(0.2f, 0.2f, 0.2f, 1.f));
	
	Vector2 selectableTitleCenter = Vector2(selectableContentsBoxCenter.x, selectableContentsBoxCenter.y + (theWindow->m_clientHeight * 0.33f) + (theWindow->m_clientHeight * 0.025f));
	theRenderer->DrawText2DCentered(selectableTitleCenter,
		Stringf("Selectable List").c_str(),
		theWindow->m_clientHeight * 0.025f,
		Rgba::YELLOW,
		1.f,
		Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));

	Vector2 selectableInstructionsCenter = Vector2(selectableContentsBoxCenter.x, selectableContentsBoxCenter.y - (theWindow->m_clientHeight * 0.33f) + (theWindow->m_clientHeight * 0.025f));
	theRenderer->DrawText2DCentered(selectableInstructionsCenter,
		Stringf("'ENTER' to ADD to the execution list").c_str(),
		theWindow->m_clientHeight * 0.01f,
		Rgba::YELLOW,
		1.f,
		Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));


	//draw each type of reset
	Vector2 optionsBoxStart = selectableContentsBox.GetTopLeftPosition();
	Vector2 optionsBoxDimensions = selectableContentsBox.GetDimensions();

	int optionDisplacementCount = 0;
	int simCount = 0;

	float startHeight = 0.9f;
	float heightDecrementAmount = 0.04;

	for (int selectableIndex = 0; selectableIndex < (int)m_selectableSimulationDefinitions.size(); ++selectableIndex)
	{
		Rgba textColor = nonHoveredColor;
		if (m_selectedBox == SELECTABLE_SIMS_BOX && m_selectedOptionIndex == selectableIndex)
			textColor = hoveredColor;

		++simCount;

		std::string simName = m_selectableSimulationDefinitions[selectableIndex].m_definition->m_name;

		//draw simulation paths
		Vector2 textPosition = Vector2(optionsBoxStart.x + (optionsBoxDimensions.x * 0.05f), ((optionsBoxStart.y * 0.95f) - (optionsBoxDimensions.y * float(optionDisplacementCount) * heightDecrementAmount)));
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

	/*static AABB2 selectedContentsBox = AABB2(Vector2(theWindow->m_clientWidth * 0.525, theWindow->m_clientHeight * 0.15f),
		Vector2(theWindow->m_clientWidth * 0.95, theWindow->m_clientHeight * 0.90f));*/

	AABB2 selectedContentsBox;
	Vector2 selectedContentsBoxCenter = Vector2(theWindow->m_clientWidth * 0.75f, theWindow->m_clientHeight * 0.55f);
	selectedContentsBox.SetCenter(selectedContentsBoxCenter);
	selectedContentsBox.SetDimensions(theWindow->m_clientWidth * 0.2f, theWindow->m_clientHeight * 0.33f);

	// reset texture
	theRenderer->SetTexture(*m_backGroundTexture);
	theRenderer->SetShader(theRenderer->m_defaultShader);
	theRenderer->m_defaultShader->EnableColorBlending(BLEND_OP_ADD, BLEND_SOURCE_ALPHA, BLEND_ONE_MINUS_SOURCE_ALPHA);

	//draw options
	if (m_selectedBox == SELECTED_SIMS_BOX)
		theRenderer->DrawAABB(selectedContentsBox * 0.015f, Rgba::YELLOW);

	theRenderer->DrawAABB(selectedContentsBox, Rgba(0.2f, 0.2f, 0.2f, 1.f));	

	//draw instrcutions
	Vector2 executionTitleCenter = Vector2(selectedContentsBoxCenter.x, selectedContentsBoxCenter.y + (theWindow->m_clientHeight * 0.33f) + (theWindow->m_clientHeight * 0.025f));
	theRenderer->DrawText2DCentered(executionTitleCenter,
		Stringf("Execution List").c_str(),
		theWindow->m_clientHeight * 0.025f,
		Rgba::YELLOW,
		1.f,
		Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));

	Vector2 executionInstructionsCenter= Vector2(selectedContentsBoxCenter.x, selectedContentsBoxCenter.y - (theWindow->m_clientHeight * 0.33f) + (theWindow->m_clientHeight * 0.025f));
	theRenderer->DrawText2DCentered(executionInstructionsCenter,
		Stringf("'ENTER' to REMOVE from execution list").c_str(),
		theWindow->m_clientHeight * 0.01f,
		Rgba::YELLOW,
		1.f,
		Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));

	//draw each type of reset
	Vector2 optionsBoxStart = selectedContentsBox.GetTopLeftPosition();
	Vector2 optionsBoxDimensions = selectedContentsBox.GetDimensions();

	int optionDisplacementCount = 0;
	int simCount = 0;

	float startHeight = 0.9f;
	float heightDecrementAmount = 0.04;

	for (int selectableIndex = 0; selectableIndex < (int)m_executionSimulationDefinitions.size(); ++selectableIndex)
	{
		Rgba textColor = nonHoveredColor;
		if (m_selectedBox == SELECTED_SIMS_BOX && m_selectedOptionIndex == selectableIndex)
			textColor = hoveredColor;

		++simCount;

		std::string simName = m_executionSimulationDefinitions[selectableIndex].m_definition->m_name;

		//draw simulation paths
		Vector2 textPosition = Vector2(optionsBoxStart.x + (optionsBoxDimensions.x * 0.05f), ((optionsBoxStart.y * 0.95f) - (optionsBoxDimensions.y * float(optionDisplacementCount) * heightDecrementAmount)));
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

	AABB2 reloadBox;
	reloadBox.SetCenter(Vector2(theWindow->m_clientWidth * 0.25f, theWindow->m_clientHeight * 0.125f));
	reloadBox.SetDimensions(theWindow->m_clientWidth * 0.1f, theWindow->m_clientHeight * 0.05f);

	Rgba textColor = nonHoveredColor;
	if (m_selectedBox == RELOAD_OPTION_BOX)
		textColor = hoveredColor;

	// reset texture
	theRenderer->SetTexture(*m_backGroundTexture);
	theRenderer->SetShader(theRenderer->m_defaultShader);
	theRenderer->m_defaultShader->EnableColorBlending(BLEND_OP_ADD, BLEND_SOURCE_ALPHA, BLEND_ONE_MINUS_SOURCE_ALPHA);

	//draw options
	if (m_selectedBox == RELOAD_OPTION_BOX)
		theRenderer->DrawAABB(reloadBox * 0.015, Rgba::YELLOW);

	theRenderer->DrawAABB(reloadBox, Rgba(0.2f, 0.2f, 0.2f, 1.f));	

	theRenderer->DrawText2DCentered(reloadBox.GetCenter(),
		Stringf("Reload Defs").c_str(),
		theWindow->m_clientHeight * 0.025f,
		textColor,
		1.f,
		Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));
}

//  =========================================================================================
void SimSelectState::RenderClearBox()
{
	Renderer* theRenderer = Renderer::GetInstance();
	Window* theWindow = Window::GetInstance();

	static Rgba hoveredColor = Rgba::WHITE;
	static Rgba simHeaderColor = Rgba::YELLOW;
	static Rgba nonHoveredColor = Rgba::GRAY;

	Rgba textColor = nonHoveredColor;
	if (m_selectedBox == CLEAR_OPTION_BOX)
		textColor = hoveredColor;

	/*static AABB2 clearBox = AABB2(Vector2(theWindow->m_clientWidth * 0.5f, theWindow->m_clientHeight * 0.075f),
		theWindow->m_clientWidth * 0.05f, theWindow->m_clientHeight * 0.05f);*/

	AABB2 clearBox;
	clearBox.SetCenter(Vector2(theWindow->m_clientWidth * 0.5f, theWindow->m_clientHeight * 0.125f));
	clearBox.SetDimensions(theWindow->m_clientWidth * 0.1f, theWindow->m_clientHeight * 0.05f);

	// reset texture
	theRenderer->SetTexture(*m_backGroundTexture);
	theRenderer->SetShader(theRenderer->m_defaultShader);
	theRenderer->m_defaultShader->EnableColorBlending(BLEND_OP_ADD, BLEND_SOURCE_ALPHA, BLEND_ONE_MINUS_SOURCE_ALPHA);

	//draw options
	if (m_selectedBox == CLEAR_OPTION_BOX)
		theRenderer->DrawAABB(clearBox * 0.015, Rgba::YELLOW);

	theRenderer->DrawAABB(clearBox, Rgba(0.2f, 0.2f, 0.2f, 1.f));

	theRenderer->DrawText2DCentered(clearBox.GetCenter(),
		Stringf("Clear Selected").c_str(),
		theWindow->m_clientHeight * 0.025f,
		textColor,
		1.f,
		Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));
}

//  =========================================================================================
void SimSelectState::RenderExecuteBox()
{
	Renderer* theRenderer = Renderer::GetInstance();
	Window* theWindow = Window::GetInstance();

	static Rgba hoveredColor = Rgba::WHITE;
	static Rgba simHeaderColor = Rgba::YELLOW;
	static Rgba nonHoveredColor = Rgba::GRAY;

	Rgba textColor = nonHoveredColor;
	if (m_selectedBox == EXECUTE_OPTION_BOX)
		textColor = hoveredColor;

	AABB2 executeBox;
	executeBox.SetCenter(Vector2(theWindow->m_clientWidth * 0.75f, theWindow->m_clientHeight * 0.125f));
	executeBox.SetDimensions(theWindow->m_clientWidth * 0.1f, theWindow->m_clientHeight * 0.05f);

	// reset texture
	theRenderer->SetTexture(*m_backGroundTexture);
	theRenderer->SetShader(theRenderer->m_defaultShader);
	theRenderer->m_defaultShader->EnableColorBlending(BLEND_OP_ADD, BLEND_SOURCE_ALPHA, BLEND_ONE_MINUS_SOURCE_ALPHA);

	//draw options
	if (m_selectedBox == EXECUTE_OPTION_BOX)
		theRenderer->DrawAABB(executeBox * 0.015, Rgba::YELLOW);

	theRenderer->DrawAABB(executeBox, Rgba(0.2f, 0.2f, 0.2f, 1.f));

	theRenderer->DrawText2DCentered(executeBox.GetCenter(),
		Stringf("Execute").c_str(),
		theWindow->m_clientHeight * 0.025f,
		textColor,
		1.f,
		Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));
}

//  =========================================================================================
void SimSelectState::ReloadSimulationDefinitions()
{
	Game* theGame = Game::GetInstance();

	//reload the game selected list
	SimulationDefinition::s_simulationDefinitions.clear();
	theGame->InitializeSimulationDefinitions();

	m_selectableSimulationDefinitions.clear();
	
	RemoveAllFromSelectedList();
	Initialize();	
}

//  =========================================================================================
void SimSelectState::AddSimulationToSelectedList(int id)
{
	if (m_executionSimulationDefinitions.size() >= 8)
		return;

	SimulationUIOption optionCopy = m_selectableSimulationDefinitions[id];
	optionCopy.m_id = (int)m_executionSimulationDefinitions.size();

	m_executionSimulationDefinitions.push_back(optionCopy);
}

//  =========================================================================================
void SimSelectState::RemoveSimulationFromSelectedList(int id)
{
	if (m_executionSimulationDefinitions.size() == 0)
		return;

	m_executionSimulationDefinitions.erase(m_executionSimulationDefinitions.begin() + id);
	m_selectedOptionIndex--;

	if (m_selectedOptionIndex < 0)
		m_selectedOptionIndex = 0;

	m_selectedOptionIndex % m_executionSimulationDefinitions.size();
}

//  =========================================================================================
void SimSelectState::Execute()
{
	//copy selected list into our game list
	Game* theGame = Game::GetInstance();
	theGame->m_selectedDefinitions.clear();

	for (int definitionIndex = 0; definitionIndex < (int)m_executionSimulationDefinitions.size(); ++definitionIndex)
	{
		theGame->m_selectedDefinitions.push_back(m_executionSimulationDefinitions[definitionIndex].m_definition);
	}

	GameState::TransitionGameStates(GetGameStateFromGlobalListByType(PLAYING_GAME_STATE));
}

//  =========================================================================================
void SimSelectState::RemoveAllFromSelectedList()
{
	m_executionSimulationDefinitions.clear();
	m_selectedOptionIndex = 0;
}

//  =========================================================================================
void SimSelectState::ReturnToMainMenu()
{
	GameState::TransitionGameStates(GetGameStateFromGlobalListByType(MAIN_MENU_GAME_STATE));
}
