#pragma once
#include "DirectionStats.h"
#include "CONFIGRATIONS.H"
#include "..\..\HTTPCore\ExchangeData.h"
#include "ChangeData.h"
#include "DBMap.h"
#include "TimeLine.h"
#include "PatternExporter.h"
#include "DataExporter.h"
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

	int DetectCommonDirection(std::vector<ChangeNode*>* potentioalRes)
	{
		if (!potentioalRes || potentioalRes->empty())
			return -1;

		ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();
		DirectionStats stats;
		stats.timePoint = m_currentTL->at(m_currentTL->size() - 1)->point_DateTime;

		for (ChangeNode* node : *potentioalRes)
		{
			int generalPipChange = 0;
			int firstPipChange = 0;

			CommonDirection generalDir = CalcGeneralPipsChange(node, &stats, policy->Pattern_Min_PipsChange_ToCount);
			CommonDirection firstDir = CalcFirstPipsChange(node, &stats, policy->Pattern_Min_PipsChange_ToCount);

		}
		bool decRes = Trade_Decession_Logic(stats);
		std::thread(DataExporter::Export_All_Statistics_Data, std::ref(stats), potentioalRes, decRes).detach();
		Send_For_PNGExport(potentioalRes);

		return -1;
	}

	void Set_NewCurrentPeriod(std::vector<ChangeNode*>* currentTimeLine) { m_currentTL = currentTimeLine; }
private:

	void Send_For_PNGExport(std::vector<ChangeNode*>* potentioalRes);
	CommonDirection CalcFirstPipsChange(ChangeNode* start, DirectionStats* stats, int pipsChangeToCount);
	CommonDirection CalcGeneralPipsChange(ChangeNode* start, DirectionStats* stats, int pipsChangeToCount);
	bool Trade_Decession_Logic(DirectionStats& stats);

	void Send_TraddeSignal(std::string pair, int type)
	{
		ExchangeData().Set_NewOrder("XAUUSD", type);
		m_orderCode++; Cur_Orders_InRow++;
	}

};

