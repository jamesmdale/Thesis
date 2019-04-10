#pragma once
#include "Game\GameStates\GameState.hpp"

enum eMainMenuOptions
{
	SIM_SELECT,
	ANALYSIS,
	EXIT,
	NUM_MAIN_MENU_OPTIONS
};

class MainMenuState : public GameState
{
public:
	MainMenuState(Camera* camera) : GameState(camera)
	{
		m_type = MAIN_MENU_GAME_STATE;
		m_backGroundTexture = Renderer::GetInstance()->CreateOrGetTexture("default");
	}

	virtual ~MainMenuState() override;

	virtual void Update(float deltaSeconds) override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;
	virtual float UpdateFromInput(float deltaSeconds) override;

	virtual void TransitionIn(float secondsTransitioning);

	virtual void ResetState() override;

public:
	Texture* m_backGroundTexture;
	eMainMenuOptions m_selectedMenuOption = SIM_SELECT;
};

