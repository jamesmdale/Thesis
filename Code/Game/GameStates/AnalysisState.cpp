#include "Game\GameStates\AnalysisState.hpp"
#include "Engine\File\File.hpp"
#include "Engine\File\FileHelpers.hpp"
#include "Engine\Window\Window.hpp"
#include "Engine\Core\StringUtils.hpp"

//  =========================================================================================
AnalysisState::~AnalysisState()
{
	m_backGroundTexture = nullptr;
}

//  =========================================================================================
void AnalysisState::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

//  =========================================================================================
void AnalysisState::PreRender()
{
}

//  =========================================================================================
void AnalysisState::Render()
{
	Renderer* theRenderer = Renderer::GetInstance();
	Window* theWindow = Window::GetInstance();

	static Rgba selectedColor = Rgba::WHITE;
	static Rgba nonSelectedColor = Rgba::GRAY;
	static AABB2 exportedDataOptionsBox = AABB2(Vector2(theWindow->m_clientWidth * 0.025, theWindow->m_clientHeight * 0.05), Vector2(theWindow->m_clientWidth * 0.35, theWindow->m_clientHeight * 0.95));
	static AABB2 graphBox = AABB2(Vector2(theWindow->m_clientWidth * 0.4, theWindow->m_clientHeight * 0.05), Vector2(theWindow->m_clientWidth * 0.95, theWindow->m_clientHeight * 0.95));

	theRenderer->SetCamera(m_camera);

	theRenderer->ClearDepth(1.f);
	theRenderer->ClearColor(Rgba::BLACK);

	theRenderer->SetTexture(*m_backGroundTexture);
	theRenderer->SetShader(theRenderer->m_defaultShader);

	theRenderer->m_defaultShader->EnableColorBlending(BLEND_OP_ADD, BLEND_SOURCE_ALPHA, BLEND_ONE_MINUS_SOURCE_ALPHA);
	theRenderer->DrawAABB(theWindow->GetClientWindow(), Rgba(0.f, 0.f, 0.f, 1.f));

	//draw options
	theRenderer->DrawAABB(exportedDataOptionsBox, Rgba(0.2f, 0.2f, 0.2f, 1.f));

	//draw graph
	theRenderer->DrawAABB(graphBox, Rgba(0.8f, 0.8f, 0.8f, 0.8f));

	//draw simulation paths
	theRenderer->DrawText2DCentered(Vector2(theWindow->m_clientWidth * .5f, theWindow->m_clientHeight * 0.975f),
		Stringf("Analyzing Simulation: %s", m_simulationDataFilePath.c_str()),
		theWindow->m_clientHeight * 0.01f,
		Rgba::YELLOW,
		1.f,
		Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));	

	theRenderer->DrawText2DCentered(Vector2(theWindow->m_clientWidth * .5f, theWindow->m_clientHeight * 0.015f),
		"Press 'ENTER' to Select OR 'ESCAPE' to Return to Analysis Select",
		theWindow->m_clientHeight * 0.01f,
		Rgba::YELLOW,
		1.f,
		Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));

	theRenderer->m_defaultShader->DisableBlending();

	theRenderer = nullptr;
}

//  =========================================================================================
float AnalysisState::UpdateFromInput(float deltaSeconds)
{
	InputSystem* theInput = InputSystem::GetInstance();

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_W) || theInput->WasKeyJustPressed(theInput->KEYBOARD_UP_ARROW))
	{
		//m_selectedSimulationPathIndex = (m_selectedSimulationPathIndex + 1) % m_simulationPaths.size();
	}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_S) || theInput->WasKeyJustPressed(theInput->KEYBOARD_DOWN_ARROW))
	{
		//m_selectedSimulationPathIndex = (m_selectedSimulationPathIndex - 1) % m_simulationPaths.size();
	}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_ENTER))
	{

	}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_ESCAPE))
	{
		ResetState();
		GameState::TransitionGameStates(GetGameStateFromGlobalListByType(ANALYSIS_SELECT_GAME_STATE));
	}

	return deltaSeconds; //new deltaSeconds
}

//  =========================================================================================
void AnalysisState::Initialize()
{

}

//  =========================================================================================
void AnalysisState::TransitionIn(float secondsTransitioning)
{
	//detect all simulations
	//ReadSubFolderNamesForPath("Data/ExportedSimulationData", m_simulationPaths);
	ResetState();
	s_isFinishedTransitioningIn = true;
}

//  =========================================================================================
void AnalysisState::TransitionOut(float secondsTransitioning)
{
	ResetState();
	s_isFinishedTransitioningOut = true;
}

//  =========================================================================================
void AnalysisState::ResetState()
{
	//m_simulationPaths.clear();
}


//  =========================================================================================
void AnalysisState::PostRender()
{

}

