#pragma once
#include "Game\GameStates\GameState.hpp"
#include "Game\Helpers\AnalysisGraph.hpp"
#include "Engine\Core\Rgba.hpp"
#include <map>

//  ----------------------------------------------
struct ImportedProfiledSimulationData
{
	ImportedProfiledSimulationData(const std::string& path)
	{
		m_path = path;
	}

	bool IsValid()
	{
		return m_entries == -1;
	}

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
struct SimulationContents
{
	SimulationContents() {};
	~SimulationContents()
	{
		importedStatisticData.clear();
	}

	std::vector<ImportedProfiledSimulationData*> importedStatisticData;
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

	//string helper functions
	std::string GetDefinitionNameFromPath(const std::string& path);
	std::string GetProfiledNameFromFileName(const std::string& filePath);

	//profile generation
	ImportedProfiledSimulationData* GenerateProfiledSimulationDataFromFile(const std::string& filePath);

	//computations
	void FillSimData(ImportedProfiledSimulationData* simData, const std::vector<float>& data);
	bool ComputeDescriptiveStatistics(ImportedProfiledSimulationData* simData, const std::vector<float>& data);
	float CalculateAverage(const std::vector<float>& data);
	float CalculateStandardDeviation(float average, const std::vector<float>& data);
	void Calculate95PercentConfidenceInterval(ImportedProfiledSimulationData* simData);

	//rendering functions
	void RenderGraph();
	void RenderLoadedDefinitionOptions();
	void RenderLoadedDataContent();
	void RenderSelectedLoadedDataContentDetails();

	//graph helpers
	bool IsOptionSelectedForGraph(int optionIndex);
	void DeselectOptionForGraph(int optionIndex);
	void ToggleOptionToGraph(int optionIndex);

public:
	Texture* m_backGroundTexture = nullptr;
	AnalysisGraph* m_analysisGraph = nullptr;
	bool m_isGraphRendering = false;

	std::string m_simulationDataFilePath = "";
	int m_selectedGraphOption = 0;
	int m_totalSelectableGraphOptions = 0;

	//matched using ID
	std::map<std::string, SimulationContents*> m_definitionsForExecutionMap;	

	std::vector<int> m_optionsSelectedForAnalysis;
};