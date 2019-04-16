#include "Game\SimulationData.hpp"
#include "Game\Definitions\MapDefinition.hpp"
#include "Game\GameCommon.hpp"
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
	m_content.reserve(MAX_SIMULATION_DATA_CSV_SIZE);
}

//  =========================================================================================
void SimulationData::WriteGeneralData()
{
	//title cell
	AddCell(Stringf("%s: %s,", SIM_NAME_OUTPUT_TEXT, m_simulationDefinitionReference->m_name.c_str()));
	AddNewLine();

	//num agents
	AddCell(Stringf("%s: %i", NUM_AGENTS_OUTPUT_TEXT, m_simulationDefinitionReference->m_numAgents));
	AddNewLine();

	//num armories
	AddCell(Stringf("%s: %i", NUM_ARMORIES_OUTPUT_TEXT, m_simulationDefinitionReference->m_numArmories));
	AddNewLine();

	//num lumberyards
	AddCell(Stringf("%s: %i", NUM_LUMBERYARDS_OUTPUT_TEXT, m_simulationDefinitionReference->m_numLumberyards));
	AddNewLine();

	//num med stations
	AddCell(Stringf("%s: %i", NUM_MED_STATIONS_OUTPUT_TEXT, m_simulationDefinitionReference->m_numMedStations));
	AddNewLine();

	//num med stations
	AddCell(Stringf("%s: %i", NUM_WELLS_OUTPUT_TEXT, m_simulationDefinitionReference->m_numWells));
	AddNewLine();

	//bombardment rate
	AddCell(Stringf("%s: %f", BOMBARDMENT_RATE_OUTPUT_TEXT, m_simulationDefinitionReference->m_bombardmentRatePerSecond));
	AddNewLine();

	//threat rate
	AddCell(Stringf("%s: %f", THREAT_RATE_OUTPUT_TEXT, m_simulationDefinitionReference->m_threatRatePerSecond));
	AddNewLine();

	//bombardment starting threat
	AddCell(Stringf("%s: %i", STARTING_THREAT_OUTPUT_TEXT, m_simulationDefinitionReference->m_startingThreat));
	AddNewLine();

	//map definition name
	AddCell(Stringf("%s: %f%f", MAP_DIMENSIONS_OUTPUT_TEXT, m_simulationDefinitionReference->m_mapDefinition->m_width, m_simulationDefinitionReference->m_mapDefinition->m_height));
	AddNewLine();

	//map definitions
	AddCell(Stringf("%s: %s", MAP_NAME_OUTPUT_TEXT, m_simulationDefinitionReference->m_mapDefinition->m_name.c_str()));
	AddNewLine();

	AddCell(Stringf("%s: %s", STARTING_TIME_OUTPUT_TEXT, GetCurrentDateTime().c_str()));
	AddNewLine();

	//optimization?
	std::string optimizedText = "";
	m_simulationDefinitionReference->m_isOptimized ? optimizedText = "true" : optimizedText = "false";

	AddCell(Stringf("%s: %s", OPTIMIZED_OUTPUT_TEXT, optimizedText.c_str()));
	AddNewLine();

	//optimization?
	std::string budgetedText = "";
	m_simulationDefinitionReference->m_isUpdateBudgeted ? budgetedText = "true" : budgetedText = "false";

	AddCell(Stringf("%s: %s", BUDGETED_OUTPUT_TEXT, budgetedText.c_str()));
	AddNewLine();
}


//  =============================================================================
bool SimulationData::ExportCSV(const std::string& filePath, const std::string& fileName)
{
	bool success = WriteToFile(Stringf("%s%s", filePath.c_str(), fileName.c_str()));
	ResetData();

	return success;
}

//  =============================================================================
void SimulationData::ResetData()
{
	ClearContent();
}

