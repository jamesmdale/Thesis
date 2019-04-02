#include "Game\SimulationData.hpp"
#include "Game\Definitions\MapDefinition.hpp"
#include "Engine\Core\StringUtils.hpp"
#include "Engine\Core\EngineCommon.hpp"


//  =============================================================================
SimulationData::SimulationData()
{
}

//  =============================================================================
SimulationData::~SimulationData()
{
	m_simulationDefinitionReference = nullptr;
}

//  =============================================================================
void SimulationData::Initialize(SimulationDefinition* simulationDefinition)
{
	m_simulationDefinitionReference = simulationDefinition;
}

//  =========================================================================================
void SimulationData::CreateComprehensiveDataSet()
{
	//title cell
	AddCell("Simulation");
	AddNewLine();

	AddCell("SimulationName");
	AddCell(Stringf("%s", m_simulationDefinitionReference->m_name.c_str()));
	AddNewLine();

	//num agents
	AddCell("NumAgents");
	AddCell(Stringf("%i", m_simulationDefinitionReference->m_numAgents));
	AddNewLine();

	//num armories
	AddCell("NumArmories");
	AddCell(Stringf("%i", m_simulationDefinitionReference->m_numArmories));
	AddNewLine();

	//num lumberyards
	AddCell("NumLumberyards");
	AddCell(Stringf("%i", m_simulationDefinitionReference->m_numLumberyards));
	AddNewLine();

	//num med stations
	AddCell("NumMedStations");
	AddCell(Stringf("%i", m_simulationDefinitionReference->m_numMedStations));
	AddNewLine();

	//num med stations
	AddCell("NumWells");
	AddCell(Stringf("%i", m_simulationDefinitionReference->m_numWells));
	AddNewLine();

	//bombardment rate
	AddCell("BombardmentRate");
	AddCell(Stringf("%f", m_simulationDefinitionReference->m_bombardmentRatePerSecond));
	AddNewLine();

	//threat rate
	AddCell("ThreatRate");
	AddCell(Stringf("%f", m_simulationDefinitionReference->m_threatRatePerSecond));
	AddNewLine();

	//bombardment starting threat
	AddCell("StartingThreat");
	AddCell(Stringf("%i", m_simulationDefinitionReference->m_startingThreat));
	AddNewLine();

	//map definition name
	AddCell("MapDimensions");
	AddCell(Stringf("%f,%f", m_simulationDefinitionReference->m_mapDefinition->m_width, m_simulationDefinitionReference->m_mapDefinition->m_height));
	AddNewLine();

	//map definitions
	AddCell("MapName");
	AddCell(Stringf("%s", m_simulationDefinitionReference->m_mapDefinition->m_name.c_str()));
	AddNewLine();
	AddNewLine();

	AddCell("StartingTime");
	AddCell(Stringf("%s", GetCurrentDateTime().c_str()));
	AddNewLine();

	//optimization?
	std::string optimizedText = "";
	m_simulationDefinitionReference->m_isOptimized ? optimizedText = "Yes" : optimizedText = "No";

	AddCell("IsOptimized");
	AddCell(optimizedText.c_str());
	AddNewLine();
}

//  =============================================================================
void SimulationData::WriteEntryWithTimeStamp(const std::string& entry)
{
	SimulationDataEntry dataEntry = SimulationDataEntry();
	dataEntry.m_value = entry;
	dataEntry.m_timeStamp = GetCurrentDateTime();

	m_entries.push_back(dataEntry);	

	dataEntry.m_value = "/n";
	m_entries.push_back(dataEntry);	
}

//  =============================================================================
bool SimulationData::ExportCSV(const std::string& filePath, const std::string& fileName)
{
	for (int entryIndex = 0; entryIndex < (int)m_entries.size(); ++entryIndex)
	{
		AddCell(Stringf("%s", m_entries[entryIndex].m_value.c_str()));
		//AddCell(Stringf("%s", m_entries[entryIndex].m_timeStamp.c_str()));
		//AddNewLine();
	}
	bool success = WriteToFile(Stringf("%s%s", filePath.c_str(), fileName.c_str()));

	ResetData();

	return success;
}

//  =============================================================================
void SimulationData::ResetData()
{
	m_entries.clear();
	ClearContent();
}

