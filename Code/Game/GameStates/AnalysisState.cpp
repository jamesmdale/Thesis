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
	RenderInfoAndInstructions();
	RenderLoadedDefinitionOptions();
	RenderLoadedDataContent();	

	//draw graph if we are showing it
	if (m_currentRenderState == ANALYSIS_STATE_GRAPH_RENDER_STATE)
	{
		RenderGraph();
	}

	theRenderer->m_defaultShader->DisableBlending();

	theRenderer = nullptr;
}

//  =========================================================================================
float AnalysisState::UpdateFromInput(float deltaSeconds)
{
	switch (m_currentRenderState)
	{
	case ANALYSIS_STATE_INFO_RENDER_STATE:
		deltaSeconds = UpdateInputInfoState(deltaSeconds);
		break;
	case ANALYSIS_STATE_GRAPH_RENDER_STATE:
		deltaSeconds = UpdateInputGraphState(deltaSeconds);
		break;
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
	std::map<std::string, SimulationDefinitionContents*>::iterator mapIterator = m_definitionsForExecutionMap.begin();
	for (mapIterator = m_definitionsForExecutionMap.begin(); mapIterator != m_definitionsForExecutionMap.end(); ++mapIterator);
	{
		SimulationDefinitionContents* simContent = mapIterator->second;
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

	int selectableForGraphCount = 0;
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
		SimulationDefinitionContents* dataContents = new SimulationDefinitionContents();
		m_definitionsForExecutionMap.insert(std::pair<std::string, SimulationDefinitionContents*>(definitionName, dataContents));

		//populate the data contents with each profiled simulation data set
		for (int fileIndex = 0; fileIndex < (int)containedFilePaths.size(); ++fileIndex)
		{
			selectableForGraphCount++;

			//generate sim data
			ImportedProfiledSimulationData* profiledData = GenerateProfiledSimulationDataFromFile(definitionName, containedFilePaths[fileIndex]);

			//add option to list of selectable sims
			int optionIndex = (int)m_allSelectableOptions.size();
			SelectableProfiledSimDataOption option = SelectableProfiledSimDataOption(optionIndex, profiledData, false);
			dataContents->m_simSelectableOptionIndexes.push_back(optionIndex);
			m_allSelectableOptions.push_back(option);			
		}
	}
}

//  =========================================================================================
void AnalysisState::SetSimulationDataFilePath(const std::string& rootFilePath)
{
	m_simulationDataFilePath = rootFilePath;
}

//  =========================================================================================
void AnalysisState::RenderGraph()
{
	Renderer* theRenderer = Renderer::GetInstance();
	Window* theWindow = Window::GetInstance();

	//reset texture
	theRenderer->SetTexture(*m_backGroundTexture);
	theRenderer->SetShader(theRenderer->m_defaultShader);
	theRenderer->m_defaultShader->EnableColorBlending(BLEND_OP_ADD, BLEND_SOURCE_ALPHA, BLEND_ONE_MINUS_SOURCE_ALPHA);

	//draw transparent overlay on what we've already drawn
	theRenderer->DrawAABB(theWindow->GetClientWindow(), Rgba(0.f, 0.f, 0.f, 0.85f));

	//draw the graph
	m_analysisGraph->Render();
}

//  =========================================================================================
void AnalysisState::RenderLoadedDefinitionOptions()
{
	Renderer* theRenderer = Renderer::GetInstance();
	Window* theWindow = Window::GetInstance();

	static Rgba hoveredColor = Rgba::WHITE;
	static Rgba simHeaderColor = Rgba::YELLOW;
	static Rgba nonHoveredColor = Rgba::GRAY;
	static Rgba colorInSelectedList = Rgba::GREEN;

	static AABB2 exportedDataOptionsBox = AABB2(Vector2(theWindow->m_clientWidth * 0.025, theWindow->m_clientHeight * 0.05), Vector2(theWindow->m_clientWidth * 0.35, theWindow->m_clientHeight * 0.95));

	//reset texture
	theRenderer->SetTexture(*m_backGroundTexture);
	theRenderer->SetShader(theRenderer->m_defaultShader);
	theRenderer->m_defaultShader->EnableColorBlending(BLEND_OP_ADD, BLEND_SOURCE_ALPHA, BLEND_ONE_MINUS_SOURCE_ALPHA);

	//draw options
	theRenderer->DrawAABB(exportedDataOptionsBox, Rgba(0.2f, 0.2f, 0.2f, 1.f));

	//draw each type of reset
	Vector2 exportedDataOptionsBoxStart = exportedDataOptionsBox.GetTopLeftPosition();
	Vector2 exportDataOptionsBoxDimensions = exportedDataOptionsBox.GetDimensions();

	int optionDisplacementCount = 0;
	int simCount = 0;

	float startHeight = 0.9f;
	float heightDecrementAmount = 0.02f;	
	std::map<std::string, SimulationDefinitionContents*>::iterator contentsIterator;

	for (contentsIterator = m_definitionsForExecutionMap.begin(); contentsIterator != m_definitionsForExecutionMap.end(); ++contentsIterator)
	{
		Rgba textColor;
		textColor = simHeaderColor;

		++simCount;

		//draw simulation paths
		Vector2 textPosition = Vector2(exportedDataOptionsBoxStart.x + (exportDataOptionsBoxDimensions.x * 0.05f), ((exportedDataOptionsBoxStart.y * 0.95) - (exportDataOptionsBoxDimensions.y * float(optionDisplacementCount) * heightDecrementAmount)));
		theRenderer->DrawText2D(textPosition,
			Stringf("%i) %s", simCount, contentsIterator->first.c_str()).c_str(),
			theWindow->m_clientHeight * 0.015f,
			textColor,
			1.f,
			Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));

		SimulationDefinitionContents* simContents = contentsIterator->second;
		++optionDisplacementCount;

		for (int simContentsOptionIndex = 0; simContentsOptionIndex < (int)simContents->m_simSelectableOptionIndexes.size(); ++simContentsOptionIndex)
		{
			int optionIndex = simContents->m_simSelectableOptionIndexes[simContentsOptionIndex];
			SelectableProfiledSimDataOption& currentOption = m_allSelectableOptions[optionIndex];

			if (IsOptionSelectedForGraph(optionIndex))
				m_currentHoveredGraphOption == optionIndex ? textColor = hoveredColor : textColor = colorInSelectedList;
			else
				m_currentHoveredGraphOption == optionIndex ? textColor = hoveredColor : textColor = nonHoveredColor;

			//draw simulation paths
			Vector2 textPosition = Vector2(exportedDataOptionsBoxStart.x + (exportDataOptionsBoxDimensions.x * 0.05f), ((exportedDataOptionsBoxStart.y * 0.95) - (exportDataOptionsBoxDimensions.y * float(optionDisplacementCount) * heightDecrementAmount)));
			theRenderer->DrawText2D(textPosition,
				Stringf("	- %s", currentOption.m_data->m_profiledName.c_str()).c_str(),
				theWindow->m_clientHeight * 0.015f,
				textColor,
				1.f,
				Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));

			++optionDisplacementCount;
		}				
	}
}

