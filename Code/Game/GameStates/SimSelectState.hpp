#pragma once
#include "Game\GameStates\GameState.hpp"
#include "Game\Definitions\SimulationDefinition.hpp"

enum eSelectSimStateOptions
{
	RUN_ALL_SELECT_SIM_OPTION,
	RUN_FROM_LIST_SELECT_SIM_OPTION,
	NUM_SELECT_SIM_OPTIONS
};

struct SimulationUIOptions
{
	uint id = 0;
	bool isSelected = 0;
};

class SimSelectState : public GameState
{
public:
	SimSelectState(Camera* camera) : GameState(camera)
	{
		m_type = SELECT_SIM_STATE;
		m_backGroundTexture = Renderer::GetInstance()->CreateOrGetTexture("default");
	}

	virtual ~SimSelectState() override;

	virtual void Update(float deltaSeconds) override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;
	virtual float UpdateFromInput(float deltaSeconds) override;

	virtual void ResetState() override;

public:
	Texture * m_backGroundTexture;
	eSelectSimStateOptions m_selectedSimMenuOption = RUN_ALL_SELECT_SIM_OPTION;
};

