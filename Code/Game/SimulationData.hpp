#pragma once
#include <vector>
#include <string>
#include "Engine\File\CSVWriter.hpp"
#include "Game\Definitions\SimulationDefinition.hpp"


struct SimulationDataEntry
{
	std::string m_value = "";
	std::string m_timeStamp = "";	
};

class SimulationData : public CSVWriter
{
public:
	SimulationData();
	~SimulationData();

	void Initialize(SimulationDefinition* simulationDefinition);

	void CreateComprehensiveDataSet();

	void WriteEntryWithTimeStamp(const std::string& entry);
	bool ExportCSV(const std::string& filePath, const std::string& fileName);
	void ResetData();

public:
	std::vector<SimulationDataEntry> m_entries;
	SimulationDefinition* m_simulationDefinitionReference = nullptr;
};

