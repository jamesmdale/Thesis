#pragma once
#include "Engine\Core\EngineCommon.hpp"

class SimulationData;

class AnalysisData
{
public:
	AnalysisData(SimulationData* exportDataReference, int iterationsBeforeLog);
	~AnalysisData();

	void Reset();
	void Start();
	void End();

	void FullReset();

public:
	int m_currentIterationCount = 0;
	int m_iterationsBeforeLog = 1;
	uint64_t m_timeAverage = 0.0;
	uint64_t m_startHPC = 0.0;
	uint64_t m_iterationStartHPC = 0.0;	

	SimulationData* m_data = nullptr;
};


