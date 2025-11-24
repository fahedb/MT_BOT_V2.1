#include <string>
#include "OrderType.h"
#include "..\MTSimulator_TimeLines\CoinsPair.h"
#include <mutex>

#pragma once
/// <summary>
/// Class for orders entities
/// Please not that this object class can't be copied becuase of mutex,
/// only move or use pointers
/// </summary>
class OrderEntity
{
	long m_order_seqNum;
	OrderType m_orderType;
	OrderStatus m_status;

	std::string m_timePoint;
	std::string m_statisticsTimePoint;
	std::string m_lastStatisticsCheckTimePoint; // The last time point that we checked the statistics for this order
	CoinsPair m_pair;
	bool m_active;
	std::mutex m_orderLocker;

	int m_sl_pips;
	int m_tp_pips;
public:
	OrderEntity()
	{
		m_order_seqNum = 0;
		m_active = false;
		m_pair = CoinsPair::NONE;
		m_orderType = OrderType::NONE;
		m_status = OrderStatus::NONE;
		m_sl_pips = 0;
		m_tp_pips = 0;
	}

	OrderEntity(const OrderEntity& other) {
		m_order_seqNum = other.m_order_seqNum;
		m_orderType = other.m_orderType;
		m_status = other.m_status;
		m_timePoint = other.m_timePoint;
		m_statisticsTimePoint = other.m_statisticsTimePoint;
		m_lastStatisticsCheckTimePoint = other.m_lastStatisticsCheckTimePoint;
		m_pair = other.m_pair;
		m_active = other.m_active;
		m_sl_pips = other.m_sl_pips;
		m_tp_pips = other.m_tp_pips;
	}

	OrderEntity(OrderEntity&& other) noexcept
		: m_order_seqNum(other.m_order_seqNum),
		m_orderType(other.m_orderType),
		m_status(other.m_status),
		m_timePoint(std::move(other.m_timePoint)),
		m_statisticsTimePoint(std::move(other.m_statisticsTimePoint)),
		m_lastStatisticsCheckTimePoint(std::move(other.m_lastStatisticsCheckTimePoint)),
		m_pair(other.m_pair),
		m_active(other.m_active),
		m_sl_pips(other.m_sl_pips),
		m_tp_pips(other.m_tp_pips)
	{
		// No need to copy the mutex, it will be moved
	}

	void ActivateOrder(OrderType long_short, std::string timePoint, std::string statisticsTimePoint, CoinsPair pair);
	OrderEntity(OrderType long_short, std::string timePoint, std::string statisticsTimePoint, CoinsPair pair) { ActivateOrder(long_short, timePoint, statisticsTimePoint, pair); }
	OrderEntity(long seqNum, OrderType orderType, std::string timePoint, std::string statisticsTimePoint, CoinsPair pair, int tp, int sl) // Full constructor
		: m_order_seqNum(seqNum),
		m_orderType(orderType),
		m_status(OrderStatus::NONE),
		m_timePoint(timePoint),
		m_statisticsTimePoint(statisticsTimePoint),
		m_lastStatisticsCheckTimePoint(""),
		m_pair(pair),
		m_active(false),
		m_tp_pips(tp),
		m_sl_pips(sl)
	{
	}
	long Get_OrderSeqNum() const { return m_order_seqNum; }
	OrderType Get_OrderType() const { return m_orderType; }
	
	std::string Get_OrderType_str() { return OrderTypeUtils::Get_OrderType_String(m_orderType); }

	std::string Get_TimePoint() const { return m_timePoint; }
	std::string Get_StatisticsTimePoint() const { return m_statisticsTimePoint; }
	std::string Get_LastStatisticsCheckTimePoint() const { return m_lastStatisticsCheckTimePoint; }

	CoinsPair Get_PairType() const { return m_pair; }
	std::string Get_PairType_str() const { return CoinsPairUtils::Get_PairType_str(m_pair); }

	bool IsActive() const { return m_active; }
	OrderStatus Get_OrderStatus() { return m_status; }
	std::string Get_OrderStatus_str() { return OrderStatusUtils::Get_OrderStatus_String(m_status); }

	void Set_SeqNum(long seqNum) { m_order_seqNum = seqNum; }
	void Set_OrderType(OrderType type) { m_orderType = type; }
	void Set_StopLoss(int sl_pips) { m_sl_pips = sl_pips; }
	void Set_TakeProfit(int tp_pips) { m_tp_pips = tp_pips; }
	void Set_LastStatisticsCheckTimePoint(std::string timePoint) {
		std::lock_guard<std::mutex> lock(m_orderLocker);
		m_lastStatisticsCheckTimePoint = timePoint;
	}
	int Get_StopLoss() const { return m_sl_pips; }
	int Get_TakeProfit() const { return m_tp_pips; }


	void CloseOrder() {
		std::lock_guard<std::mutex> lock(m_orderLocker);
		m_status = OrderStatus::CLOSED; 
	}

};

