#pragma once
#include "Game\GameStates\GameState.hpp"

enum eAnalysisStateOptions
{
	PLAY,
	EXIT,
	NUM_MAIN_MENU_OPTIONS
};

class AnalysisState : public GameState
{
public:
	AnalysisState(Camera* camera) : GameState(camera)
	{
		m_type = ANALYSIS_STATE;
		m_backGroundTexture = Renderer::GetInstance()->CreateOrGetTexture("default");
	}

	virtual ~AnalysisState() override;

	virtual void Update(float deltaSeconds) override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;
	virtual float UpdateFromInput(float deltaSeconds) override;

	virtual void ResetState() override;

public:
	Texture* m_backGroundTexture;
	eAnalysisStateOptions m_selectedAnalysisStateOption = PLAY;
};

