#include "PatternGod.h"
#include "StatisticsDecession.h"

#pragma once
class ScannerUtils
{
public:
	static void GoToSleep(int seconds);
	static void ScoreResultsCacheCleanLogic(bool isScanActivated,int& scansNum);
	static void InitScanObjectsBeforeScan(PatternGod* patternScan, StatisticsDecession* statisticsCalcucator, const std::string& lastTimePoint);
	static void InitPolicyExporters_BeforeScan();
	static void InitLayersImporters_BeforeScan(std::vector<ChangeNode*>* hisyoryTl, DBMap* histDBMap );
	static void ResetLayerExportersIfNeeded();
};

