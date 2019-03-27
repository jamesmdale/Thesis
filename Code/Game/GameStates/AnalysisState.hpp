#pragma once
#include "Game\GameStates\GameState.hpp"
#include "Engine\Core\Rgba.hpp"

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

	virtual void TransitionIn(float secondsTransitioning) override;
	virtual void TransitionOut(float secondsTransitioning) override;

	virtual void ResetState() override;

public:
	Texture * m_backGroundTexture;

	std::string m_simulationDataFilePath = "";
	//int m_selectedSimulationPathIndex = 0;
	//std::vector<std::string> m_simulationPaths;
};