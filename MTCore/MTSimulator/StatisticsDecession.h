#pragma once
#include "DirectionStats.h"
#include "CONFIGRATIONS.H"
#include "..\..\HTTPCore\ExchangeData.h"
#include "ChangeData.h"
#include "DBMap.h"
#include <thread>
class StatisticsDecession
{
	int m_orderCode;
	int Cur_Orders_InRow;
	DBMap* m_historyDB_Map;
	std::vector<ChangeNode*>* m_historyTL;
	std::vector<ChangeNode*>* m_currentTL;
public:
	StatisticsDecession(std::vector<ChangeNode*>* historyTL ,DBMap* historyDB_Map)
	{
		m_currentTL = nullptr;
		m_historyTL = historyTL;
		m_historyDB_Map = historyDB_Map;
		Cur_Orders_InRow = 0;
		m_orderCode = 0; // Need an logic to implement from DB
	}
	StatisticsDecession(TimeLine* history)
	{
		m_historyDB_Map = history->Get_DBMap();
		m_historyTL = history->Get_TimeLine();
		m_currentTL = nullptr;
		Cur_Orders_InRow = 0;
		m_orderCode = 0; // Need an logic to implement from DB
	}

	DirectionStats DetectCommonDirection(std::vector<ChangeNode*>* potentioalRes) // TODO: Need to change the logic here to TimeLine
	{
		DirectionStats stats;

		if (!potentioalRes || potentioalRes->empty())
			return stats;

		ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();
		stats.timePoint = m_currentTL->at(m_currentTL->size() - 1)->point_DateTime;

		for (ChangeNode* node : *potentioalRes)
		{
			int generalPipChange = 0;
			int firstPipChange = 0;

			CommonDirection generalDir = CalcGeneralPipsChange(node, &stats);
			CommonDirection firstDir = CalcFirstPipsChange(node, &stats);
			CommonDirection HighestDir = CalcHighestPipsChange(node, &stats);

		}
		
		return stats;
	}

	void Set_NewCurrentPeriod(std::vector<ChangeNode*>* currentTimeLine) { m_currentTL = currentTimeLine; }
private:
	CommonDirection CalcFirstPipsChange(ChangeNode* start, DirectionStats* stats);
	CommonDirection CalcGeneralPipsChange(ChangeNode* start, DirectionStats* stats);
	CommonDirection CalcHighestPipsChange(ChangeNode* start, DirectionStats* stats);

};

