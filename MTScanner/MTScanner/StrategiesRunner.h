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
#include "LOGGER.H"
#include <future>

#include "PolynomTimeLine.h"
#include "PolynomTimeLineCache.h"

/// <summary>	
/// StrategiesRunner class for running
/// trading strategies on historical data.
/// It loads configurations, retrieves historical data,
/// and executes the PatternGod strategy on the data.
/// </summary>
/// Author: Fahed Elbatash
/// Date: 2025-07-12
/// /// This class is responsible for initializing the trading environment,
/// loading historical data, and executing the PatternGod strategy.


#pragma once
class StrategiesRunner
{
public:
	StrategiesRunner() : m_scansCounter(0) { }

	void Run()
	{
		HttpServerSyncer::LoadAllConfigsToHttpCore();

		std::vector<ChangeNode*> history;
		XAUUSD_OneMinTable table(Configrations::getInstance()->Get_DB_FileDir());
		HistoryTimeLineLoader historyLoader(history, &table);
		historyLoader.Run();
		
		// Test logic:
		//auto m = PolynomChangeNodePackageBuilder::BuildPolynomForChangeNodes(std::vector<ChangeNode*>(history.begin(), history.begin() + 5));
		{
			std::unique_ptr<PolynomTimeLine> tl = std::make_unique<PolynomTimeLine>(std::vector<ChangeNode*>(history.begin(), history.begin() + 5));

			size_t size = sizeof(tl);
			//////////////////////////
			Polynom* a = tl->Get_PolynomPackage()->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L15, IndicatorValueType::Value));
			Polynom* b = tl->Get_PolynomPackage()->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L15, IndicatorValueType::Slope));

			PolynomTimeLineCache::getInstance()->InsertPolynomTimeLine(std::move(tl));
		}
		PolynomTimeLine* tl2 = PolynomTimeLineCache::getInstance()->GetPolynomTimeLine(PolynomTimeLineKey(history.at(0)->point_DateTimeLong, history.at(4)->point_DateTimeLong));
		Polynom* a = tl2->Get_PolynomPackage()->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L15, IndicatorValueType::Value));
		Polynom* b = tl2->Get_PolynomPackage()->GetPolynom(PolynomKey(IndicatorType::RSI, IndicatorLen::L15, IndicatorValueType::Slope));
		Logger::getInstance().WriteLog("About to run PatternGod !!", true, true, LogType::SPECIAL_LOG);
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

