#pragma once
#include "Game\GameCommon.hpp"
#include "Engine\Time\Clock.hpp"
#include "Engine\Camera\Camera.hpp"
#include "Engine\Camera\OrbitCamera.hpp"
#include "Engine\Core\GameObject.hpp"
#include "Engine\Renderer\RenderScene.hpp"
#include "Engine\Renderer\ForwardRenderingPath.hpp"
#include "Engine\ParticleSystem\ParticleEmitter.hpp"
#include "Engine\Renderer\ForwardRenderingPath2D.hpp"
#include <vector>

class Game
{
public:  

	//camera members
	Camera* m_gameCamera = nullptr;
	Clock* m_gameClock = nullptr;

	//rendering members
	ForwardRenderingPath2D* m_forwardRenderingPath2D = nullptr;

public:
	Game();
	~Game();
	static Game* GetInstance();
	static Game* CreateInstance();

	void Update(); //use internal clock for delta seconds;
	void PreRender();
	void Render();
	void PostRender();
	void Initialize();
	float UpdateInput(float deltaSeconds);

	void InitializeTileDefinitions();
	void InitializeMapDefinitions();
	void InitializeAgentDefinitions();
	void InitializeSimulationDefinitions();

};

Clock* GetGameClock();




