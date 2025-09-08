#include "MTScannerUtils.h"
#include "ScoreCalcs.h"

void MTScannerUtils::GoToSleep(int seconds)
{
	Logger::getInstance().WriteLog("Sleep for " + std::to_string(seconds) + " seconds...", false, false);
	Sleep(seconds * 1000); // Sleep for 5 seconds before start a new scan
}

void MTScannerUtils::ScoreResultsCacheCleanLogic(bool isScanActivated, int& scansNum)
{
	
	ScoreResultsCache* scoreResultsCache = ScoreResultsCache::getInstance();
	if (isScanActivated)
	{
		std::vector<size_t> sizes = scoreResultsCache->GetCacheSizes();
		std::string cacheSizes = "Score Results Cache Sizes per Thread:\n";

		for (size_t i = 0; i < sizes.size(); i++)
		{
			cacheSizes += "Thread [" + std::to_string(i) + "] : ";
			cacheSizes += std::to_string(sizes[i]);
			cacheSizes += "\n";
		}

		Logger::getInstance().WriteLog("Cache Status before clean:\n " + cacheSizes, false, false, LogType::LOG);
		scansNum++; // Increase the scans counter if a scan was activated
	}
	if (Configrations::getInstance()->Get_Policy<ScoreCachingPolicy>(PolicyType::ScoreCachingPolicy)->CleanEveryXScans < scansNum)
	{
		scoreResultsCache->CleanOldResults();
		ScoreResultsCachePolynom::getInstance()->CleanOldResults();
		scansNum = 0; // Reset the scans counter after cleaning
	}
}

void MTScannerUtils::InitScanObjectsBeforeScan(PatternGod* patternScan, StatisticsDecession* statisticsCalcucator, const std::string& lastTimePoint)
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
