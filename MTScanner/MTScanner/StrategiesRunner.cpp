#include "StrategiesRunner.h"
#include "MTScannerUtils.h"
#include "TimeLineBuilder.h"

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

	while (true)
	{
		HttpServerSyncer::UpdateCurrentTimeLine(&table);
		MTScannerUtils::ScoreResultsCacheCleanLogic(OneMinutePatternGodLogic(&patternScaner, &statisticsCalcucator, &historyTimeLine), m_scansCounter); // Start scanning
		HttpServerSyncer::UpdateConfigs();
		HttpServerSyncer::Set_IsScanning(false); // Set scanning to false after scan is done -- Set true in the precondition function

		MTScannerUtils::GoToSleep(1);
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

	if (patternScan->ScanForOneMinPatterns_MultiThreaded(results)) // Scan for patterns
	{
		// Start building stats 
		logger.WriteLog("Start detecting common direction...", false, false);

		std::vector<ChangeNode*> headsOfPotentialRes = TimeLineBuilder::SplitChangeNodeFromScores(&results);
		DirectionStats stats = statisticsCalcucator->DetectCommonDirection(&headsOfPotentialRes);

		logger.WriteLog("Done detecting common direction.", false, false);
		// -----------------------------------------------------------------------------------------------------

		// Start checking for petential trade through stats
		logger.WriteLog("Start Checking for a potentail trade...", false, false);
		OrderEntity* tradeRes = tradeMngr->CheckForTrade_PatternGod(stats);
		// -----------------------------------------------------------------------------------------------------

		std::vector<TimeLineScored> similarPatterns = TimeLineBuilder::Build_TimeLineScored_Patterns(history, &results, currentTimeLine->size(), Configrations::getInstance()->Get_Policy<PatternGodStatisticsPolicy>(PolicyType::PatternGodStatisticsPolicy)->Score_Future_Vision_Candles);
		stats.Add_Patterns(&similarPatterns, true);
		stats.Add_CurrPattern(TimeLine(new std::vector<ChangeNode*>(*currentTimeLine), true));

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
		std::cout << "\n\nNo new data, Last point time: " << m_lastTimePoint << std::endl;
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
