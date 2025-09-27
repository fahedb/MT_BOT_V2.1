#include "OrderStruct.h"
#include <vector>

#pragma once
class OrderEntityCache
{
	std::mutex orderRepLocker;
	std::vector<OrderEntity*> m_ordersRep;
	
public:
	void Add_NewOrder(OrderEntity* order) { 
		
		if (!order)
			return;

		std::lock_guard<std::mutex> lock(orderRepLocker);
		m_ordersRep.push_back(order); 
	}

	int Get_OrdersCount() { return m_ordersRep.size(); }

	OrderEntity* Get_Order_byIdx(int idx)
	{
		if (m_ordersRep.size() > idx)
			return m_ordersRep.at(idx);
		return nullptr;
	}

	OrderEntity* Get_Order(long seqNum)
	{
		for (int i = 0; i < m_ordersRep.size(); i++)
		{
			if (m_ordersRep.at(i)->Get_OrderSeqNum() == seqNum)
				return m_ordersRep.at(i);
		}
		return nullptr;
	}

	OrderEntity* Get_Order(const std::string& timePoint)
	{
		for (int i = 0; i < m_ordersRep.size(); i++)
			if (m_ordersRep.at(i)->Get_TimePoint() == timePoint)
				return m_ordersRep.at(i);
		return nullptr;
	}

	void RefreshOrders()
	{
		std::lock_guard<std::mutex> lock(orderRepLocker);
		for (int i = static_cast<int>(m_ordersRep.size()) - 1; i >= 0; --i)
			if (m_ordersRep[i]->Get_OrderStatus() == OrderStatus::CLOSED)
			{
				OrderEntity* ord = m_ordersRep[i];
				m_ordersRep.erase(m_ordersRep.begin() + i);
				delete ord;
			}
	}
	static OrderEntityCache* getInstance() {
		static OrderEntityCache instance; // Guaranteed to be destroyed and instantiated on first use
		return &instance;
	}

private:
	OrderEntityCache() {}
	OrderEntityCache(const OrderEntityCache&) = delete;
	OrderEntityCache& operator=(const OrderEntityCache&) = delete;

};

