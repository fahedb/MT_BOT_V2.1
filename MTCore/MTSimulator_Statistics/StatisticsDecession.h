#pragma once
#include "DirectionStats.h"
#include "..\..\HTTPCore\ExchangeData.h"
#include "ChangeData.h"
#include "DBMap.h"
#include <thread>
#include "BaseVote.h"

class StatisticsDecession
{
	int m_orderCode;
	int Cur_Orders_InRow;
	DBMap* m_historyDB_Map;
	BaseVote* m_voter = nullptr;
	std::vector<ChangeNode*>* m_historyTL;
	std::vector<ChangeNode*>* m_currentTL;
public:
	StatisticsDecession(std::vector<ChangeNode*>* historyTL ,DBMap* historyDB_Map);
	StatisticsDecession(TimeLine* history);

	DirectionStats DetectCommonDirection(std::vector<std::pair<ChangeNode*, ScoresResult>>* potentioalRes,int futureLen =0);

	void Set_NewCurrentPeriod(std::vector<ChangeNode*>* currentTimeLine) { m_currentTL = currentTimeLine; }
private:
	void CalcPipsChange(ChangeNode* start, DirectionStats* stats, int futureLen);
	CommonDirection CalcFirstPipsChange  (ChangeNode* start,double weight, DirectionStats* stats,int futureLen);
	CommonDirection CalcGeneralPipsChange(ChangeNode* start,double weight, DirectionStats* stats,int futureLen);
	CommonDirection CalcHighestPipsChange(ChangeNode* start,double weight, DirectionStats* stats,int futureLen);

};

