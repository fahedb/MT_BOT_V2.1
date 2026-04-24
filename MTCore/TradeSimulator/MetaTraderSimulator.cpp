#include "MetaTraderSimulator.h"
#include "Converts.h"
#include "Logger.h"
#include "MTExportTagsDef.h"
#include "TimeLineBuilder.h"

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

void MetaTraderSimulator::CalcFloatingPnL()
{
	m_floatingPnL = 0.0;
	for(const auto& order : m_activeOrders)
	{
		double priceDifference = 0.0;
		if(order.type == OrderType::LONG)
		{
			priceDifference = m_currentPrice - order.openPrice;
		}
		else if(order.type == OrderType::SHORT)
		{
			priceDifference = order.openPrice - m_currentPrice;
		}
		double pnl = priceDifference * order.lotSize * m_contractSize;
		m_floatingPnL += pnl;
	}
	m_equityBalance = m_acountBalance + m_floatingPnL;
}

void MetaTraderSimulator::ProcessActiveOrders()
{
	// Logic to process each active order based on current bar data
	for(auto& order : m_activeOrders)
	{
		order.numOfBarsHeld += 1; // Increment the number of bars the order has been held
		
		bool isClosed = CheckOrderClosureConditionsAndCreateDeal(order);
		order.isActive = !isClosed; // Mark order as inactive if closed
	}
	// Remove closed orders from active orders list
	RemoveClosedOrders();
}

void MetaTraderSimulator::CalcUsedMargin()
{
	m_usedMargin = 0.0;
	const double marginPerLot = (m_currentPrice * m_contractSize) / m_leverage;
	for(const auto& order : m_activeOrders)
	{
		m_usedMargin += order.lotSize * marginPerLot;
	}
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
		if(order.tpPips > 0 && order.slPips > 0)
		{
			double slPrice = order.openPrice - (order.slPips * pointMultiplier); // note: slPips is positive
			if(m_lowPrice <= slPrice)
			{
				// Create deal for stop loss hit
				CreateDealFromOrder(order, ClosureReason::STOP_LOSS_HIT);
				return true;
			}

			double tpPrice = order.openPrice + (order.tpPips * pointMultiplier); // note: tpPips is positive
			if(m_highPrice >= tpPrice)
			{
				// Create deal for take profit hit
				CreateDealFromOrder(order, ClosureReason::TAKE_PROFIT_HIT);
				return true;
			}
		}
		else
			Logger::getInstance().WriteLog("Order ID " + std::to_string(order.orderId) + " has invalid TP or SL values for LONG order. Skipping closure check.", false, false, LogType::ERR);
	}
	else if(orderType == OrderType::SHORT)
	{
		// Check SHORT order closure conditions
		if(order.tpPips > 0 && order.slPips > 0)
		{
			double slPrice = order.openPrice + (order.slPips * pointMultiplier); // note: slPips is positive
			if(m_highPrice >= slPrice)
			{
				// Create deal for stop loss hit
				CreateDealFromOrder(order, ClosureReason::STOP_LOSS_HIT);
				return true;
			}

			double tpPrice = order.openPrice - (order.tpPips * pointMultiplier); // note: tpPips is positive
			if(m_lowPrice <= tpPrice)
			{
				// Create deal for take profit hit
				CreateDealFromOrder(order, ClosureReason::TAKE_PROFIT_HIT);
				return true;
			}

		}
		else
			Logger::getInstance().WriteLog("Order ID " + std::to_string(order.orderId) + " has invalid TP or SL values for SHORT order. Skipping closure check.", false, false, LogType::ERR);
	}
	return false; // Order remains active
}

