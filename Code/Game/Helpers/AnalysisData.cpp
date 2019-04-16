#include "Game\Helpers\AnalysisData.hpp"
#include "Game\SimulationData.hpp"
#include "Engine\Time\Time.hpp"
#include "Engine\Core\StringUtils.hpp"



//  =========================================================================================
AnalysisData::AnalysisData(SimulationData* exportDataReferene, int iterationsBeforeLog)
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
}

//  =========================================================================================
void AnalysisData::Start()
{
	++m_currentIterationCount;

	m_startHPC = GetPerformanceCounter();
}

//  =========================================================================================
void AnalysisData::End()
{
	//update count
	++m_data->m_count;

	//check to see if we are at capacity. If so we can skip adding cells
	if (m_data->IsAtCapacity())
		return;

	//we can skip cumulative average if iterations before log is only 1
	if (m_iterationsBeforeLog == 1)
	{
		uint64_t endHPC = GetPerformanceCounter();
		m_data->AddCell(Stringf("%f", (float)PerformanceCounterToSeconds(endHPC - m_startHPC)), true);
	}
	else
	{
		uint64_t totalHPC = GetPerformanceCounter() - m_startHPC;
		m_timeAverage = ((m_timeAverage * (m_currentIterationCount - 1)) + totalHPC) / m_currentIterationCount;
		if (m_currentIterationCount == m_iterationsBeforeLog)
		{
			float secondsAverage = (float)PerformanceCounterToSeconds(m_timeAverage);
			m_data->AddCell(Stringf("%f", secondsAverage), true);
		}
	}

	//reset data
	Reset();
}

//  =========================================================================================
void AnalysisData::FullReset()
{
	m_currentIterationCount = 0;
	m_timeAverage = 0.0;
	m_startHPC = 0.0;
	m_iterationStartHPC = 0.0;
}
