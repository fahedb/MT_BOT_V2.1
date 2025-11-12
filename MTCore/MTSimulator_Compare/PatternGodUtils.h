#pragma once
#include "TimeLine.h"
//#include <string>

class PatternGodUtils
{
public:
	static void SortScores(std::vector<std::pair< ChangeNode*, ScoresResult>>& nodesResults2D);
	static void FilterDuplicatesTimeLines(std::vector<std::pair<ChangeNode*, ScoresResult>>& nodesResults2D, DBMap* historyDB_Map);
	static void filterByHighScores(std::vector<std::pair< ChangeNode*, ScoresResult>>& nodesResults2D, int numOfTopResultToFilterOut);
	static void filterByThresholdScore(std::vector<std::pair< ChangeNode*, ScoresResult>>& nodesResults2D, double thresholdPercent);
	static std::string BuildScanResultLog(const std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D);

};

