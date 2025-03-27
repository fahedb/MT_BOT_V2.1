#pragma once

#include "ChangeData.h"
#include "DBMap.h"
#include <algorithm>
#include "DataBaseConnection.h"
#include <queue>
#define STAIRS_VISION 3

enum class CommonDirection
{
	UP =0,
	DOWN = 1,
	BLUR = 3
};

enum TimeWindow
{
	w_Year = 0,
	w_Month = 1,
	w_Day = 3
};

struct DirectionStats {
	int generalTotal = 0;
	int generalUpCount = 0;
	int generalDownCount = 0;
	int generalBlurCount = 0;
	double generalSumPips = 0.0;
	double generalSumSquaredPips = 0.0;

	int firstTotal = 0;
	int firstUpCount = 0;
	int firstDownCount = 0;
	int firstBlurCount = 0;
	double firstSumPips = 0.0;
	double firstSumSquaredPips = 0.0;


	void add_general(double pipChange, CommonDirection dir) {
		generalTotal++;
		generalSumPips += pipChange;
		generalSumSquaredPips += pipChange * pipChange;

		switch (dir) {
		case CommonDirection::UP: generalUpCount++; break;
		case CommonDirection::DOWN: generalDownCount++; break;
		case CommonDirection::BLUR: generalBlurCount++; break;
		}
	}

	double mean_general() const {
		return (generalTotal == 0) ? 0.0 : generalSumPips / generalTotal;
	}

	double stddev_general() const {
		if (generalTotal == 0) return 0.0;
		double avg = mean_general();
		return std::sqrt((generalSumSquaredPips / generalTotal) - (avg * avg));
	}


	void add_first(double pipChange, CommonDirection dir) {
		firstTotal++;
		firstSumPips += pipChange;
		firstSumSquaredPips += pipChange * pipChange;

		switch (dir)
		{
			case CommonDirection::UP: firstUpCount++; break;
			case CommonDirection::DOWN: firstDownCount++; break;
			case CommonDirection::BLUR: firstBlurCount++; break;
		}
	}

	double mean_first() const {
		return (firstTotal == 0) ? 0.0 : firstSumPips / firstTotal;
	}

	double stddev_first() const {
		if (firstTotal == 0) return 0.0;
		double avg = mean_first();
		return std::sqrt((firstSumSquaredPips / firstTotal) - (avg * avg));
	}

};


class PatternGod
{
public:
	std::vector<ChangeNode*>* m_historyTL;
	std::vector<ChangeNode*>* m_currentTL;
	int Cur_Orders_InRow; // How many orders opened in a row

	PatternGod(std::vector<ChangeNode*>* historyTimeLine, std::vector<ChangeNode*>* currentTimeLine)
	{
		m_historyTL = historyTimeLine;
		m_currentTL = currentTimeLine;
		m_orderCode = 100;
		BuildDBHistoryMap();
	}
	PatternGod(std::vector<ChangeNode*>* historyTimeLine)
	{
		m_historyTL = historyTimeLine;
		BuildDBHistoryMap();
	}
	~PatternGod()
	{
		m_Db_Work.CloseConnection();
	}
	void filterByHighScores(std::vector<std::pair< ChangeNode*, ScoresResult>>& localResults2D);
	void SortScores(std::vector<std::pair< ChangeNode*, ScoresResult>>& localResults2D);
	void FilterDuplicatesTimeLines(std::vector<std::pair< ChangeNode*, ScoresResult>>& localResults2D);
	void Set_NewCurrentPeriod(std::vector<ChangeNode*>* currentTimeLine) { m_currentTL = currentTimeLine; }
	ScoresResult CompareOneMinNodesPeriods(int startIdx);
	int ScanForOneMinPatters(std::vector<ChangeNode*>*);
	int ScanForOneMinPatterns_MultiThreaded(std::vector<ChangeNode*>* resultNodes);
	int DetectCommonDirection(std::vector<ChangeNode*>* potentioalRes);
	CommonDirection CalcGeneralPipsChange(ChangeNode* start, int pipsChangeToCount, int* outPipChange);
	CommonDirection CalcFirstPipsChange(ChangeNode* start, int pipsChangeToCount, int* outPipChange);
private:
	void Send_TraddeSignal(std::string pair, int type);
	bool Trade_Decession_Logic(DirectionStats stats);
	void Send_For_PNGExport(std::vector<ChangeNode*>* potentioalRes);
	std::string GetFormattedCurrentTime();
	ScoresResult CheckForOneMinNodeMatch(ChangeNode* historyNode, ChangeNode* currentNode); 
	void BuildDBHistoryMap();
	bool Time_Check(TimeWindow);
	DataBaseConnection m_Db_Work;
	DBMap m_historyDB_Map;
	int m_orderCode;

};

