#include "Game\GameStates\AnalysisState.hpp"
#include "Engine\File\File.hpp"
#include "Engine\File\FileHelpers.hpp"
#include "Engine\Window\Window.hpp"
#include "Engine\Core\StringUtils.hpp"
#include "Engine\File\CSVEditor.hpp"

//  =========================================================================================
AnalysisState::~AnalysisState()
{
	m_backGroundTexture = nullptr;
}

//  =========================================================================================
void AnalysisState::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

//  =========================================================================================
void AnalysisState::PreRender()
{
}

//  =========================================================================================
void AnalysisState::Render()
{
	Renderer* theRenderer = Renderer::GetInstance();
	Window* theWindow = Window::GetInstance();

	theRenderer->SetCamera(m_camera);

	theRenderer->ClearDepth(1.f);
	theRenderer->ClearColor(Rgba::BLACK);

	theRenderer->SetTexture(*m_backGroundTexture);
	theRenderer->SetShader(theRenderer->m_defaultShader);

	theRenderer->m_defaultShader->EnableColorBlending(BLEND_OP_ADD, BLEND_SOURCE_ALPHA, BLEND_ONE_MINUS_SOURCE_ALPHA);
	theRenderer->DrawAABB(theWindow->GetClientWindow(), Rgba(0.f, 0.f, 0.f, 1.f));

	//draw options
	RenderOptionsList();

	//draw graph
	RenderGraph();

	//draw simulation paths
	theRenderer->DrawText2DCentered(Vector2(theWindow->m_clientWidth * .5f, theWindow->m_clientHeight * 0.975f),
		Stringf("Analyzing Simulation: %s", m_simulationDataFilePath.c_str()),
		theWindow->m_clientHeight * 0.01f,
		Rgba::YELLOW,
		1.f,
		Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));	

	theRenderer->DrawText2DCentered(Vector2(theWindow->m_clientWidth * .5f, theWindow->m_clientHeight * 0.015f),
		"Press 'ENTER' to Select OR 'ESCAPE' to Return to Analysis Select",
		theWindow->m_clientHeight * 0.01f,
		Rgba::YELLOW,
		1.f,
		Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));

	theRenderer->m_defaultShader->DisableBlending();

	theRenderer = nullptr;
}

//  =========================================================================================
float AnalysisState::UpdateFromInput(float deltaSeconds)
{
	InputSystem* theInput = InputSystem::GetInstance();

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_W) || theInput->WasKeyJustPressed(theInput->KEYBOARD_UP_ARROW))
	{
		//m_selectedSimulationPathIndex = (m_selectedSimulationPathIndex + 1) % m_simulationPaths.size();
	}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_S) || theInput->WasKeyJustPressed(theInput->KEYBOARD_DOWN_ARROW))
	{
		//m_selectedSimulationPathIndex = (m_selectedSimulationPathIndex - 1) % m_simulationPaths.size();
	}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_ENTER))
	{

	}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_ESCAPE))
	{
		ResetState();
		GameState::TransitionGameStates(GetGameStateFromGlobalListByType(ANALYSIS_SELECT_GAME_STATE));
	}

	return deltaSeconds; //new deltaSeconds
}

//  =========================================================================================
void AnalysisState::Initialize()
{

}

//  =========================================================================================
void AnalysisState::TransitionIn(float secondsTransitioning)
{	
	//detect all simulations
	InitializeSimulationAnalysisData();
	
	s_isFinishedTransitioningIn = true;
}

//  =========================================================================================
void AnalysisState::TransitionOut(float secondsTransitioning)
{
	ResetState();
	s_isFinishedTransitioningOut = true;
}

//  =========================================================================================
void AnalysisState::ResetState()
{
	std::map<std::string, SimulationContents*>::iterator mapIterator = m_definitionsForExecutionMap.begin();
	for (mapIterator = m_definitionsForExecutionMap.begin(); mapIterator != m_definitionsForExecutionMap.end(); ++mapIterator);
	{
		SimulationContents* simContent = mapIterator->second;
		delete(simContent);
		simContent = nullptr;
	}

	m_definitionsForExecutionMap.clear();

	delete(m_analysisGraph);
	m_analysisGraph = nullptr;

	m_simulationDataFilePath = "";
}

