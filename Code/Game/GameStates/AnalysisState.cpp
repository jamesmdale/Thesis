#include "Game\GameStates\AnalysisState.hpp"
#include "Engine\Window\Window.hpp"

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

	Rgba playColor = Rgba::GRAY;
	Rgba quitColor = Rgba::GRAY;

	switch (m_selectedAnalysisStateOption)
	{
	case PLAY:
		playColor = Rgba::WHITE;
		break;
	case EXIT:
		quitColor = Rgba::WHITE;
		break;
	}

	theRenderer->SetCamera(m_camera);

	theRenderer->ClearDepth(1.f);
	theRenderer->ClearColor(Rgba::BLACK);

	theRenderer->SetTexture(*m_backGroundTexture);
	theRenderer->SetShader(theRenderer->m_defaultShader);

	theRenderer->m_defaultShader->EnableColorBlending(BLEND_OP_ADD, BLEND_SOURCE_ALPHA, BLEND_ONE_MINUS_SOURCE_ALPHA);

	theRenderer->DrawAABB(theWindow->GetClientWindow(), Rgba(0.f, 0.f, 0.f, 1.f));
	theRenderer->DrawText2DCentered(Vector2(theWindow->m_clientWidth * .5f, theWindow->m_clientHeight * .66666f), "Thesis", theWindow->m_clientHeight * .1f, Rgba::WHITE, 1.f, Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));
	theRenderer->DrawText2DCentered(Vector2(theWindow->m_clientWidth * .5f, theWindow->m_clientHeight * .35f), "Play", theWindow->m_clientHeight * .075f, playColor, 1.f, Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));
	theRenderer->DrawText2DCentered(Vector2(theWindow->m_clientWidth * .5f, theWindow->m_clientHeight * .25f), "Quit", theWindow->m_clientHeight * .075f, quitColor, 1.f, Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));

	theRenderer->m_defaultShader->DisableBlending();

	theRenderer = nullptr;
}

//  =========================================================================================
float AnalysisState::UpdateFromInput(float deltaSeconds)
{
	InputSystem* theInput = InputSystem::GetInstance();

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_W) || theInput->WasKeyJustPressed(theInput->KEYBOARD_UP_ARROW))
	{
		if (m_selectedAnalysisStateOption == PLAY)
		{
			m_selectedAnalysisStateOption = EXIT;
		}
		else
		{
			m_selectedAnalysisStateOption = PLAY;
		}
	}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_S) || theInput->WasKeyJustPressed(theInput->KEYBOARD_DOWN_ARROW))
	{
		if (m_selectedAnalysisStateOption == PLAY)
		{
			m_selectedAnalysisStateOption = EXIT;
		}
		else
		{
			m_selectedAnalysisStateOption = PLAY;
		}
	}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_SPACE))
	{
		switch (m_selectedAnalysisStateOption)
		{
		case(PLAY):
			ResetState();
			GameState::TransitionGameStates(GetGameStateFromGlobalListByType(PLAYING_GAME_STATE));
			break;
		case(EXIT):
			g_isQuitting = true;
			break;
		}
	}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_ESCAPE))
	{
		g_isQuitting = true;
	}

	theInput = nullptr;
	delete(theInput);
	return deltaSeconds; //new deltaSeconds
}

//  =========================================================================================
void AnalysisState::ResetState()
{
	m_selectedAnalysisStateOption = PLAY;
}

//  =========================================================================================
void AnalysisState::PostRender()
{

}

