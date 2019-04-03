#include "Game\GameStates\AnalysisSelectState.hpp"
#include "Game\GameStates\AnalysisState.hpp"
#include "Engine\File\File.hpp"
#include "Engine\File\FileHelpers.hpp"
#include "Engine\Window\Window.hpp"
#include "Engine\Core\StringUtils.hpp"

//  =========================================================================================
AnalysisSelectState::~AnalysisSelectState()
{
	m_backGroundTexture = nullptr;
}

//  =========================================================================================
void AnalysisSelectState::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

//  =========================================================================================
void AnalysisSelectState::PreRender()
{
}

//  =========================================================================================
void AnalysisSelectState::Render()
{

	if (g_transitionState != nullptr)
	{
		if (g_transitionState->m_type == ANALYSIS_GAME_STATE)
		{
			RenderAnalysisLoading();
		}
	}		
	else
		RenderAnalysisSelect();
}

//  =========================================================================================
float AnalysisSelectState::UpdateFromInput(float deltaSeconds)
{
	InputSystem* theInput = InputSystem::GetInstance();

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_W) || theInput->WasKeyJustPressed(theInput->KEYBOARD_UP_ARROW))
	{
		m_selectedSimulationPathIndex = (m_selectedSimulationPathIndex - 1) % m_simulationPaths.size();
	}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_S) || theInput->WasKeyJustPressed(theInput->KEYBOARD_DOWN_ARROW))
	{
		m_selectedSimulationPathIndex = (m_selectedSimulationPathIndex + 1) % m_simulationPaths.size();
	}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_ENTER))
	{
		InitializeAnalysisStateFromSelectedSim();
		GameState::TransitionGameStates(GetGameStateFromGlobalListByType(ANALYSIS_GAME_STATE));
	}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_ESCAPE))
	{
		GameState::TransitionGameStates(GetGameStateFromGlobalListByType(MAIN_MENU_GAME_STATE));
	}

	return deltaSeconds; //new deltaSeconds
}

//  =========================================================================================
void AnalysisSelectState::Initialize()
{

}

//  =========================================================================================
void AnalysisSelectState::TransitionIn(float secondsTransitioning)
{
	//detect all simulations
	ResetState();
	ReadSubFolderNamesForPath("Data/ExportedSimulationData", m_simulationPaths);
	s_isFinishedTransitioningIn = true;
}

//  =========================================================================================
void AnalysisSelectState::TransitionOut(float secondsTransitioning)
{
	ResetState();
	s_isFinishedTransitioningOut = true;
}

//  =========================================================================================
void AnalysisSelectState::ResetState()
{
	m_simulationPaths.clear();
}

//  =========================================================================================
void AnalysisSelectState::RenderAnalysisSelect()
{
	Renderer* theRenderer = Renderer::GetInstance();
	Window* theWindow = Window::GetInstance();

	static Rgba selectedColor = Rgba::WHITE;
	static Rgba nonSelectedColor = Rgba::GRAY;

	theRenderer->SetCamera(m_camera);

	theRenderer->ClearDepth(1.f);
	theRenderer->ClearColor(Rgba::BLACK);

	theRenderer->SetTexture(*m_backGroundTexture);
	theRenderer->SetShader(theRenderer->m_defaultShader);

	theRenderer->m_defaultShader->EnableColorBlending(BLEND_OP_ADD, BLEND_SOURCE_ALPHA, BLEND_ONE_MINUS_SOURCE_ALPHA);
	theRenderer->DrawAABB(theWindow->GetClientWindow(), Rgba(0.f, 0.f, 0.f, 1.f));

	//draw simulation paths
	theRenderer->DrawText2D(Vector2(theWindow->m_clientWidth * .05f, theWindow->m_clientHeight * 0.95f),
		"Select Simulation",
		theWindow->m_clientHeight * 0.025f,
		Rgba::YELLOW,
		1.f,
		Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));

	float simulationStartHeight = 0.9f;
	float simulationHeightDecrease = 0.02f;
	for (int simulationIndex = 0; simulationIndex < m_simulationPaths.size(); ++simulationIndex)
	{
		Rgba textColor;
		m_selectedSimulationPathIndex == simulationIndex ? textColor = selectedColor : textColor = nonSelectedColor;

		//draw simulation paths
		theRenderer->DrawText2D(Vector2(theWindow->m_clientWidth * .05f, theWindow->m_clientHeight * (simulationStartHeight - (float(simulationIndex) * simulationHeightDecrease))),
			Stringf("%i) %s", simulationIndex, m_simulationPaths[simulationIndex].c_str()).c_str(),
			theWindow->m_clientHeight * 0.015f,
			textColor,
			1.f,
			Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));
	}

	theRenderer->DrawText2DCentered(Vector2(theWindow->m_clientWidth * .5f, theWindow->m_clientHeight * 0.025f),
		"Press 'ENTER' to Select OR 'ESCAPE' to Return to Main",
		theWindow->m_clientHeight * 0.015f,
		Rgba::YELLOW,
		1.f,
		Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));

	theRenderer->m_defaultShader->DisableBlending();
}

//  =========================================================================================
void AnalysisSelectState::RenderAnalysisLoading()
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

	//draw simulation paths
	theRenderer->DrawText2D(Vector2(theWindow->m_clientWidth * .05f, theWindow->m_clientHeight * 0.95f),
		"Loading Simulation Data...",
		theWindow->m_clientHeight * 0.025f,
		Rgba::YELLOW,
		1.f,
		Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));

	theRenderer->m_defaultShader->DisableBlending();
}

//  =========================================================================================
void AnalysisSelectState::PostRender()
{

}

//  =========================================================================================
void AnalysisSelectState::InitializeAnalysisStateFromSelectedSim()
{
	AnalysisState* analysisState = (AnalysisState*)GameState::GetGameStateFromGlobalListByType(ANALYSIS_GAME_STATE);
	ASSERT_OR_DIE(analysisState != nullptr, "ANALYSIS GAME STATE UNINITIALIZED!!");

	analysisState->SetSimulationDataFilePath(m_simulationPaths[m_selectedSimulationPathIndex]);
}


