#pragma once
#include "Game\GameStates\GameState.hpp"

enum eAnalysisStateOptions
{
	EXECUTE_SELECTED_ANALYSIS_STATE,
	EXIT_ANALYSIS_STATE,
	ANALYSIS_STATE_OPTIONS
};

class AnalysisState : public GameState
{
public:
	AnalysisState(Camera* camera) : GameState(camera)
	{
		m_type = ANALYSIS_GAME_STATE;
		m_backGroundTexture = Renderer::GetInstance()->CreateOrGetTexture("default");
	}

	virtual ~AnalysisState() override;

	virtual void Update(float deltaSeconds) override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;
	virtual float UpdateFromInput(float deltaSeconds) override;

	virtual void Initialize();
	virtual void ResetState() override;

public:
	Texture* m_backGroundTexture;
	eAnalysisStateOptions m_selectedAnalysisStateOption = EXECUTE_SELECTED_ANALYSIS_STATE;
};

