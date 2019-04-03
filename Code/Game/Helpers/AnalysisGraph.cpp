#include "Game\Helpers\AnalysisGraph.hpp"
#include "Game\GameStates\AnalysisState.hpp"
#include "Engine\Renderer\Renderer.hpp"
#include "Engine\Window\Window.hpp"
#include "Engine\Core\StringUtils.hpp"

Rgba g_graphColorList[8] = { Rgba::RED, Rgba::GREEN, Rgba::BLUE, Rgba::YELLOW, Rgba::LIGHT_BLUE, Rgba::ORANGE, Rgba::PURPLE };

//  =========================================================================================
AnalysisGraph::AnalysisGraph()
{
	for (int dataIndex = 0; dataIndex < (int)m_simulationDataContents.size(); ++dataIndex)
	{
		m_simulationDataContents[dataIndex] = nullptr;
	}
}

//  =========================================================================================
AnalysisGraph::~AnalysisGraph()
{
}

//  =========================================================================================
void AnalysisGraph::GenerateGraph()
{
	if (m_simulationDataContents.size() == 0)
		return;

	m_minConfidenceValue = m_simulationDataContents[0]->m_confidenceIntervalRangeLow;
	m_maxConfidenceValue = m_simulationDataContents[0]->m_confidenceIntervalRangeHigh;

	//get min and max values from data
	for (int simDataIndex = 0; simDataIndex < (int)m_simulationDataContents.size(); ++simDataIndex)
	{
		if (m_simulationDataContents[simDataIndex]->m_confidenceIntervalRangeLow < m_minConfidenceValue)
			m_minConfidenceValue = m_simulationDataContents[simDataIndex]->m_confidenceIntervalRangeLow;

		if (m_simulationDataContents[simDataIndex]->m_confidenceIntervalRangeHigh > m_maxConfidenceValue)
			m_maxConfidenceValue = m_simulationDataContents[simDataIndex]->m_confidenceIntervalRangeHigh;
	}

	//anything else goes here
}

