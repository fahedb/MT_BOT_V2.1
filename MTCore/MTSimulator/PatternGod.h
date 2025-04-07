#pragma once

#include "ChangeData.h"
#include "DBMap.h"
#include <algorithm>
#include "DataBaseConnection.h"
#include <queue>

#include "TimeLine.h"

#define STAIRS_VISION 3

enum TimeWindow
{
	w_Year = 0,
	w_Month = 1,
	w_Day = 3
};


class PatternGod
{
public:
	std::vector<ChangeNode*>* m_historyTL;
	std::vector<ChangeNode*>* m_currentTL;

	PatternGod(std::vector<ChangeNode*>* historyTimeLine, std::vector<ChangeNode*>* currentTimeLine)
	{
		m_historyTL = historyTimeLine;
		m_currentTL = currentTimeLine;
		BuildDBHistoryMap();
	}
	PatternGod(std::vector<ChangeNode*>* historyTimeLine)
	{
		m_historyTL = historyTimeLine;
		BuildDBHistoryMap();
	}
	PatternGod(TimeLine* historyTLP)
	{
		m_currentTL = nullptr;
		m_historyTL = historyTLP->Get_TimeLine();
		m_historyDB_Map = historyTLP->Get_DBMap();
	}

	~PatternGod()
	{
		m_Db_Work.CloseConnection();
	}
	void filterByHighScores(std::vector<std::pair< ChangeNode*, ScoresResult>>& localResults2D);
	void SortScores(std::vector<std::pair< ChangeNode*, ScoresResult>>& localResults2D);
	void FilterDuplicatesTimeLines(std::vector<std::pair< ChangeNode*, ScoresResult>>& localResults2D);
	void Set_NewCurrentPeriod(std::vector<ChangeNode*>* currentTimeLine) { m_currentTL = currentTimeLine; }
	inline ScoresResult CompareOneMinNodesPeriods(int startIdx);
	int ScanForOneMinPatterns_MultiThreaded(std::vector<ChangeNode*>* resultNodes);
private:
	std::string GetFormattedCurrentTime();
	inline ScoresResult CheckForOneMinNodeMatch(ChangeNode* historyNode, ChangeNode* currentNode); 
	void BuildDBHistoryMap();
	bool Time_Check(TimeWindow);
	
	DataBaseConnection m_Db_Work;
	DBMap* m_historyDB_Map;

};

