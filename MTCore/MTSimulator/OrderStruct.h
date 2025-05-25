#include <string>
#include "CoinsPair.h"
#include "OrderType.h"
#include "SeqNumTable.h"
#include "Static_DBConnection.h"

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

	void ActivateOrder(OrderType long_short, std::string timePoint, std::string statisticsTimePoint, CoinsPair pair);
	OrderEntity(OrderType long_short, std::string timePoint, std::string statisticsTimePoint, CoinsPair pair) { ActivateOrder(long_short, timePoint, statisticsTimePoint, pair); }

	long Get_OrderSeqNum() const { return m_order_seqNum; }
	OrderType Get_OrderType() const { return m_orderType; }

	std::string Get_TimePoint() const { return m_timePoint; }
	std::string Get_StatisticsTimePoint() const { return m_statisticsTimePoint; }
	CoinsPair Get_PairType() const { return m_pair; }

	bool IsActive() const { return m_active; }
	OrderStatus Get_OrderStatus() { return m_status; }

	void Set_StopLoss(int sl_pips) { m_sl_pips = sl_pips; }
	void Set_TakeProfit(int tp_pips) { m_tp_pips = tp_pips; }

	int Get_StopLoss() { return m_sl_pips; }
	int Get_TakeProfit() { return m_tp_pips; }


	void CloseOrder() {
		std::lock_guard<std::mutex> lock(m_orderLocker);
		m_status = OrderStatus::CLOSED; 
	}

};

