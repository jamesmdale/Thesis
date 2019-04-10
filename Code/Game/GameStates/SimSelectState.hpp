#pragma once
#include "Game\GameStates\GameState.hpp"
#include "Game\Definitions\SimulationDefinition.hpp"


enum eCurrentSelectedInput
{
	SELECTABLE_SIMS_BOX,
	SELECTED_SIMS_BOX,
	RELOAD_OPTION_BOX,
	EXECUTE_OPTION_BOX,
	CLEAR_OPTION_BOX,
	NUM_OPTIONS
};

struct SimulationUIOption
{
	SimulationUIOption(uint id, SimulationDefinition* definition)
	{
		m_id = id;
		m_definition = definition;
		bool m_isSelected = false;
	}
	~SimulationUIOption()
	{
		m_definition = nullptr;
	}

	void ToggleSelected() { m_isSelected = !m_isSelected; }

	int m_id = 0;
	SimulationDefinition* m_definition = nullptr;
	bool m_isSelected = false;
};

class SimSelectState : public GameState
{
public:
	SimSelectState(Camera* camera) : GameState(camera)
	{
		m_type = SIM_SELECT_GAME_STATE;
		m_backGroundTexture = Renderer::GetInstance()->CreateOrGetTexture("default");
	}

	virtual ~SimSelectState() override;

	virtual void Update(float deltaSeconds) override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;
	virtual float UpdateFromInput(float deltaSeconds) override;

	virtual void Initialize();

	virtual void TransitionIn(float secondsTransitioning) override;
	virtual void TransitionOut(float secondsTransitioning) override;

	void UpdateInputSelectableBox();
	void UpdateInputSelectedBox();
	void UpdateInputExecuteBox();
	void UpdateInputClearBox();
	void UpdateInputReloadBox();

	void RenderInstructions();
	void RenderSelectableBox();
	void RenderSelectedBox();
	void RenderReloadBox();
	void RenderClearBox();
	void RenderExecuteBox();

	void ReloadSimulationDefinitions();
	void AddSimulationToSelectedList(int id);
	void RemoveSimulationFromSelectedList(int id);
	void Execute();
	void RemoveAllFromList();
	void ReturnToMainMenu();

public:
	Texture* m_backGroundTexture;

	int m_selectedOptionIndex = 0;
	eCurrentSelectedInput m_selectedBox = SELECTABLE_SIMS_BOX;

	std::vector<SimulationUIOption> m_selectableSimulationDefinitions;
	std::vector<SimulationUIOption> m_selectedSimulationDefinitions;


};

