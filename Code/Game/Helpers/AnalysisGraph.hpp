#pragma once
#include "Engine\Math\AABB2.hpp"
#include "Engine\Core\Rgba.hpp"
#include <vector>

extern Rgba g_graphColorList[8];

class ImportedProfiledSimulationData;

class AnalysisGraph
{
public:
	AnalysisGraph();
	~AnalysisGraph();

	void GenerateGraph();

	void Render();
	void AddDataToGraph(ImportedProfiledSimulationData* data);

	inline int GetDataCount() { return (int)m_simuldationDataContents.size(); }

private:
	float m_minConfidenceValue = 0.f;
	float m_maxConfidenceValue = 0.f;

	std::vector<ImportedProfiledSimulationData*> m_simuldationDataContents;
};