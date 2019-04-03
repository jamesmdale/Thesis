#pragma once
#include "Game\GameStates\GameState.hpp"
#include "Game\Helpers\AnalysisGraph.hpp"
#include "Engine\Core\Rgba.hpp"
#include <map>


//  ----------------------------------------------
struct ImportedProfiledSimulationData
{
	ImportedProfiledSimulationData(const std::string& simDefinitionNameKey, const std::string& path)
	{
		m_simulationDefinitionContentsNameKey = simDefinitionNameKey;
		m_path = path;
	}

	bool IsValid()
	{
		return m_entries == -1;
	}

	std::string m_simulationDefinitionContentsNameKey;
	std::string m_profiledName;
	std::string m_path;

	int m_entries = -1;
	float m_average = 0.f;
	float m_median = 0.f;
	float m_maxValue = 0.f;
	float m_minValue = 0.f;
	float m_standardDeviation = 0.f;
	float m_confidenceInterval95 = 0.f;
	float m_confidenceIntervalRangeLow = 0.f;
	float m_confidenceIntervalRangeHigh = 0.f;
};

//  ----------------------------------------------
struct SelectableProfiledSimDataOption
{

	SelectableProfiledSimDataOption(int optionIndex, ImportedProfiledSimulationData* data, bool isSelected = false)
	{
		m_optionIndex = optionIndex;
		m_data = data;
		m_isSelectedForGraph = isSelected;
	}

	~SelectableProfiledSimDataOption()
	{
		m_data = nullptr;
	}

	int m_optionIndex = -1;
	ImportedProfiledSimulationData* m_data = nullptr;
	bool m_isSelectedForGraph = false;
};


//  ----------------------------------------------
struct SimulationDefinitionContents
{
	std::vector<int> m_simSelectableOptionIndexes;
};

//  ----------------------------------------------
enum AnalysisStateRenderState
{
	ANALYSIS_STATE_INFO_RENDER_STATE,
	ANALYSIS_STATE_GRAPH_RENDER_STATE,
	NUM_ANALYSIS_STATE_RENDER_STATES
};

//  ----------------------------------------------
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

	void InitializeSimulationAnalysisData();

	void SetSimulationDataFilePath(const std::string& rootFilePath);

private:

	//input functions
	float UpdateInputInfoState(float deltaSeconds);
	float UpdateInputGraphState(float deltaSeconds);

	//string helper functions
	std::string GetDefinitionNameFromPath(const std::string& path);
	std::string GetProfiledNameFromFileName(const std::string& filePath);

	//profile generation
	ImportedProfiledSimulationData* GenerateProfiledSimulationDataFromFile(const std::string& definitionName, const std::string& filePath);

	//computations
	void FillSimData(ImportedProfiledSimulationData* simData, const std::vector<float>& data);
	bool ComputeDescriptiveStatistics(ImportedProfiledSimulationData* simData, const std::vector<float>& data);
	float CalculateAverage(const std::vector<float>& data);
	float CalculateStandardDeviation(float average, const std::vector<float>& data);
	void Calculate95PercentConfidenceInterval(ImportedProfiledSimulationData* simData);

	//rendering functions	
	void RenderLoadedDefinitionOptions();
	void RenderLoadedDataContent();
	void RenderSelectedLoadedDataContentDetails();
	void RenderInfoAndInstructions();
	void RenderGraph();

	//ui helpers
	bool IsOptionSelectedForGraph(int optionIndex);
	void SelectOptionForGraph(int optionIndex);
	void DeselectOptionForGraph(int optionIndex);
	void ToggleOptionToGraph(int optionIndex);

	//graph helpers
	void GenerateGraphFromSelectedOptions();

private:
	Texture* m_backGroundTexture = nullptr;
	AnalysisGraph* m_analysisGraph = nullptr;

	std::string m_simulationDataFilePath = "";
	int m_currentHoveredGraphOption = 0;

	AnalysisStateRenderState m_currentRenderState = ANALYSIS_STATE_INFO_RENDER_STATE;

	//matched using ID
	std::map<std::string, SimulationDefinitionContents*> m_definitionsForExecutionMap;	
	std::vector<SelectableProfiledSimDataOption> m_allSelectableOptions;
};