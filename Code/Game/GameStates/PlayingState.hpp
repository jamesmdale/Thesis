#pragma once
#include "Game\GameStates\GameState.hpp"
#include "Game\Map\Map.hpp"
#include "Engine\Core\Command.hpp"
#include "Engine\Time\Stopwatch.hpp"
#include "Engine\Core\Widget.hpp"
#include "Engine\Utility\Grid.hpp"

class SimulationData;
class SimulationDefinition;
class Mesh;

class PlayingState : public GameState
{
public:
	PlayingState(Camera* camera) : GameState(camera)
	{
		m_type = PLAYING_GAME_STATE;
	}

	virtual ~PlayingState() override;
	
	virtual void Initialize() override;
	virtual void Update(float deltaSeconds) override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;
	virtual float UpdateFromInput(float deltaSeconds) override;

	void UpdateFPSCounters();

	void RenderGame();
	void RenderDebugUI();

	//simulations
	void InitializeSimulation(SimulationDefinition* definition);
	void CreateMapForSimulation(SimulationDefinition * definition);
	void ResetMapForSimulation(SimulationDefinition* definition);
	void InitializeSimulationData();
	void ResetCurrentSimulationData();
	void DeleteMap();
	void ExportSimulationData();

	void GenerateOutputDirectory();
	void FinalizeGeneralSimulationData();

	//render helpers	
	Mesh* CreateUIDebugTextMesh();
	Mesh* CreateWorldDebugTextMesh();
	Mesh* CreateDisectedAgentPathMesh();	

	void ClearDisectedAgent();
	
public:
	Camera* m_uiCamera = nullptr;
	Map* m_map = nullptr;

	Stopwatch* m_simulationTimer = nullptr;

	bool m_isCameraLockedToAgent = false;
	Agent* m_disectedAgent = nullptr;

	Stopwatch* m_inputDelayTimer = nullptr;
};

void TogglePaused(Command& cmd);
void DisectAgent(Command& cmd);
void ToggleBlockedData(Command& cmd);
void ToggleAgentIds(Command& cmd);
