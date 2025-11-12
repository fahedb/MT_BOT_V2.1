#include "StrategiesRunner.h"
#include "MTScannerUtils.h"
#include "TimeLineBuilder.h"

#include "InterfaceLayers_BKTSTExporter.h"

void StrategiesRunner::Run_PatternGod(TimeLine& historyTimeLine)
{
	Logger& logger = Logger::getInstance();
	Configrations* confs = Configrations::getInstance();
	std::thread tracer_thread(TradeTracer::RunTracer, &historyTimeLine);

	PatternGod patternScaner(&historyTimeLine); // Init a PatternGod object for patterns Scanning
	StatisticsDecession statisticsCalcucator(&historyTimeLine);

	XAUUSD_CurrentTable table;

	HttpServerSyncer::UpdateCurrentTimeLine(&table);
	ScoreCachingPolicy* scoreCachePolicy = confs->Get_Policy<ScoreCachingPolicy>(PolicyType::ScoreCachingPolicy);
	ConfigrationsPolicy* confPolicy = confs->Get_Policy<ConfigrationsPolicy>(PolicyType::ConfigrationsPolicy);
	scoreCachePolicy->ScoreResultsCacheSizePerThread = (historyTimeLine.Get_TimeLineLen() / confPolicy->ThreadsCount) * Static_CurrentTimeLineRep::getInstance()->Get_CurrTimeLine()->Get_TimeLineLen(); // Set the cache size per thread based on the history time line length and the number of threads
	confs->RefreshPolicy(); // Refresh all policies
	int scanNumber = 0;
	
	MTScannerUtils::InitPolicyExporters_BeforeScan(); // Init all exporters policies before starting the scan loop
	
	while (true)
	{
		HttpServerSyncer::UpdateCurrentTimeLine(&table);
		MTScannerUtils::ScoreResultsCacheCleanLogic(OneMinutePatternGodLogic(&patternScaner, &statisticsCalcucator, &historyTimeLine), m_scansCounter); // Start scanning
		
		scanNumber++;
		//if (scanNumber % 5 == 0) // Every 5 scans refresh the policies
			HttpServerSyncer::UpdateConfigs();
		//else if(scanNumber >= MAXINT)
		//	scanNumber = 0; // Reset scan number if exceeds max int
		HttpServerSyncer::Notify_DoneScanning();
		HttpServerSyncer::Set_IsScanning(false); // Set scanning to false after scan is done -- Set true in the precondition function

		HttpServerSyncer::Wait_ForNewBarM1(); // Wait until new M1 bar is available

		if (HttpServerSyncer::ShouldSubmitLayersExportersData())
			InterfaceLayers_BKTSTExporter::getInstance().Sumbmit_All_Exporters_Data(); // Submit all exporters data every 20 scans
	}
	tracer_thread.join();
}

bool StrategiesRunner::OneMinutePatternGodLogic(PatternGod* patternScan, StatisticsDecession* statisticsCalcucator, TimeLine* history)
{
	if (!ScanPreCondition())
		return false;

	std::vector<std::pair<ChangeNode*, ScoresResult>> results; // Vector for the End of every time period
	MTScannerUtils::InitScanObjectsBeforeScan(patternScan, statisticsCalcucator, m_lastTimePoint);
	TradesManager* tradeMngr = TradesManager::getInstance();

	Static_CurrentTimeLineRep* currTimeLineRep = Static_CurrentTimeLineRep::getInstance();
	std::vector<ChangeNode*>* currentTimeLine = currTimeLineRep->Get_CurrTimeLine()->Get_TimeLine();
	Logger& logger = Logger::getInstance();

	// Layer 1, This layer should score patterns from the history and get top X patterns
	if (patternScan->ScanForOneMinPatterns_MultiThreaded(results)) // Scan for patterns
	{
		InterfaceLayers_BKTSTExporter::getInstance().SendSummeryData_ScoringLayer(m_lastTimePoint, results); // Export the scoring data

		// Layer 2, This layer should filter out unwanted results
		patternScan->PolynomEngineFilter(results);
		InterfaceLayers_BKTSTExporter::getInstance().SendFilteredData_FilterLayer(m_lastTimePoint, results); // Export the filtered scoring data

		// Start building stats 
		logger.WriteLog("Start detecting common direction...", false, false);

		DirectionStats stats = statisticsCalcucator->DetectCommonDirection(&results);
		InterfaceLayers_BKTSTExporter::getInstance().SendStatisticsData_StatisticsLayer(&stats); // Export the direction stats data
		logger.WriteLog("Done detecting common direction.", false, false);
		// -----------------------------------------------------------------------------------------------------
		// -----------------------------------------------------------------------------------------------------

		// Layer 3, This layer should build the statistics data and export it
		std::vector<TimeLineScored> similarPatterns = TimeLineBuilder::Build_TimeLineScored_Patterns(history, &results, currentTimeLine->size(), Configrations::getInstance()->Get_Policy<PatternGodStatisticsPolicy>(PolicyType::PatternGodStatisticsPolicy)->Score_Future_Vision_Candles);
		stats.Add_Patterns(&similarPatterns, true);
		stats.Add_CurrPattern(TimeLine(new std::vector<ChangeNode*>(*currentTimeLine), true));

		// Start checking for petential trade through stats
		logger.WriteLog("Start Checking for a potentail trade...", false, false);
		// Layer 4, This layer should check for a potentail trade based on the stats generated
		OrderEntity* tradeRes = tradeMngr->CheckForTrade_PatternGod(stats);
		if (tradeRes && tradeRes->IsActive())
			InterfaceLayers_BKTSTExporter::getInstance().SendTradeData_TradeLayer(tradeRes); // Export the trade decession data

		DataExporter::Export_All_Statistics_Data(stats, tradeRes ? tradeRes->IsActive() : false);
		DirectionStatsCache::getInstance()->Add_NewStat(stats, true);
	}
	else
		logger.WriteLog("Didn't find any patterns with min score !!", false, true);

	logger.WriteLog("Done scanning and exporting data.");
	return true;

}

bool StrategiesRunner::ScanPreCondition()
{
	Logger& logger = Logger::getInstance();

	std::vector<ChangeNode*>* currentTimeLine = Static_CurrentTimeLineRep::getInstance()->Get_CurrTimeLine()->Get_TimeLine();

	if (!currentTimeLine || !currentTimeLine->size())
	{
		logger.WriteLog("No current time line.", false, true, LogType::ERR);
		logger.WriteLog("Sleeping for 5 minutes.", false, true, LogType::ERR);
		Sleep(1 * 60000);
		return false;
	}

	logger.WriteLog("current time line size: " + std::to_string(currentTimeLine->size()));
	if (currentTimeLine->at(currentTimeLine->size() - 1)->point_DateTime == m_lastTimePoint) // If the last time point scanned == to the current time point (start of the current vector) don't continue
	{
		logger.WriteLog("No new data, Last point time : " + m_lastTimePoint);
		return false;
	}

	ExchangeData exh;
	if (!exh.IsScanningAllowed())
	{
		logger.WriteLog("Scanning Stopped by user, skipping scanning...", false, true);
		return false;
	}

	m_lastTimePoint = currentTimeLine->at(currentTimeLine->size() - 1)->point_DateTime;

	HttpServerSyncer::Set_IsScanning(true);
	
	return true;
}
