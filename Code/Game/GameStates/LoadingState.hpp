#pragma once
#include "Game\GameStates\GameState.hpp"

class LoadingState : public GameState
{
public:
	LoadingState(Camera* camera) : GameState(camera)
	{
		m_type = LOADING_GAME_STATE;
		m_backGroundTexture = Renderer::GetInstance()->CreateOrGetTexture("default");
	}

	virtual ~LoadingState() override;

	virtual void Update(float deltaSeconds) override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;
	virtual float UpdateFromInput(float deltaSeconds) override;

public:
	Texture* m_backGroundTexture;
};

