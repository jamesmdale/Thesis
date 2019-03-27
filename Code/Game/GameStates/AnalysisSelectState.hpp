#pragma once
#include "Game\GameStates\GameState.hpp"
#include "Engine\Core\Rgba.hpp"


class AnalysisSelectState : public GameState
{
public:
	AnalysisSelectState(Camera* camera) : GameState(camera)
	{
		m_type = ANALYSIS_SELECT_GAME_STATE;
		m_backGroundTexture = Renderer::GetInstance()->CreateOrGetTexture("default");
	}

	virtual ~AnalysisSelectState() override;

	virtual void Update(float deltaSeconds) override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;
	virtual float UpdateFromInput(float deltaSeconds) override;

	virtual void Initialize();

	virtual void TransitionIn(float secondsTransitioning) override;
	virtual void TransitionOut(float secondsTransitioning) override;

	virtual void ResetState() override;

	void InitializeAnalysisStateFromSelectedSim();

public:
	Texture* m_backGroundTexture;

	int m_selectedSimulationPathIndex = 0;
	std::vector<std::string> m_simulationPaths;
};