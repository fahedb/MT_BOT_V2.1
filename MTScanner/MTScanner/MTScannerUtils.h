#include "ScoreResultsCache.h"
#include "PatternGod.h"
#include "StatisticsDecession.h"

#pragma once
class MTScannerUtils
{
public:
	static void GoToSleep(int seconds);
	static void ScoreResultsCacheCleanLogic(bool isScanActivated,int& scansNum);
	static void InitScanObjectsBeforeScan(PatternGod* patternScan, StatisticsDecession* statisticsCalcucator, const std::string& lastTimePoint);

};

