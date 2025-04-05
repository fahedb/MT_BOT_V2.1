#pragma once
#include "DirectionStats.h"
#include "CONFIGRATIONS.H"
#include "..\..\HTTPCore\ExchangeData.h"
#include "ChangeData.h"
#include "DBMap.h"
#include "TimeLine.h"
#include "PatternExporter.h"
#include "CSVExportData.h"

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

		for (ChangeNode* node : *potentioalRes)
		{
			int generalPipChange = 0;
			int firstPipChange = 0;

			CommonDirection generalDir = CalcGeneralPipsChange(node, &stats, policy->Pattern_Min_PipsChange_ToCount);
			CommonDirection firstDir = CalcFirstPipsChange(node, &stats, policy->Pattern_Min_PipsChange_ToCount);

		}
		bool decRes = Trade_Decession_Logic(stats);
		Export_AllData(stats,decRes);

		Send_For_PNGExport(potentioalRes);

		return -1;
	}

	void Set_NewCurrentPeriod(std::vector<ChangeNode*>* currentTimeLine) { m_currentTL = currentTimeLine; }
private:

	void Send_For_PNGExport(std::vector<ChangeNode*>* potentioalRes);

	void Export_DataToJson(DirectionStats& stats, bool tradeDecRes = false);

	void Export_AllData(DirectionStats& stats,bool tradeDecRes)
	{
		std::vector<std::string> summery_logs = PrepareSummeryLogs(stats); // Prepare all summery logs in vector

		for (int i = 0; i < summery_logs.size(); i++)
		{
			Logger::getInstance().WriteLog(summery_logs.at(i), false, true); // Send summery logs to logs
			if (tradeDecRes)Logger::getInstance().WriteLog(summery_logs.at(i), false, true, LogType::SPECIAL_LOG); // if there is a trade send, send also to SPECIAL log
		}

		Export_DataToJson(stats, false);
		if (tradeDecRes) Export_DataToJson(stats,true);

		if (tradeDecRes) Export_DataToCSV(stats);

	}
	std::vector<std::string> PrepareSummeryLogs(DirectionStats& stats);

	void Export_DataToCSV(DirectionStats& stats);

	CommonDirection CalcFirstPipsChange(ChangeNode* start, DirectionStats* stats, int pipsChangeToCount)
	{
		int indexStart = m_historyDB_Map->GetIdx_Of_TimePoint(start->point_DateTime);
		int futureLen = Configrations::getInstance()->Get_Policy()->Score_Future_Vision_Candles;
		int pipChange = 0;
		int candlesCounter = 0;
		bool upD = false, downD = false;
		for (int i = indexStart; i < indexStart + futureLen && i < m_historyTL->size(); i++)
		{
			pipChange += m_historyTL->at(i)->Pip_Change;
			candlesCounter++;
			if (pipChange >= pipsChangeToCount)
			{
				upD = true;
				break;

			}
			if (pipChange <= -pipsChangeToCount)
			{
				downD = true;
				break;
			}
		}
		CommonDirection dire;
		if (upD) dire = CommonDirection::UP;
		else if (downD) dire = CommonDirection::DOWN;
		else dire = CommonDirection::BLUR;

		stats->add(pipChange, dire, candlesCounter, StatsType::First);
		return dire;
	}


	CommonDirection CalcGeneralPipsChange(ChangeNode* start, DirectionStats* stats, int pipsChangeToCount)
	{
		int indexStart = m_historyDB_Map->GetIdx_Of_TimePoint(start->point_DateTime);
		int futureLen = Configrations::getInstance()->Get_Policy()->Score_Future_Vision_Candles;
		int pipChange = 0;
		int candlesCounter = 0;

		bool upD = false, downD = false;

		for (int i = indexStart; i < indexStart + futureLen && i < m_historyTL->size(); i++)
		{
			candlesCounter++;
			pipChange += m_historyTL->at(i)->Pip_Change;
			stats->addStep_ForCandleIdx(pipChange, candlesCounter - 1);
			if (pipChange >= pipsChangeToCount)
			{
				upD = true;
			}

			if (pipChange <= -pipsChangeToCount) {
				downD = true;
			}
		}

		CommonDirection dire;
		if (upD) dire = CommonDirection::UP;
		else if (downD) dire = CommonDirection::DOWN;
		else dire = CommonDirection::BLUR;

		stats->add(pipChange, dire, candlesCounter, StatsType::General);
		return dire;
	}


	bool Trade_Decession_Logic(DirectionStats& stats);

	void Send_TraddeSignal(std::string pair, int type)
	{
		ExchangeData().Set_NewOrder("XAUUSD", type);
		m_orderCode++; Cur_Orders_InRow++;
	}

};