//  =========================================================================================
void AnalysisGraph::Render()
{
	Renderer* theRenderer = Renderer::GetInstance();
	Window* theWindow = Window::GetInstance();

	AABB2 boundsBox = AABB2(Vector2(theWindow->m_clientWidth * 0.1f, theWindow->m_clientHeight * 0.1f), Vector2(theWindow->m_clientWidth * 0.9f, theWindow->m_clientHeight * 0.9f));
	Vector2 boundsBoxDimensions = boundsBox.GetDimensions();

	//relative to graph box
	AABB2 titleBox = AABB2(Vector2(boundsBox.mins.x, boundsBox.maxs.y - (boundsBoxDimensions.y * 0.075f)), Vector2(boundsBox.maxs.x, boundsBox.maxs.y - (boundsBoxDimensions.y * 0.01f)));
	AABB2 verticalLegendBox = AABB2(Vector2(boundsBox.mins.x + (boundsBoxDimensions.x * 0.05f), boundsBox.mins.y + (boundsBoxDimensions.y * 0.01f)), Vector2(boundsBox.maxs.x, boundsBox.maxs.y - (boundsBoxDimensions.y * 0.01f)));
	AABB2 horizontalLegendBox = AABB2(Vector2(boundsBox.mins.x + (boundsBoxDimensions.x * 0.05f), boundsBox.mins.y + (boundsBoxDimensions.y * 0.01f)), Vector2(boundsBox.maxs.x - (boundsBoxDimensions.x * 0.05f), boundsBox.mins.y + (boundsBoxDimensions.y * 0.05f)));
	AABB2 graphBox = AABB2(Vector2(boundsBox.mins.x + (boundsBoxDimensions.x * 0.1f), boundsBox.mins.y + (boundsBoxDimensions.y * 0.15f)), Vector2(boundsBox.maxs.x - (boundsBoxDimensions.x * 0.05f), boundsBox.maxs.y - (boundsBoxDimensions.y * 0.1f)));
	
	//positions for graph box
	Vector2 graphBoxDimensions = graphBox.GetDimensions();
	Vector2 topLeftGraphPosition = graphBox.GetTopLeftPosition();
	Vector2 topRightGraphPosition = graphBox.GetTopRightPosition();
	Vector2 bottomLeftGraphPosition = graphBox.GetBottomLeftPosition();
	Vector2 bottomRightGraphPosition = graphBox.GetBottomRightPosition();

	//reset texture
	theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("default"));
	theRenderer->SetShader(theRenderer->m_defaultShader);
	theRenderer->m_defaultShader->EnableColorBlending(BLEND_OP_ADD, BLEND_SOURCE_ALPHA, BLEND_ONE_MINUS_SOURCE_ALPHA);

	//  ----------------------------------------------
	//draw graph space
	theRenderer->DrawAABB(boundsBox, Rgba(1.f, 1.f, 1.f, 1.f));
	//theRenderer->DrawAABB(titleBox, Rgba(1.f, 0.f, 0.f, 1.f));
	theRenderer->DrawAABB(graphBox, Rgba(1.f, 1.f, 1.f, 1.f));

	//draw graph lines
	theRenderer->DrawLineWithColor(topLeftGraphPosition, topRightGraphPosition, Rgba::BLACK);
	theRenderer->DrawLineWithColor(bottomLeftGraphPosition, bottomRightGraphPosition, Rgba::BLACK);
	theRenderer->DrawLineWithColor(bottomLeftGraphPosition, topLeftGraphPosition, Rgba::BLACK);
	theRenderer->DrawLineWithColor(bottomRightGraphPosition, topRightGraphPosition, Rgba::BLACK);

	//  ----------------------------------------------
	//draw horizontal graph lines (always 10 for now)
	float percentageSpacingBetweenHorizontalLines = 1.f/9.f;
	float confidencePercentage = (m_maxConfidenceValue - m_minConfidenceValue) * percentageSpacingBetweenHorizontalLines;

	//bottom -> top
	for (int dataContentIndex = 0; dataContentIndex < 9; ++dataContentIndex)
	{
		float yPosition = graphBox.mins.y + (graphBoxDimensions.y * percentageSpacingBetweenHorizontalLines * (float)dataContentIndex);
		Vector2 lineStart = Vector2(graphBox.mins.x, yPosition);
		Vector2 lineEnd = Vector2(graphBox.maxs.x, yPosition);
		theRenderer->DrawLineWithColor(lineStart, lineEnd, Rgba(0.2f, 0.2f, 0.2f, 0.5f));
	}

	//draw vertical legend header text
	Vector2 verticalLegendHeaderTextDrawPosition = Vector2(boundsBox.mins.x + (boundsBoxDimensions.x * 0.01f), graphBox.maxs.y + ((theWindow->m_clientHeight * 0.015f * 0.5f) * 2.f));
	theRenderer->DrawText2D(verticalLegendHeaderTextDrawPosition,
		Stringf("Seconds").c_str(),
		theWindow->m_clientHeight * 0.015f,
		Rgba::BLACK,
		1.f,
		Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));

	//bottom -> top
	for (int dataContentIndex = 0; dataContentIndex < 10; ++dataContentIndex)
	{	
		float yPosition = graphBox.mins.y + (graphBoxDimensions.y * percentageSpacingBetweenHorizontalLines * (float)dataContentIndex);

		//draw horizontal legend text
		Vector2 drawPosition = Vector2(boundsBox.mins.x + (boundsBoxDimensions.x * 0.01f), yPosition - ((theWindow->m_clientHeight * 0.015f * 0.5f)));
		theRenderer->DrawText2D(drawPosition,
			Stringf("%f", m_minConfidenceValue + (confidencePercentage * (float)dataContentIndex)).c_str(),
			theWindow->m_clientHeight * 0.015f,
			Rgba::BLACK,
			1.f,
			Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));
	}
	
	//  ----------------------------------------------
	//draw data points for confidence intervals vertically
	int simDataContentCount = (int)m_simulationDataContents.size();
	float percentageSpacingBetweenVerticalLines = 1.f / ((float)simDataContentCount + 1.f);

	for (int dataContentIndex = 0; dataContentIndex < simDataContentCount; ++dataContentIndex)
	{
		//reset texture
		theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("default"));
		theRenderer->SetShader(theRenderer->m_defaultShader);
		theRenderer->m_defaultShader->EnableColorBlending(BLEND_OP_ADD, BLEND_SOURCE_ALPHA, BLEND_ONE_MINUS_SOURCE_ALPHA);

		ImportedProfiledSimulationData* data = m_simulationDataContents[dataContentIndex];

		//get x position
		float xPosition = graphBox.mins.x + (graphBoxDimensions.x * percentageSpacingBetweenVerticalLines * ((float)dataContentIndex + 1.f));

		//get y range
		float lowYPercentage = RangeMapFloat(data->m_confidenceIntervalRangeLow, m_minConfidenceValue, m_maxConfidenceValue, 0.f, 1.f);
		float averageYPercentage = RangeMapFloat(data->m_average, m_minConfidenceValue, m_maxConfidenceValue, 0.f, 1.f);
		float highYPercentage = RangeMapFloat(data->m_confidenceIntervalRangeHigh, m_minConfidenceValue, m_maxConfidenceValue, 0.f, 1.f);

		Vector2 lineStart = Vector2(xPosition, graphBox.mins.y + (lowYPercentage * graphBoxDimensions.y));
		Vector2 lineEnd = Vector2(xPosition, graphBox.mins.y + (highYPercentage * graphBoxDimensions.y));
		Vector2 averagePoint = Vector2(xPosition, graphBox.mins.y + (averageYPercentage * graphBoxDimensions.y));

		//create point AABBS
		float pointRadiusY = theWindow->m_clientHeight * 0.0025f;
		float pointRadiusX = pointRadiusY * CLIENT_ASPECT;
		AABB2 lowConfidencePoint = AABB2(lineStart, pointRadiusX, pointRadiusY);
		AABB2 averageConfidencePoint = AABB2(averagePoint, pointRadiusX, pointRadiusY);
		AABB2 highConfidencePoint = AABB2(lineEnd, pointRadiusX, pointRadiusY);

		Rgba drawColor = g_graphColorList[dataContentIndex % 8];

		theRenderer->SetLineWidth(2.f);
		theRenderer->DrawLineWithColor(lineStart, lineEnd, g_graphColorList[dataContentIndex % 8]);
		theRenderer->SetLineWidth(1.f);

		theRenderer->DrawAABB(lowConfidencePoint, drawColor);
		theRenderer->DrawAABB(averageConfidencePoint, drawColor);
		theRenderer->DrawAABB(highConfidencePoint, drawColor);

		//draw text
		theRenderer->DrawText2DCentered(Vector2(xPosition, graphBox.mins.y - (boundsBoxDimensions.y * 0.1f * 0.5f)),
			Stringf("%s", m_simulationDataContents[dataContentIndex]->m_simulationDefinitionContentsNameKey.c_str()).c_str(),
			theWindow->m_clientHeight * 0.0125f,
			drawColor,
			1.f,
			Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));
	}


	//  ----------------------------------------------
	//draw title
	theRenderer->DrawText2DCentered(titleBox.GetCenter(),
		Stringf("%s", m_simulationDataContents[0]->m_profiledName.c_str()).c_str(),
		theWindow->m_clientHeight * 0.025f,
		Rgba::BLACK,
		1.f,
		Renderer::GetInstance()->CreateOrGetBitmapFont("SquirrelFixedFont"));
}

//  =========================================================================================
void AnalysisGraph::AddDataToGraph(ImportedProfiledSimulationData* data)
{
	m_simulationDataContents.push_back(data);
}
