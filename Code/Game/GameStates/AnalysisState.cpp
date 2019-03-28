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

	return simData;
}

//  =========================================================================================
void AnalysisState::PostRender()
{

}

