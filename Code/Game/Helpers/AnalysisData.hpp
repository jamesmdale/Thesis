#pragma once
#include "Engine\Core\EngineCommon.hpp"

class SimulationData;

class AnalysisData
{
public:
	AnalysisData(SimulationData* exportDataReference, uint iterationsBeforeLog);
	~AnalysisData();

	void Reset();
	void Start();
	void End();

public:
	int m_currentIterationCount = 0;
	uint m_iterationsBeforeLog = 0;
	uint64_t m_timeAverage = 0.0;
	uint64_t m_startHPC = 0.0;
	uint64_t m_iterationStartHPC = 0.0;	

	uint m_totalNumCalls = 0;
	SimulationData* m_data = nullptr;
};