//  =========================================================================================
void AnalysisState::RenderLoadedDataContent()
{
	Renderer* theRenderer = Renderer::GetInstance();
	Window* theWindow = Window::GetInstance();

	static Rgba detailTextColor = Rgba::WHITE;

	static AABB2 simDetailBox = AABB2(Vector2(theWindow->m_clientWidth * 0.375, theWindow->m_clientHeight * 0.55), Vector2(theWindow->m_clientWidth * 0.95, theWindow->m_clientHeight * 0.95));
	static AABB2 profiledSimulationDataBox = AABB2(Vector2(theWindow->m_clientWidth * 0.375, theWindow->m_clientHeight * 0.05), Vector2(theWindow->m_clientWidth * 0.95, theWindow->m_clientHeight * 0.525));

	//set render settings back to default
	theRenderer->SetTexture(*m_backGroundTexture);
	theRenderer->SetShader(theRenderer->m_defaultShader);
	theRenderer->m_defaultShader->EnableColorBlending(BLEND_OP_ADD, BLEND_SOURCE_ALPHA, BLEND_ONE_MINUS_SOURCE_ALPHA);	

	//render boxes
	theRenderer->DrawAABB(simDetailBox, Rgba(0.2f, 0.2f, 0.2f, 1.f));
	theRenderer->DrawAABB(profiledSimulationDataBox, Rgba(0.2f, 0.2f, 0.2f, 1.f));

	//get sim definition data
	TODO("GET SIM DEFINITION DATA FROM GENERAL INFO LOADED LIST");

	//get hovered profiled content

}

//  =========================================================================================
void AnalysisState::RenderSelectedLoadedDataContentDetails()
{

}

//  =========================================================================================
void AnalysisState::RenderInfoAndInstructions()
{
	Renderer* theRenderer = Renderer::GetInstance();
	Window* theWindow = Window::GetInstance();

	//draw simulation paths
	theRenderer->DrawText2DCentered(Vector2(theWindow->m_clientWidth * .5f, theWindow->m_clientHeight * 0.975f),
		Stringf("Analyzing Simulation: %s", m_simulationDataFilePath.c_str()),
		theWindow->m_clientHeight * 0.01f,
		Rgba::YELLOW,
		1.f,
		Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));

	theRenderer->DrawText2DCentered(Vector2(theWindow->m_clientWidth * .5f, theWindow->m_clientHeight * 0.015f),
		"Press 'ENTER' to Add Data to Graph - Press 'SPACE' to Generate Graph - Press 'ESCAPE' to Return to Analysis Select",
		theWindow->m_clientHeight * 0.01f,
		Rgba::YELLOW,
		1.f,
		Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));
}

