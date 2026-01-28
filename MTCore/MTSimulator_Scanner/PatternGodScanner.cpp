#include "PatternGodScanner.h"

#include "TimeLineBuilder.h"
#include "Converts.h"
#include "InterfaceLayers_BKTSTExporter.h"
#include "InterfaceLayers_BKTSTImporter.h"
#include "TradeTracer.h"
#include "HttpServerSyncer.h"
#include "ScannerUtils.h"
#include "DataExporter.h"
OrderEntity* PatternGodScanner::Get_LastPlacedOrder()
{
	if (m_lastOrderGetPlaced) // If the last placed order was already retrieved, return nullptr to avoid duplicates
		return nullptr;

	m_lastOrderGetPlaced = true;
	return m_lastPlacedOrder.get();
}

void PatternGodScanner::Run_PatternGod(TimeLine& historyTimeLine)
{
	Logger& logger = Logger::getInstance();
	Configrations* confs = Configrations::getInstance();
	std::thread tracer_thread(TradeTracer::RunTracer, &historyTimeLine);

	PatternGod patternScaner(&historyTimeLine); // Init a PatternGod object for patterns Scanning
	StatisticsDecession statisticsCalcucator(&historyTimeLine);

	XAUUSD_CurrentTable table;

	UpdateCurrTimeLine(&table);
	ScoreCachingPolicy* scoreCachePolicy = confs->Get_Policy<ScoreCachingPolicy>(PolicyType::ScoreCachingPolicy);
	ConfigrationsPolicy* confPolicy = confs->Get_Policy<ConfigrationsPolicy>(PolicyType::ConfigrationsPolicy);
	scoreCachePolicy->ScoreResultsCacheSizePerThread = (historyTimeLine.Get_TimeLineLen() / confPolicy->ThreadsCount) * Static_CurrentTimeLineRep::getInstance()->Get_CurrTimeLine()->Get_TimeLineLen(); // Set the cache size per thread based on the history time line length and the number of threads
	confs->RefreshPolicy(); // Refresh all policies
	int scanNumber = 0;

	ScannerUtils::InitPolicyExporters_BeforeScan(); // Init all exporters policies before starting the scan loop
	ScannerUtils::InitLayersImporters_BeforeScan(historyTimeLine.Get_TimeLine(), historyTimeLine.Get_DBMap()); // Init all importers before starting the scan loop if needed

	while (true)
	{
		UpdateCurrTimeLine(&table);
		std::chrono::steady_clock::time_point scanStartTime = std::chrono::steady_clock::now();
		ScannerUtils::ScoreResultsCacheCleanLogic(OneMinutePatternGodLogic(&patternScaner, &statisticsCalcucator, &historyTimeLine), m_scansCounter); // Start scanning
		std::chrono::steady_clock::time_point scanEndTime = std::chrono::steady_clock::now();
		auto scanDuration = std::chrono::duration_cast<std::chrono::milliseconds>(scanEndTime - scanStartTime).count();
		logger.WriteLog("Total Scan Duration: " + std::to_string(scanDuration) + " ms", false, true, LogType::LOG);

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

bool PatternGodScanner::OneMinutePatternGodLogic(PatternGod* patternScan, StatisticsDecession* statisticsCalcucator, TimeLine* history)
{
	if (!ScanPreCondition())
		return false;

	std::vector<std::pair<ChangeNode*, ScoresResult>> results; // Vector for the End of every time period
	ScannerUtils::InitScanObjectsBeforeScan(patternScan, statisticsCalcucator, m_lastTimePoint);
	TradesManager* tradeMngr = TradesManager::getInstance();

	Static_CurrentTimeLineRep* currTimeLineRep = Static_CurrentTimeLineRep::getInstance();
	std::vector<ChangeNode*>* currentTimeLine = currTimeLineRep->Get_CurrTimeLine()->Get_TimeLine();
	Logger& logger = Logger::getInstance();

	// Layer 1, This layer should score patterns from the history and get top X patterns
	results = ScoringLayerRun(patternScan);

	// Layer 2, This layer should filter out unwanted results
	results = FilterLayerRun(patternScan, results);

	// Layer 3, This layer should build the statistics data and export it
	logger.WriteLog("Start detecting common direction...", false, false);
	m_lastStatisticsResult = StatisticsLayerRun(statisticsCalcucator, results, history, currentTimeLine);
	logger.WriteLog("Done detecting common direction.", false, false);

	// Start checking for petential trade through stats
	logger.WriteLog("Start Checking for a potentail trade...", false, false);
	// Layer 4, This layer should check for a potentail trade based on the stats generated
	m_lastPlacedOrder = std::move(TradeLayerRun(m_lastStatisticsResult));
	if(m_lastPlacedOrder)
		m_lastOrderGetPlaced = false;

	DataExporter::Export_All_Statistics_Data(m_lastStatisticsResult, m_lastPlacedOrder ? m_lastPlacedOrder->IsActive() : false);
	DirectionStatsCache::getInstance()->Add_NewStat(m_lastStatisticsResult, false);
	logger.WriteLog("Done scanning and exporting data.");
	return true;
}

bool PatternGodScanner::ScanPreCondition()
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

void PatternGodScanner::UpdateCurrTimeLine(BasePricesTable* table)
{
	if(m_externalCurrTimeLineQueue)
	{
		return; // If the current timeline is managed externally, do not update it here
	}
	else
	{
		HttpServerSyncer::UpdateCurrentTimeLine(table);
	}
}

std::vector<std::pair<ChangeNode*, ScoresResult>> PatternGodScanner::ScoringLayerRun(PatternGod* patternScan)
{

	std::vector<std::pair<ChangeNode*, ScoresResult>> results; // Vector for the End of every time period

	LayerType initializedLayer = GetInitializedImporterLayerType();
	if (static_cast<int>(initializedLayer) > static_cast<int>(LayerType::ScoringLayer))
		return results; // If the importer is initialized to a layer after this one, return empty results

	if (initializedLayer == LayerType::ScoringLayer)
	{
		results = Get_FilterLayer_ImporterData_NextItem(); // Clear the filter layer data from the importer queue
		return results; // If data is available from the importer, return it directly
	}

	// Layer 1, This layer should score patterns from the history and get top X patterns
	if (patternScan->ScanForOneMinPatterns_MultiThreaded(results)) // Scan for patterns
	{
		InterfaceLayers_BKTSTExporter::getInstance().SendSummeryData_ScoringLayer(m_lastTimePoint, results); // Export the scoring data
	}
	return results;
}

std::vector<std::pair<ChangeNode*, ScoresResult>> PatternGodScanner::FilterLayerRun(PatternGod* patternScan, std::vector<std::pair<ChangeNode*, ScoresResult>>& results)
{
	std::vector<std::pair<ChangeNode*, ScoresResult>> importedResults;
	LayerType initializedLayer = GetInitializedImporterLayerType();
	if (static_cast<int>(initializedLayer) > static_cast<int>(LayerType::FilterLayer))
		return importedResults; // If the importer is initialized to a layer after this one, return empty results

	if (initializedLayer == LayerType::FilterLayer)
	{
		importedResults = Get_FilterLayer_ImporterData_NextItem(); // Clear the filter layer data from the importer queue
		return importedResults; // If data is available from the importer, return it directly
	}

	// Layer 2, This layer should filter out unwanted results
	if (patternScan->PolynomEngineFilter(results))
	{
		InterfaceLayers_BKTSTExporter::getInstance().SendFilteredData_FilterLayer(m_lastTimePoint, results); // Export the filtered scoring data
	}
	return results;
}

DirectionStats PatternGodScanner::StatisticsLayerRun(StatisticsDecession* statisticsCalcucator, std::vector<std::pair<ChangeNode*, ScoresResult>> results, TimeLine* historyTL, std::vector<ChangeNode*>* currTL)
{ // Layer 3, This layer should build the statistics data and export it
	LayerType initializedLayer = GetInitializedImporterLayerType();

	if (static_cast<int>(initializedLayer) > static_cast<int>(LayerType::StatisticsLayer))
		return DirectionStats(); // If the importer is initialized to a layer after this one, return empty results
	if (initializedLayer == LayerType::StatisticsLayer)
	{
		DirectionStats importedStats = Get_StatisticsLayer_ImporterData_NextItem(); // Clear the statistics layer data from the importer queue
		return importedStats; // If data is available from the importer, return it directly
	}

	DirectionStats stats = statisticsCalcucator->DetectCommonDirection(&results);
	InterfaceLayers_BKTSTExporter::getInstance().SendStatisticsData_StatisticsLayer(&stats); // Export the direction stats data
	// Layer 3, This layer should build the statistics data and export it
	std::vector<TimeLineScored> similarPatterns = TimeLineBuilder::Build_TimeLineScored_Patterns(historyTL, &results, currTL->size(), Configrations::getInstance()->Get_Policy<PatternGodStatisticsPolicy>(PolicyType::PatternGodStatisticsPolicy)->Score_Future_Vision_Candles);
	stats.Add_Patterns(&similarPatterns, true);
	stats.Add_CurrPattern(TimeLine(new std::vector<ChangeNode*>(*currTL), true));

	return stats;
}

std::unique_ptr<OrderEntity> PatternGodScanner::TradeLayerRun(DirectionStats& stats)
{
	// Layer 4, This layer should check for a potentail trade based on the stats generated
	TradesManager* tradeMngr = TradesManager::getInstance();
	LayerType initializedLayer = GetInitializedImporterLayerType();
	//OrderEntity* tradeRes = nullptr;
	
	OrderEntity* tradeRes = nullptr;
	std::unique_ptr<OrderEntity> tradeRes_Imp = nullptr;

	if (initializedLayer == LayerType::TradeManagementLayer)
		tradeRes_Imp = Get_TradeLayer_ImporterData_NextItem();
	else
		tradeRes = tradeMngr->CheckForTrade_PatternGod(stats);

	if(tradeRes_Imp)
		tradeRes = tradeRes_Imp.get();

	if (tradeRes && tradeRes->IsActive())
		InterfaceLayers_BKTSTExporter::getInstance().SendTradeData_TradeLayer(tradeRes); // Export the trade decession data
	
	if (!tradeRes)
		return nullptr;

	return std::make_unique<OrderEntity>(*tradeRes);
}

std::vector<std::pair<ChangeNode*, ScoresResult>> PatternGodScanner::Get_ScoringLayer_ImporterData_NextItem()
{
	ScoringLayer_QueueItemType item = Get_MyQueuedItem<ScoringLayer_QueueItemType>();
	return item.second;
}

std::vector<std::pair<ChangeNode*, ScoresResult>> PatternGodScanner::Get_FilterLayer_ImporterData_NextItem()
{
	FilterLayer_QueueItemType item = Get_MyQueuedItem<FilterLayer_QueueItemType>();
	return item.second;
}

DirectionStats PatternGodScanner::Get_StatisticsLayer_ImporterData_NextItem()
{
	StatisticsLayer_QueueItemType item = Get_MyQueuedItem<StatisticsLayer_QueueItemType>();
	return item;
}

std::unique_ptr<OrderEntity> PatternGodScanner::Get_TradeLayer_ImporterData_NextItem()
{
	TradeLayer_QueueItemType item = Get_MyQueuedItem<TradeLayer_QueueItemType>();
	return std::move(item);
}

LayerType PatternGodScanner::GetInitializedImporterLayerType()
{
	InterfaceLayers_BKTSTImporter& importer = InterfaceLayers_BKTSTImporter::getInstance();
	return importer.Get_InitializedLayerType();
}


template<typename QueueItemType>
QueueItemType PatternGodScanner::Get_MyQueuedItem()
{
	InterfaceLayers_BKTSTImporter& importer = InterfaceLayers_BKTSTImporter::getInstance();
	if (!importer.IsImportedDataAvailable())
	{
		Logger::getInstance().WriteLog("PatternGodScanner::Get_ScoringLayer_ImporterData - Imported data is not available!", true, false, LogType::ERR);
		return {};
	}
	auto item(std::move(importer.Get_NextQueueItem<QueueItemType>()));
	long long dataTime = importer.Get_MyQueuedItem_TimePoint(item);
	long long currTime = Converts::ToLongLong(m_lastTimePoint);

	if (dataTime == currTime) // 20250602010000000 -- YYYYMMDDHHMMSSMMM
		return item;

	if (currTime > dataTime) // If the data time is less than the current time by 1 hour, loop till you get the right data
	{
		if (currTime - dataTime < 6000000)
		{

			while (dataTime < currTime)
			{
				Logger::getInstance().WriteLog("PatternGodScanner::Get_ScoringLayer_ImporterData - Skipping old data time: " + std::to_string(dataTime) + " Current time: " + std::to_string(currTime), true, false, LogType::ERR);
				item = std::move(importer.Get_NextQueueItem<QueueItemType>());
				dataTime = importer.Get_MyQueuedItem_TimePoint(item);
			}
			if (dataTime == currTime)
				return item;
		}
		else
		{
			Logger::getInstance().WriteLog("PatternGodScanner::Get_ScoringLayer_ImporterData - Data time: " + std::to_string(dataTime) + " is too old compared to current time: " + std::to_string(currTime) + ", skipping all old data.", true, false, LogType::ERR);
			return QueueItemType();
		}
	}
	else // If the data time is greater than the current time, log error and return empty
	{
		// Push back the item to the queue front !
		importer.Insert_BackToFront(std::move(item));
		Logger::getInstance().WriteLog("PatternGodScanner::Get_ScoringLayer_ImporterData - Data time: " + std::to_string(dataTime) + " is greater than current time: " + std::to_string(currTime) + ", The data inserted back to the front queue", true, false, LogType::ERR);
		return QueueItemType();
	}

}
