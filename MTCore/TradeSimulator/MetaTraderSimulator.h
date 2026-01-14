#pragma once
#include "OrderStruct.h"
#include "ChangeData.h"
#include "DealStruct.h"
#include "SimulatorActiveOrder.h"
#include "ClosureReason.h"

class MetaTraderSimulator
{
private:
	int m_simulationId; // Unique identifier for the simulation session
	
	double m_acountBalance; // Initial account balance for the simulation
	double m_floatingPnL; // Floating profit and loss balance
	double m_equityBalance; // Equity balance for the simulation
	double m_usedMargin; // Used margin in the simulation
	int m_leverage; // Leverage used in the simulation

	int m_totalProfitInPips; // Total profit in pips for the simulation
	double m_contractSize; // Contract size for the trading symbol
	double m_currentPrice; // Current price in the simulation
	double m_pointMultiplier;
	ChangeNode* m_currentBarData; // Current bar data in the simulation
	double m_lotForTrade; // Lot size for trades in the simulation
	long long m_currentTimePoint; // Current time point in the simulation
	std::string m_symbol; // Trading symbol for the simulation

	std::vector<ActiveOrderInfo> m_activeOrders; // List of active orders in the simulation
	std::vector<Deal> m_closedDeals; // List of closed deals in the simulation
private:
	// Private methods for internal simulation logic can be added here
	ActiveOrderInfo ConvertOrderEntityToActiveOrderInfo(const OrderEntity* order);
	void ProcessActiveOrders();
	bool CheckOrderClosureConditionsAndCreateDeal(const ActiveOrderInfo& order);
	void CreateDealFromOrder(const ActiveOrderInfo& order, ClosureReason closeReason);
	void RemoveClosedOrders(); // Remove all isActive = false orders from m_activeOrders
	void UpdateAcountBalance(const Deal& deal);
public:
	MetaTraderSimulator(int simulationId, double initialBalance,double lotForTrade, double contractSize, std::string symbol, int leverage = 1)
		: m_simulationId(simulationId), m_acountBalance(initialBalance), m_currentPrice(0.0), m_leverage(leverage > 0 ? leverage : 1)
		, m_currentTimePoint(0), m_totalProfitInPips(0), m_currentBarData(nullptr), m_pointMultiplier(0.0)
		, m_lotForTrade(lotForTrade), m_contractSize(contractSize)
		, m_symbol(symbol)
	{
	}
	// Geters:
	double Get_AccountBalance() const { return m_acountBalance; }
	int Get_SimulationId() const { return m_simulationId; }

	// Seters:
	void Set_NewOrder(OrderEntity* newOrder);

	void Set_NewCurrBarData(ChangeNode* newBarData);
	void CalcFloatingPnL(); // Calculate floating PnL based on current active orders and current price
	void CalcUsedMargin(); // Calculate used margin based on current active orders
	double Get_FreeMargin() const { return m_equityBalance - m_usedMargin; }
	void MarginStopOutCheck(); // Check if margin level is below stop out level and close orders if necessary

	static double RequiredMargin(double price, double contractSize, double lotSize, int leverage) { return (price * contractSize * lotSize) / leverage; }
	
};

