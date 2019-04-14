#pragma once
#include <vector>
#include <string>
#include "Engine\File\CSVEditor.hpp"
#include "Game\Definitions\SimulationDefinition.hpp"


struct SimulationDataEntry
{
	std::string m_value = "";
	std::string m_timeStamp = "";	
};

class SimulationData : public CSVEditor
{
public:
	SimulationData();
	~SimulationData();

	void Initialize(SimulationDefinition* simulationDefinition);

	void WriteGeneralData();

	void WriteEntryWithTimeStamp(const std::string& entry);
	bool ExportCSV(const std::string& filePath, const std::string& fileName);
	void ResetData();

public:
	std::vector<SimulationDataEntry> m_entries;
	SimulationDefinition* m_simulationDefinitionReference = nullptr;
};

