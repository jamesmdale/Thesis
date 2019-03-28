#pragma once
#include "Game\GameStates\GameState.hpp"
#include "Game\Helpers\AnalysisGraph.hpp"
#include "Engine\Core\Rgba.hpp"
#include <map>

struct ImportedProfiledSimulationData
{
	std::string m_profiledName;
	std::string m_path;

	int m_entries = 0;
	float m_average = 0.f;
	float m_median = 0.f;
	float m_maxValue = 0.f;
	float m_minValue = 0.f;
	float m_standardDeviation = 0.f;
	float m_confidenceInterval95 = 0.f;
};

struct SimulationContents
{
	SimulationContents() {};
	~SimulationContents()
	{
		m_dataContents.clear();
	}

	std::vector<ImportedProfiledSimulationData*> m_dataContents;
};

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
	void RenderOptionsList();
	void RenderGraph();

	//string helper functions
	std::string GetDefinitionNameFromPath(const std::string& path);
	std::string GetProfiledNameFromFileName(const std::string& filePath);
	ImportedProfiledSimulationData* GenerateProfiledSimulationDataFromFile(const std::string& filePath);

public:
	Texture* m_backGroundTexture = nullptr;
	AnalysisGraph* m_analysisGraph = nullptr;

	std::string m_simulationDataFilePath = "";

	//matched using ID
	std::map<std::string, SimulationContents*> m_definitionsForExecutionMap;	
};