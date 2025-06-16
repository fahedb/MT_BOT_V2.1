#include "LOGGER.H"
#include "XAUUSD_OneMinTable.h"
#include "XAUUSD_CurrentTable.h"
#include "TimeLine.h"
#include "TradeTracer.h"
#include "ScoreResultsCache.h"
#include "HistoryTimeLineLoader.h"
#include "TradeTracer.h"
#include "DataExporter.h"
#include "HttpServerSyncer.h"

#include <future>


#pragma once
class StrategiesRunner
{
public:
	StrategiesRunner() : m_scansCounter(0) { }

	void Run()
	{
		HttpServerSyncer::LoadAllConfigsToHttpCore();

		std::vector<ChangeNode*> history;
		HistoryTimeLineLoader historyLoader(history);
		historyLoader.Run();
		TimeLine historyTimeLine(&history);
		Run_PatternGod(historyTimeLine);

	}
private:
	std::string m_lastTimePoint;
	int m_scansCounter;

	void Run_PatternGod(TimeLine& historyTimeLine);
	bool OneMinutePatternGodLogic(PatternGod* patternScan, StatisticsDecession* statisticsCalcucator, TimeLine* history);
	bool ScanPreCondition(); // Precondition function before start scanning
};