//  =========================================================================================
void AnalysisState::InitializeSimulationAnalysisData()
{
	std::vector<std::string> simulationDefinitionPaths;
	ReadSubFolderNamesForPath(m_simulationDataFilePath.c_str(), simulationDefinitionPaths);

	for (int definitionIndex = 0; definitionIndex < (int)simulationDefinitionPaths.size(); ++definitionIndex)
	{
		//read in all of the contained file paths.
		std::vector<std::string> containedFilePaths;
		ReadContainedFilePathsForPath(simulationDefinitionPaths[definitionIndex], containedFilePaths);

		//if there aren't any files in side this folder, return
		if (containedFilePaths.size() == 0)
			return;

		//add structure to map
		std::string definitionName = GetDefinitionNameFromPath(simulationDefinitionPaths[definitionIndex]);
		SimulationContents* dataContents = new SimulationContents();
		m_definitionsForExecutionMap.insert(std::pair<std::string, SimulationContents*>(definitionName, dataContents));

		//populate the data contents with each profiled simulation data set
		for (int fileIndex = 0; fileIndex < (int)containedFilePaths.size(); ++fileIndex)
		{
			ImportedProfiledSimulationData* profiledData = GenerateProfiledSimulationDataFromFile(containedFilePaths[fileIndex]);
			dataContents->m_dataContents.push_back(profiledData);
		}
	}
}

//  =========================================================================================
void AnalysisState::RenderOptionsList()
{
	Renderer* theRenderer = Renderer::GetInstance();
	Window* theWindow = Window::GetInstance();

	static Rgba selectedColor = Rgba::WHITE;
	static Rgba nonSelectedColor = Rgba::GRAY;

	static AABB2 exportedDataOptionsBox = AABB2(Vector2(theWindow->m_clientWidth * 0.025, theWindow->m_clientHeight * 0.05), Vector2(theWindow->m_clientWidth * 0.35, theWindow->m_clientHeight * 0.95));

	//draw options
	theRenderer->DrawAABB(exportedDataOptionsBox, Rgba(0.2f, 0.2f, 0.2f, 1.f));

	//render each option
}

//  =========================================================================================
void AnalysisState::RenderGraph()
{
	Renderer* theRenderer = Renderer::GetInstance();
	Window* theWindow = Window::GetInstance();

	static AABB2 graphBox = AABB2(Vector2(theWindow->m_clientWidth * 0.4, theWindow->m_clientHeight * 0.05), Vector2(theWindow->m_clientWidth * 0.95, theWindow->m_clientHeight * 0.95));

	//draw the graph
	theRenderer->DrawAABB(graphBox, Rgba(0.8f, 0.8f, 0.8f, 0.8f));
}

//  =========================================================================================
std::string AnalysisState::GetDefinitionNameFromPath(const std::string& path)
{
	//structure should be Data\ExportedSimulationData\SIMULATION_TEST_3-26-119_19.37.41\Simulation_Definition_100_optimized
	//we want everything that follows '..Simulation_Definition_'
	std::string definitionName = GetRemainingStringFromFoundString(path, "Simulation_Definition_");
	//ASSERT_OR_DIE(definitionName.length() == 0, "SIM DEFINITION IMPORT READ INVALID!!!");

	return definitionName;
}

//  =========================================================================================
std::string AnalysisState::GetProfiledNameFromFileName(const std::string& filePath)
{
	//structure should be "Data\ExportedSimulationData\SIMULATION_TEST_3-26-119_19.37.41\Simulation_Definition_100_optimized\ActionStackAverageTimersPer_100_optimized.csv"
	//we want 'ActionStackAverageTimersPer' as the name
	std::string outFileName = "";
	bool success = GetFileNameFromPathNoExtension(filePath, outFileName);
	ASSERT_OR_DIE(success, "INVALID READ FOR PROFILED SIMULATION DATA FILE!!!");

	outFileName = GetPrecedingStringFromFoundString(outFileName, "_");

	return outFileName;
}

