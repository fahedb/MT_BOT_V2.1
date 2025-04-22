#include <string>
#include "CoinsPair.h"
#include "OrderType.h"
#include "SeqNumTable.h"
#include "Static_DBConnection.h"

#pragma once
class OrderEntity
{
	long m_order_seqNum;
	OrderType m_orderType;
	OrderStatus m_status;

	std::string m_timePoint;
	std::string m_statisticsTimePoint;
	CoinsPair m_pair;
	bool m_active;
public:
	OrderEntity()
	{
		m_order_seqNum = 0;
		m_active = false;
		m_pair = CoinsPair::NONE;
		m_orderType = OrderType::NONE;
		m_status = OrderStatus::NONE;
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
	void CloseOrder() { m_status = OrderStatus::CLOSED; }
};

