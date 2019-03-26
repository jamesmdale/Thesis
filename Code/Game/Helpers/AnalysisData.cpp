#include "Game\Helpers\AnalysisData.hpp"
#include "Game\SimulationData.hpp"
#include "Engine\Time\Time.hpp"
#include "Engine\Core\StringUtils.hpp"



//  =========================================================================================
AnalysisData::AnalysisData(SimulationData* exportDataReferene, uint iterationsBeforeLog)
{
	m_data = exportDataReferene;
	m_iterationsBeforeLog = iterationsBeforeLog;
}

//  =========================================================================================
AnalysisData::~AnalysisData()
{
	m_data = nullptr;
}

//  =========================================================================================
void AnalysisData::Reset()
{
	m_iterationStartHPC = GetPerformanceCounter();
	m_currentIterationCount = 0;
	m_timeAverage = 0.0;
	m_currentIterationCount = 0;
}

//  =========================================================================================
void AnalysisData::Start()
{
	++m_totalNumCalls;
	++m_currentIterationCount;

	m_startHPC = GetPerformanceCounter();
}

//  =========================================================================================
void AnalysisData::End()
{
	uint64_t totalHPC = GetPerformanceCounter() - m_startHPC;

	m_timeAverage = ((m_timeAverage * (m_currentIterationCount - 1)) + totalHPC) / m_currentIterationCount;
	if (m_currentIterationCount == m_iterationsBeforeLog)
	{
		float totalSeconds = (float)PerformanceCounterToSeconds(GetPerformanceCounter() - m_iterationStartHPC);
		float iterationsPerSecond = totalSeconds / 100.f;

		float secondsAverage = (float)PerformanceCounterToSeconds(m_timeAverage);
		//DevConsolePrintf(Rgba::GREEN, "Average Time After 100 iterations (UpdatePlan) %f", secondsAverage);
		//DevConsolePrintf(Rgba::GREEN, "Iterations per second %f (UpdatePlan) (total time %f)", iterationsPerSecond, totalSeconds);

		m_data->AddCell(Stringf("%f", secondsAverage), true);

		//g_generalSimulationData->WriteEntryWithTimeStamp(Stringf("Iterations per second %f (UpdatePlan) (total time between: %f)", iterationsPerSecond, totalSeconds));

		//reset data
		Reset();
	}
}