#include "MetaTraderSimulator.h"
#include "Converts.h"
#include "Logger.h"

ActiveOrderInfo MetaTraderSimulator::ConvertOrderEntityToActiveOrderInfo(const OrderEntity* order)
{
	ActiveOrderInfo activeOrder;
	activeOrder.orderId = static_cast<int>(order->Get_OrderSeqNum());
	activeOrder.lotSize = m_lotForTrade; // Placeholder value, replace with actual lot size from OrderEntity if available
	activeOrder.openPrice = m_currentPrice; // Placeholder value, replace with actual open price from OrderEntity if available
	activeOrder.openTime = m_currentTimePoint; // Convert string time point to long long
	activeOrder.slPips = order->Get_StopLoss();
	activeOrder.tpPips = order->Get_TakeProfit();
	activeOrder.symbol = order->Get_PairType_str(); // Assuming symbol is derived from CoinsPair
	activeOrder.type = order->Get_OrderType();
	activeOrder.isActive = true; // New orders are active by default
	return activeOrder;
}

void MetaTraderSimulator::ProcessActiveOrders()
{
	// Logic to process each active order based on current bar data
	for(auto& order : m_activeOrders)
	{
		bool isClosed = CheckOrderClosureConditionsAndCreateDeal(order);
		order.isActive = !isClosed; // Mark order as inactive if closed
	}
	// Remove closed orders from active orders list
	RemoveClosedOrders();
}

bool MetaTraderSimulator::CheckOrderClosureConditionsAndCreateDeal(const ActiveOrderInfo& order)
{
	// Implementation for processing active orders in the simulation
	double m_highPrice = m_currentBarData->m_highPrice;
	double m_lowPrice = m_currentBarData->m_lowPrice;
	double m_openPrice = m_currentBarData->m_openPrice;
	double m_closePrice = m_currentBarData->m_closePrice;
	int pointPlace = static_cast<int>(m_currentBarData->GetPoints());
	double pointMultiplier = m_pointMultiplier;
	OrderType orderType = order.type;

	// Note: Positive tpPips indicates take profit above open price for LONG orders
	// and below open price for SHORT orders
	if(order.tpPips== 0 || order.slPips == 0)
	{
		Logger::getInstance().WriteLog("Order ID " + std::to_string(order.orderId) + " has invalid TP or SL values. Skipping closure check.", false, false, LogType::ERR);
		return false;
	}

	if(orderType == OrderType::LONG)
	{
		// Check LONG order closure conditions
		if(order.tpPips > 0)
		{
			double tpPrice = order.openPrice + (order.tpPips * pointMultiplier); // note: tpPips is positive
			if(m_highPrice >= tpPrice)
			{
				// Create deal for take profit hit
				CreateDealFromOrder(order, true);
				return true;
			}

			double slPrice = order.openPrice - (order.slPips * pointMultiplier); // note: slPips is positive
			if(m_lowPrice <= slPrice)
			{
				// Create deal for stop loss hit
				CreateDealFromOrder(order, false);
				return true;
			}
		}
	}
	else if(orderType == OrderType::SHORT)
	{
		// Check SHORT order closure conditions
		if(order.tpPips < 0)
		{
			double tpPrice = order.openPrice + (order.tpPips * pointMultiplier); // note: tpPips is negative
			if(m_lowPrice <= tpPrice)
			{
				// Create deal for take profit hit
				CreateDealFromOrder(order, true);
				return true;
			}

			double slPrice = order.openPrice + (order.slPips * pointMultiplier); // note: slPips is positive
			if(m_highPrice >= slPrice)
			{
				// Create deal for stop loss hit
				CreateDealFromOrder(order, false);
				return true;
			}
		}
	}
	return false; // Order remains active
}

void MetaTraderSimulator::CreateDealFromOrder(const ActiveOrderInfo& order, bool isTakeProfitHit)
{
	Deal deal;
	deal.symbol = order.symbol;
	deal.ticket = order.orderId; // Using orderId as ticket for simplicity
	deal.openTime = order.openTime;
	deal.closeTime = m_currentBarData->point_DateTimeLong; // Current time point from bar data
	deal.lotSize = order.lotSize;
	deal.openPrice = order.openPrice;
	deal.type = order.type;
	deal.stopLossHit = !isTakeProfitHit;
	deal.takeProfitHit = isTakeProfitHit;

	// Determine close price based on whether take profit or stop loss was hit
	if(isTakeProfitHit)
	{
		if(order.type == OrderType::LONG)
		{
			deal.closePrice = order.openPrice + (order.tpPips * m_pointMultiplier);
		}
		else if(order.type == OrderType::SHORT)
		{
			deal.closePrice = order.openPrice + (order.tpPips * m_pointMultiplier);
		}
	}
	else
	{
		if(order.type == OrderType::LONG)
		{
			deal.closePrice = order.openPrice - (order.slPips * m_pointMultiplier);
		}
		else if(order.type == OrderType::SHORT)
		{
			deal.closePrice = order.openPrice + (order.slPips * m_pointMultiplier);
		}
	}
	// Calculate profit and profit in pips
	if(order.type == OrderType::LONG)
	{
		deal.profit = (deal.closePrice - deal.openPrice) * deal.lotSize * m_contractSize;
		deal.profitInPips = static_cast<int>((deal.closePrice - deal.openPrice) / m_pointMultiplier);
	}
	else if(order.type == OrderType::SHORT)
	{
		deal.profit = (deal.openPrice - deal.closePrice) * deal.lotSize * m_contractSize; // Assuming standard lot size
		deal.profitInPips = static_cast<int>((deal.openPrice - deal.closePrice) / m_pointMultiplier);
	}
	m_closedDeals.push_back(deal);
	UpdateAcountBalance(deal);
}

void MetaTraderSimulator::RemoveClosedOrders()
{
	// Remove all orders that are not active
	m_activeOrders.erase(
		std::remove_if(m_activeOrders.begin(), m_activeOrders.end(),
			[](const ActiveOrderInfo& order) { return !order.isActive; }),
		m_activeOrders.end());
}

// Remove all isActive = false orders from m_activeOrders
void MetaTraderSimulator::UpdateAcountBalance(const Deal& deal) {
	m_acountBalance += deal.profit;
	m_totalProfitInPips += deal.profitInPips;
}

// Seters:
void MetaTraderSimulator::Set_NewOrder(OrderEntity* newOrder)
{
	if (!newOrder) return;

	if(!m_currentBarData)
	{
		Logger::getInstance().WriteLog("Current bar data is not set. Cannot add new order.", false, false, LogType::ERR);
		return;
	}

	// Implementation for setting a new order in the simulation
	ActiveOrderInfo activeOrder = ConvertOrderEntityToActiveOrderInfo(newOrder);
	m_activeOrders.push_back(activeOrder);

}

void MetaTraderSimulator::Set_NewCurrBarData(ChangeNode* newBarData)
{
	// Implementation for updating current bar data in the simulation
	m_currentBarData = newBarData;
	m_currentPrice = newBarData->m_closePrice;
	m_currentTimePoint = newBarData->point_DateTimeLong;
	
	if (!m_pointMultiplier) // If not set yet
	{
		int pointPlace = static_cast<int>(m_currentBarData->GetPoints());
		m_pointMultiplier = pow(10, -1 * pointPlace);
	}
	// After updating the current bar data, process active orders
	ProcessActiveOrders();
}
