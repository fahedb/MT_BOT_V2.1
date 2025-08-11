#pragma once

#include "..\..\HTTPCore\ExchangeData.h"
#include "CONFIGRATIONS.H"
#include "DirectionStats.h"
#include "OrderStruct.h"
#include "OrdersStructCache.h"
/// <summary>
/// This Class is responsible for send a trades 
/// Though DirectionStats object that we get from StatisticsDecession Class
/// </summary>

class TradesManager
{
private:
	int m_orderCode;
	int Cur_Orders_InRow;
	OrderEntityCache* m_ordersCache;

	TradeManagerPolicy* m_tradeMngrPolicy;
	OrdersPolicy* m_ordersPolicy;

public:
    // Deleted copy constructor and assignment operator to prevent copies
    TradesManager(const TradesManager&) = delete;
    TradesManager& operator=(const TradesManager&) = delete;

    // Static method to get the instance of the Singleton
    static TradesManager* getInstance() {
        static TradesManager instance; // Guaranteed to be destroyed and instantiated on first use
        return &instance;
    }

public:
	OrderEntity* Get_Order_ByIdx(int idx) { return m_ordersCache->Get_Order_byIdx(idx);}
	OrderEntity* Get_Order(long seq) { return m_ordersCache->Get_Order(seq); }
	OrderEntity* Get_Order(std::string& timePoint) { return m_ordersCache->Get_Order(timePoint); }
	void Refresh_OrdersCache() { m_ordersCache->RefreshOrders(); }
	int Get_OrdersCount() { return m_ordersCache->Get_OrdersCount(); }
	
	OrderEntity* CheckForTrade_PatternGod(DirectionStats& stats);
	CommonDirection CalcCommonDirection(DirectionStats& stats);
	CommonDirection CalcCommonDirection_Pure(DirectionStats& stats);

	void Send_TraddeSignal(std::string pair, int type, int seqNum, int stopLoss, int takeProfit)
	{
		ExchangeData().Set_NewOrder(pair, type, seqNum, stopLoss, takeProfit);
		m_orderCode++; Cur_Orders_InRow++;
	}
private: 

	CommonDirection CheckDirection(DirectionStats& stats,StatsType stype);
	CommonDirection CheckDirection_Pure(DirectionStats& stats, StatsType stype);

private:
    TradesManager()
    {
		m_tradeMngrPolicy = Configrations::getInstance()->Get_Policy<TradeManagerPolicy>(PolicyType::TradeManagerPolicy);
		m_ordersPolicy = Configrations::getInstance()->Get_Policy<OrdersPolicy>(PolicyType::OrdersPolicy);
		Cur_Orders_InRow = 0;
		m_orderCode = 0; // TODO: Need to emplement a logic to get orderCode from seqnum table
		m_ordersCache = OrderEntityCache::getInstance();
    }
	~TradesManager() {}

};