void MetaTraderSimulator::CreateDealFromOrder(const ActiveOrderInfo& order, ClosureReason closeReason)
{
	Deal deal;
	deal.symbol = order.symbol;
	deal.ticket = order.orderId; // Using orderId as ticket for simplicity
	deal.openTime = order.openTime;
	deal.closeTime = m_currentBarData->point_DateTimeLong; // Current time point from bar data
	deal.lotSize = order.lotSize;
	deal.openPrice = order.openPrice;
	deal.numOfBarsHeld = order.numOfBarsHeld;

	deal.stopLossPrice = order.type == OrderType::LONG ?
		order.openPrice - (order.slPips * m_pointMultiplier) :
		order.openPrice + (order.slPips * m_pointMultiplier);

	deal.takeProfitPrice = order.type == OrderType::LONG ?
		order.openPrice + (order.tpPips * m_pointMultiplier) :
		order.openPrice - (order.tpPips * m_pointMultiplier);

	deal.type = order.type;

	deal.closeReason = closeReason;

	switch (closeReason)
	{
	case ClosureReason::TAKE_PROFIT_HIT:
		deal.closePrice = deal.takeProfitPrice;
		break;
	case ClosureReason::STOP_LOSS_HIT:
		deal.closePrice = deal.stopLossPrice;
		break;

	default:
		deal.closePrice = m_currentPrice;
		break;
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
	m_dataExporter->ExportCurrentSimulationState(
		m_acountBalance,
		m_equityBalance,
		m_usedMargin,
		Get_FreeMargin(),
		m_floatingPnL,
		deal.profit,
		deal);
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
void MetaTraderSimulator::Set_NewOrder(OrderEntity* newOrder, DirectionStats* stats)
{
	if (!m_currentBarData)
	{
		Logger::getInstance().WriteLog("Current bar data is not set. Cannot add new order.", false, false, LogType::ERR);
		return;
	}

	m_dataExporter->ExportDirectionStatsData(stats);
	if (!newOrder) return;

	// Implementation for setting a new order in the simulation
	ActiveOrderInfo activeOrder = ConvertOrderEntityToActiveOrderInfo(newOrder);
	CalcFloatingPnL(); // Recalculate floating PnL before adding new order
	CalcUsedMargin(); // Recalculate used margin before adding new order
	
	double freeMargin = Get_FreeMargin();
	double requiredMargin = RequiredMargin(m_currentPrice, m_contractSize, activeOrder.lotSize, m_leverage);

	if (freeMargin < requiredMargin)
	{
		Logger::getInstance().WriteLog("Insufficient margin to place new order. Free Margin: " + std::to_string(freeMargin) + ", Required Margin: " + std::to_string(requiredMargin), false, false, LogType::ERR);
		return;
	}

	m_activeOrders.push_back(activeOrder);

}

void MetaTraderSimulator::Set_NewCurrBarData(ChangeNode* newBarData)
{
	// Implementation for updating current bar data in the simulation
	m_currentBarData = newBarData;
	m_currentPrice = newBarData->m_closePrice;
	m_currentTimePoint = newBarData->point_DateTimeLong;
	
	m_currDataVector.push_back(newBarData);
	if(m_currDataVector.size() > 30) // Keep only last 20 bars
	{
		m_currDataVector.erase(m_currDataVector.begin());
	}

	// Recalculate floating PnL based on new current price
	CalcFloatingPnL();
	// Recalculate used margin 
	CalcUsedMargin();
	// Check for margin stop out conditions
	MarginStopOutCheck();

	if (!m_pointMultiplier) // If not set yet
	{
		int pointPlace = static_cast<int>(m_currentBarData->GetPoints());
		m_pointMultiplier = pow(10, -1 * pointPlace);
	}
	// After updating the current bar data, process active orders
	ProcessActiveOrders();
	// Recalculate floating PnL based on new current price
	CalcFloatingPnL();
	// Recalculate used margin 
	CalcUsedMargin();
	// Check for margin stop out conditions
	MarginStopOutCheck();

}

void MetaTraderSimulator::MarginStopOutCheck()
{
	if(m_usedMargin <= 0.0)
		return; // No used margin, no need to check

	// Implementation for margin stop out check
	const double marginLevel = (m_equityBalance / m_usedMargin) * 100.0; // Margin level in percentage
	const double stopOutLevel = 50.0; // stop out level at 50%
	if (marginLevel < stopOutLevel)
	{
		Logger::getInstance().WriteLog("Margin level below stop out level. Closing all active orders.", false, false, LogType::LOG);
		// Close all active orders
		for (auto& order : m_activeOrders)
		{
			CreateDealFromOrder(order, ClosureReason::STOP_OUT); // Assume all orders closed at stop loss
			order.isActive = false; // Mark order as inactive
		}
		// Remove closed orders from active orders list
		RemoveClosedOrders();
		// Recalculate floating PnL and used margin after closing orders
		CalcFloatingPnL();
		CalcUsedMargin();
	}
}