//  =========================================================================================
ImportedProfiledSimulationData* AnalysisState::GenerateProfiledSimulationDataFromFile(const std::string& filePath)
{
	std::string fileName = GetProfiledNameFromFileName(filePath);

	CSVEditor editor;
	editor.ReadFromFile(filePath);

	std::vector<float> contentAsFloats;
	for (int entryIndex = 0; entryIndex < (int)editor.m_content.size(); ++entryIndex)
	{
		float valueAsFloat = ConvertStringToFloat(editor.m_content[entryIndex]);
		ASSERT_OR_DIE(valueAsFloat != NULL, "INVALID READ FOR SIMULATION DATA FILE!!");

		contentAsFloats.push_back(valueAsFloat);
	}

	//perform calculation
	ImportedProfiledSimulationData* simData = new ImportedProfiledSimulationData();

	ComputeDescriptiveStatistics(simData, contentAsFloats);

	return simData;
}

//  =========================================================================================
void AnalysisState::ComputeDescriptiveStatistics(ImportedProfiledSimulationData* simData, const std::vector<float>& data)
{
	//first calculate mean
	bool success = FillSimData(simData, data);

	if (!success)
		return;
}

//  =========================================================================================
bool AnalysisState::FillSimData(ImportedProfiledSimulationData* simData, const std::vector<float>& data)
{
	float nCount = (float)data.size();
	
	//early out in case our count is 0
	if (nCount == 0)
		return false;

	float sum = 0.f;
	float minValue = INT_MAX;
	float maxValue = INT_MIN;
	float median = INT_MIN;

	//compute sum to be used for average and get max and min values while we are at it
	for (int dataIndex = 0; dataIndex < (int)nCount; ++dataIndex)
	{
		sum += data[dataIndex];

		if (data[dataIndex] > maxValue)
			maxValue = data[dataIndex];
		else if (data[dataIndex] < minValue)
			minValue = data[dataIndex];
	}

	//calculate median
	float medianIndex = nCount * 0.5f;
	if (floorf(medianIndex) == medianIndex)
	{
		//we need to get an average of this index and index - 1
		median = (data[(int)medianIndex] + data[(int)medianIndex - 1]) * 0.5f;
	}
	else
	{
		median = data[(int)medianIndex];
	}	

	//fill data
	simData->m_entries = (int)nCount;
	simData->m_minValue = minValue;
	simData->m_maxValue = maxValue;
	simData->m_median = median;
	simData->m_average = sum / nCount;
	simData->m_standardDeviation = CalculateStandardDeviation(simData->m_average, data);
	Calculate95PercentConfidenceInterval(simData);

	return true;
}

//  =========================================================================================
float AnalysisState::CalculateAverage(const std::vector<float>& data)
{
	float nCount = (float)data.size();
	float sum = 0.f;
	for (int dataIndex = 0; dataIndex < (int)nCount; ++dataIndex)
	{
		sum += data[dataIndex];
	}

	return sum / nCount;
}

//  =========================================================================================
float AnalysisState::CalculateStandardDeviation(float average, const std::vector<float>& data)
{
	float adjustedSum = 0.f;

	for (int dataIndex = 0; dataIndex < (int)data.size(); ++dataIndex)
	{
		float indexMinusMean = data[dataIndex] - average;
		adjustedSum += (indexMinusMean * indexMinusMean);
	}

	return sqrt((1.f / float(data.size()) * adjustedSum));
}

//  =========================================================================================
void AnalysisState::Calculate95PercentConfidenceInterval(ImportedProfiledSimulationData* simData)
{
	//1.96 corresponds to the z value
	simData->m_confidenceInterval95 = 1.96f * (simData->m_standardDeviation / (sqrtf((float)simData->m_entries)));

	simData->m_confidenceIntervalRangeLow = simData->m_average - simData->m_confidenceInterval95;
	simData->m_confidenceIntervalRangeHigh = simData->m_average + simData->m_confidenceInterval95;	
}

//  =========================================================================================
void AnalysisState::PostRender()
{

}

