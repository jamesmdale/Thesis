#pragma once
#include <vector>
#include <string>
#include "Engine\File\CSVEditor.hpp"
#include "Game\Definitions\SimulationDefinition.hpp"


class SimulationData : public CSVEditor
{
public:
	SimulationData();
	~SimulationData();

	void Initialize(SimulationDefinition* simulationDefinition);

	void WriteGeneralData();

	bool ExportCSV(const std::string& filePath, const std::string& fileName);
	void ResetData();

public:
	SimulationDefinition* m_simulationDefinitionReference = nullptr;
	uint64_t m_count = 0;
};

