#pragma once
#include "Game\GameStates\GameState.hpp"

enum eReadyStateOptions
{
	READY,
	NUM_READY_STATE_OPTIONS
};

class ReadyState : public GameState
{
public:
	ReadyState(Camera* camera) : GameState(camera)
	{
		m_type = READY_UP_GAME_STATE;
		m_backGroundTexture = Renderer::GetInstance()->CreateOrGetTexture("default");
	}

	virtual ~ReadyState() override;

	virtual void Update(float deltaSeconds) override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;
	virtual float UpdateFromInput(float deltaSeconds) override;

public:
	Texture* m_backGroundTexture;
	eReadyStateOptions m_selectedMenuOption = READY;
};

