#include "ScannerUtils.h"
#include "ScoreCalcs.h"
#include "LOGGER.H"

#include "PolynomTimeLineCache.h"
#include "ScoreResultsCachePolynom.h"
#include "ScoreResultsCache.h"
#include "InterfaceLayers_BKTSTExporter.h"
#include "InterfaceLayers_BKTSTImporter.h"

#include "Policies.h"
#include "HttpServerSyncer.h"



void ScannerUtils::GoToSleep(int seconds)
{
	Logger::getInstance().WriteLog("Sleep for " + std::to_string(seconds) + " seconds...", false, false);
	Sleep(seconds * 1000); // Sleep for 5 seconds before start a new scan
}

void ScannerUtils::ScoreResultsCacheCleanLogic(bool isScanActivated, int& scansNum)
{
	
	ScoreResultsCache* scoreResultsCache = ScoreResultsCache::getInstance();
	if (isScanActivated)
	{
		std::vector<size_t> sizes = scoreResultsCache->GetMyCacheSizes();
		std::string cacheSizes = "Score Results Cache Sizes per Thread:\n";

		for (size_t i = 0; i < sizes.size(); i++)
		{
			cacheSizes += "Thread [" + std::to_string(i) + "] : ";
			cacheSizes += std::to_string(sizes[i]);
			cacheSizes += "\n";
		}

		Logger::getInstance().WriteLog("Cache Status before clean:\n " + cacheSizes, false, true, LogType::LOG);
		scansNum++; // Increase the scans counter if a scan was activated
	}
	if (Configrations::getInstance()->Get_Policy<ScoreCachingPolicy>(PolicyType::ScoreCachingPolicy)->CleanEveryXScans < scansNum)
	{
		scoreResultsCache->ClearOldResults(false);
		ScoreResultsCachePolynom::getInstance()->ClearOldResults(false);
		PolynomTimeLineCache::getInstance()->ClearOldResults(false);
		scansNum = 0; // Reset the scans counter after cleaning
	}
}

void ScannerUtils::InitScanObjectsBeforeScan(PatternGod* patternScan, StatisticsDecession* statisticsCalcucator, const std::string& lastTimePoint)
{
	Static_CurrentTimeLineRep* currTimeLineRep = Static_CurrentTimeLineRep::getInstance();
	std::vector<ChangeNode*>* currentTimeLine = currTimeLineRep->Get_CurrTimeLine()->Get_TimeLine();

	Logger& logger = Logger::getInstance();
	std::string startScanLog = std::string("!!!!!!!!!!!!		Start New Period Scan		!!!!!!!!!\n\n") +
		std::string("Current TimePoint: ") + lastTimePoint + "\n" +
		"Scanning period bars count: " + std::to_string(currentTimeLine->size());
	logger.WriteLog(startScanLog, true, true);

	patternScan->Set_CurrentTimeLine(currentTimeLine); // Set a new Current Period for PatternScan object for the new scan
	patternScan->RefreshComparers(); // Refresh the comparers to use the new current time line
	statisticsCalcucator->Set_NewCurrentPeriod(currentTimeLine); // Set a new current period for Statistics calc

	if (currTimeLineRep->Get_CurrTimeLine()->Get_TimeLineLen()) // Getting the max score possible for 2 nodes match
		Configrations::getInstance()->Get_Policy<ScoreCachingPolicy>(PolicyType::ScoreCachingPolicy)->maxScorePossibleForChangeNodes = ProxyScoreCalcs::CalcMaxScorePossible(currTimeLineRep->Get_CurrTimeLine()->Get_TimeLine()->at(0)).totalScore.total;
}

void ScannerUtils::InitPolicyExporters_BeforeScan()
{
	InterfaceLayers_BKTSTExporter& layerExporter = InterfaceLayers_BKTSTExporter::getInstance();
	Configrations* confs = Configrations::getInstance();

	// Layer 1 Policies initialization
	layerExporter.SendPolicyData(confs->Get_Policy<ScoringLayerPolicy>(PolicyType::ScoringLayerPolicy), LayerType::ScoringLayer);
	layerExporter.SendPolicyData(confs->Get_Policy<ScoreComparerPolicy>(PolicyType::ScoreComparerPolicy), LayerType::ScoringLayer);

	// Layer 2 Policies initialization
	layerExporter.SendPolicyData(confs->Get_Policy<FilterLayerPolicy>(PolicyType::FilterLayerPolicy), LayerType::FilterLayer);
	layerExporter.SendPolicyData(confs->Get_Policy<PolynomComparerPolicy>(PolicyType::PolynomComparerPolicy), LayerType::FilterLayer);

	// Layer 3 Policies initialization
	layerExporter.SendPolicyData(confs->Get_Policy<PatternGodStatisticsPolicy>(PolicyType::PatternGodStatisticsPolicy), LayerType::StatisticsLayer);

	// Layer 4 Policies initialization
	layerExporter.SendPolicyData(confs->Get_Policy<TradeManagerPolicy>(PolicyType::TradeManagerPolicy), LayerType::TradeManagementLayer);
	layerExporter.SendPolicyData(confs->Get_Policy<OrdersPolicy>(PolicyType::OrdersPolicy), LayerType::TradeManagementLayer);


}

void ScannerUtils::InitLayersImporters_BeforeScan(std::vector<ChangeNode*>* hisyoryTl, DBMap* histDBMap)
{
	InterfaceLayers_BKTSTImporter& layerImporter = InterfaceLayers_BKTSTImporter::getInstance();
	layerImporter.Set_HisyoryTL(hisyoryTl, histDBMap);
	layerImporter.InitImporters();
	bool isDataAvailable = layerImporter.IsImportedDataAvailable();
	Logger::getInstance().WriteLog("ScannerUtils::InitLayersImporters_BeforeScan - Importers initialized. Is data available: " + std::string(isDataAvailable ? "Yes" : "No"), false, true, LogType::LOG);
}

void ScannerUtils::ResetLayerExportersIfNeeded()
{
	InterfaceLayers_BKTSTExporter& layerExporter = InterfaceLayers_BKTSTExporter::getInstance();
	std::string bktstName = HttpServerSyncer::GetNewBKTSTName();

	if (!bktstName.empty())
	{
		layerExporter.ResetExporters(bktstName); // Reset all exporters if a new backtest name is provided
		InitPolicyExporters_BeforeScan();
	}
}