//  =========================================================================================
bool AnalysisState::IsOptionSelectedForGraph(int optionIndex)
{
	return m_allSelectableOptions[optionIndex].m_isSelectedForGraph;
}

//  =========================================================================================
void AnalysisState::SelectOptionForGraph(int optionIndex)
{
	m_allSelectableOptions[optionIndex].m_isSelectedForGraph = true;
}

//  =========================================================================================
void AnalysisState::DeselectOptionForGraph(int optionIndex)
{
	m_allSelectableOptions[optionIndex].m_isSelectedForGraph = false;
}

//  =========================================================================================
void AnalysisState::ToggleOptionToGraph(int optionIndex)
{
	if (!IsOptionSelectedForGraph(optionIndex))
		SelectOptionForGraph(optionIndex);
	else
		DeselectOptionForGraph(optionIndex);
}

//  =========================================================================================
void AnalysisState::GenerateGraphFromSelectedOptions()
{
	m_analysisGraph = new AnalysisGraph();

	//for every option selected, add it to the list
	for (int optionIndex = 0; optionIndex < (int)m_allSelectableOptions.size(); ++optionIndex)
	{
		if (m_allSelectableOptions[optionIndex].m_isSelectedForGraph)
			m_analysisGraph->AddDataToGraph(m_allSelectableOptions[optionIndex].m_data);
	}

	m_analysisGraph->GenerateGraph();
}

//  =========================================================================================
float AnalysisState::UpdateInputInfoState(float deltaSeconds)
{
	InputSystem* theInput = InputSystem::GetInstance();

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_W) || theInput->WasKeyJustPressed(theInput->KEYBOARD_UP_ARROW))
	{
		int newSelectedOption = (m_currentHoveredGraphOption - 1);
		if (newSelectedOption == -1)
			newSelectedOption = (int)m_allSelectableOptions.size() - 1;
		m_currentHoveredGraphOption = newSelectedOption % (int)m_allSelectableOptions.size();
	}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_S) || theInput->WasKeyJustPressed(theInput->KEYBOARD_DOWN_ARROW))
	{
		m_currentHoveredGraphOption = (m_currentHoveredGraphOption + 1) % (int)m_allSelectableOptions.size();
	}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_ENTER))
	{
		ToggleOptionToGraph(m_currentHoveredGraphOption);
	}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_SPACE))
	{
		GenerateGraphFromSelectedOptions();
		m_currentRenderState = ANALYSIS_STATE_GRAPH_RENDER_STATE;
	}

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_ESCAPE))
	{
		ResetState();
		GameState::TransitionGameStates(GetGameStateFromGlobalListByType(ANALYSIS_SELECT_GAME_STATE));
	}

	return deltaSeconds;
}

//  =========================================================================================
float AnalysisState::UpdateInputGraphState(float deltaSeconds)
{
	InputSystem* theInput = InputSystem::GetInstance();

	if (theInput->WasKeyJustPressed(theInput->KEYBOARD_ESCAPE))
	{
		m_currentRenderState = ANALYSIS_STATE_INFO_RENDER_STATE;
	}

	return deltaSeconds;
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
ImportedProfiledSimulationData* AnalysisState::GenerateProfiledSimulationDataFromFile(const std::string& definitionName, const std::string& filePath)
{
	//perform calculation
	ImportedProfiledSimulationData* simData = new ImportedProfiledSimulationData(definitionName, filePath);

	CSVEditor editor;
	if (!editor.ReadFromFile(filePath))
		return simData;

	std::vector<float> contentAsFloats;
	for (int entryIndex = 0; entryIndex < (int)editor.m_content.size(); ++entryIndex)
	{
		float valueAsFloat = ConvertStringToFloat(editor.m_content[entryIndex]);
		ASSERT_OR_DIE(valueAsFloat != FLOAT_MAX, "INVALID READ FOR SIMULATION DATA FILE!!");

		contentAsFloats.push_back(valueAsFloat);
	}	

	FillSimData(simData, contentAsFloats);

	return simData;
}

//  =========================================================================================
void AnalysisState::FillSimData(ImportedProfiledSimulationData* simData, const std::vector<float>& data)
{
	//get the file name
	simData->m_profiledName = GetProfiledNameFromFileName(simData->m_path);

	//compute average, median, standard deviation, population size, and 95% confidence interval
	bool success = ComputeDescriptiveStatistics(simData, data);
	UNUSED(success);
}

// Computes average, median, standard deviation, pop size, and 95% confidence interval =========================================================================================
bool AnalysisState::ComputeDescriptiveStatistics(ImportedProfiledSimulationData* simData, const std::vector<float>& data)
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
		//we need to get an average of this index and index - 1
		median = (data[(int)medianIndex] + data[(int)medianIndex - 1]) * 0.5f;
	else
		median = data[(int)medianIndex];

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

