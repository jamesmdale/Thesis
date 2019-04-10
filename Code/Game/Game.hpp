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
#include "Engine\Math\RNG.hpp"
#include <vector>

class Game
{
public:
	Game();
	~Game();
	static Game* GetInstance();
	static Game* CreateInstance();
	static RNG* GetGlobalRNG();

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

public:  

	//camera members
	Camera* m_gameCamera = nullptr;
	Clock* m_gameClock = nullptr;

	//rendering members
	ForwardRenderingPath2D* m_forwardRenderingPath2D = nullptr;
	std::vector<SimulationDefinition*> m_selectedDefinitions;
	int m_currentSimDefinitionIndex = 0;
};

Clock* GetGameClock();




