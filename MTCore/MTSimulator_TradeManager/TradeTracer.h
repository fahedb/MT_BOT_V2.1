#include "CONFIGRATIONS.H"
#include "Logger.h"

#include "TradesManager.h"
#include "PatternGod.h"
#include "Static_CurrentTimeLineRep.h"
#include "StatisticsDecession.h"
#include "DirectionStatsCache.h"
#include "TimeLine.h"

enum class TradeTracingType
{
	DynamicTracing,
	ConstTracing,
	NONE
};

#pragma once
class TradeTracer
{
public:
	static void RunTracer(TimeLine* historyTL)
	{
		return; // temp disable trade tracing

		if (!historyTL)
		{
			Logger::getInstance().WriteLog("No valid history time line for TradeTracing", false, true, LogType::SPECIAL_LOG);
			return;
		}

		TradeTracer* tracer = new TradeTracer();
		tracer->TraceTrades(historyTL);
	}
	void TraceTrades(TimeLine* historyTL);
private:
	PatternGod* m_patternScanner;
	StatisticsDecession* m_statisticsEngine;

	ScoringLayerPolicy* m_confPolicy;
	PatternGodStatisticsPolicy* m_statsPolicy;
	OrderTracerPolicy* m_tradeTracingPolicy;

	TradesManager* m_trader;
	TimeLine* m_historyTL;

	DirectionStats* GetRelativeStats(OrderEntity* order, TradeTracingType forceTradeTracing = TradeTracingType::NONE);


private:
	void ActOnOrder(OrderEntity* order,DirectionStats& stats)
	{
		if (!order )
		{
			Logger::getInstance().WriteLog("No valid order for TradeTracing", false, true, LogType::SPECIAL_LOG);
			return;
		}

		if (!stats.IsActive())
		{
			Logger::getInstance().WriteLog("Stats For order " + std::to_string(order->Get_OrderSeqNum()) + " is not active\nClosing Order !!", false, true, LogType::TRADE_TRACE_LOG);
			order->CloseOrder();
			return;
		}

		CommonDirection newDirection = m_trader->CalcCommonDirection_Pure(stats);
		CommonDirection desiredDirection = order->Get_OrderType() == OrderType::LONG ? CommonDirection::UP : CommonDirection::DOWN;

		if (desiredDirection == newDirection)
		{
			Logger::getInstance().WriteLog("Order " + std::to_string(order->Get_OrderSeqNum()) + " is in the right direction, Keeping it open", false, true, LogType::TRADE_TRACE_LOG);
		}
		else
		{
			std::string desiredDirStr = CommonDirection_Utils::GetCommonDirectionStr(desiredDirection);
			std::string newDirStr = CommonDirection_Utils::GetCommonDirectionStr(newDirection);
			Logger::getInstance().WriteLog("Order " + std::to_string(order->Get_OrderSeqNum()) + " is in the wrong direction, Closing it\n"
				+ "Desired direction: " + desiredDirStr + "\nDirection found: " + newDirStr, false, true, LogType::TRADE_TRACE_LOG);

			order->CloseOrder();
		}
	}

	DirectionStats* DynamicTradeTracing(OrderEntity* order);
	DirectionStats* ConstTradeTracing(OrderEntity* order) 
	{
		DirectionStats* stats = DirectionStatsCache::getInstance()->Get_LastStats();
		if(!stats)
			return nullptr; // If no stats found, return empty stats
		return stats;
	}

};

