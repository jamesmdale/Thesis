#pragma once
#include <string>
#include <vector>
#include "Engine\ThirdParty\tinyxml2\tinyxml2.h"
#include "Engine\Math\IntVector2.hpp"

class MapDefinition;

class SimulationDefinition
{
public:
	explicit SimulationDefinition( const tinyxml2::XMLElement& element );
	~SimulationDefinition();
	static void Initialize(const std::string& filePath);
	static SimulationDefinition* GetSimulationByName(const std::string& definitionName);

public:
	std::string m_name = "default";
	std::string m_mapName = "";
	int m_numAgents = 0;
	int m_numArmories = 0;
	int m_numLumberyards = 0;
	int m_numMedStations = 0;
	int m_numWells = 0;
	float m_bombardmentRatePerSecond = 0.f;
	float m_threatRatePerSecond = 0.f;
	int m_startingThreat = 0;
	float m_totalProcessingTimeInSeconds = 0.f;
	bool m_isOptimized = false;
	bool m_isUpdateBudgeted = false;

	MapDefinition* m_mapDefinition = nullptr;

	static std::vector<SimulationDefinition*> s_simulationDefinitions;
};

