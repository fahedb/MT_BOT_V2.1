#include "CONFIGRATIONS.H"
#include "Logger.h"

#include "TradesManager.h"
#include "PatternGod.h"
#include "Static_CurrentTimeLineRep.h"
#include "StatisticsDecession.h"
#include "DirectionStatsCache.h"
#include "ScoreCalcs.h"
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
		if (!historyTL)
		{
			Logger::getInstance().WriteLog("No valid history time line for TradeTracing", false, true, LogType::SPECIAL_LOG);
			return;
		}

		TradeTracer* tracer = new TradeTracer();
		tracer->TraceTrades(historyTL);
	}
private:
	PatternGod* m_patternScanner;
	StatisticsDecession* m_statisticsEngine;

	ConfigrationsPolicy* m_confPolicy;
	PatternGodStatisticsPolicy* m_statsPolicy;
	OrderTracerPolicy* m_tradeTracingPolicy;

	TradesManager* m_trader;
	TimeLine* m_historyTL;

	DirectionStats* GetRelativeStats(OrderEntity* order, TradeTracingType forceTradeTracing = TradeTracingType::NONE);

	void TraceTrades(TimeLine* historyTL)
	{
		m_confPolicy = Configrations::getInstance()->Get_Policy<ConfigrationsPolicy>(PolicyType::ConfigrationsPolicy);
		m_statsPolicy = Configrations::getInstance()->Get_Policy<PatternGodStatisticsPolicy>(PolicyType::PatternGodStatisticsPolicy);
		m_tradeTracingPolicy = Configrations::getInstance()->Get_Policy<OrderTracerPolicy>(PolicyType::OrderTracerPolicy);

		m_trader = TradesManager::getInstance();
		m_historyTL = historyTL;
		m_statisticsEngine = new StatisticsDecession(historyTL);
		m_patternScanner = new PatternGod(historyTL);

		while (true)
		{
			Sleep(m_confPolicy->TimeBetweenScans * 1000);
			if (!m_tradeTracingPolicy->EnableTradeTracing)
			{
				Logger::getInstance().WriteLog("Trade tracing is disabled, sleeping for 3 minutes", false, true, LogType::TRADE_TRACE_LOG);
				Sleep(180 * 1000);
				continue;
			}

			int ordersCount = m_trader->Get_OrdersCount();
			for (int i = 0; i < ordersCount; i++)
			{
				OrderEntity* order = m_trader->Get_Order_ByIdx(i);
				if(!order || order->Get_OrderStatus() == OrderStatus::CLOSED)
				{
					Logger::getInstance().WriteLog("Invalid order found in TradeTracing, skipping", false, true, LogType::ERR);
					continue; // Skip invalid orders
				}

				DirectionStats* stats = GetRelativeStats(order);
				if (!stats || !stats->IsActive())
				{
					Logger::getInstance().WriteLog("No valid stats for order " + std::to_string(order->Get_OrderSeqNum()), false, true, LogType::ERR);
					continue; // If no stats found, skip this order
				}

				if(stats->Get_TimePoint() == order->Get_LastStatisticsCheckTimePoint())
					continue; // If the stats are not updated, skip this order
				order->Set_LastStatisticsCheckTimePoint(stats->Get_TimePoint());
				ActOnOrder(order, *stats);
				
				if (order->Get_OrderStatus() == OrderStatus::CLOSED)
					m_trader->Refresh_OrdersCache(); // Refresh the orders cache after closing an order)
			}
		}
	}

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

