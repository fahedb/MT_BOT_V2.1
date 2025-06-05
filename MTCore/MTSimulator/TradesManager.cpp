#include "TradesManager.h"

OrderEntity* TradesManager::CheckForTrade_PatternGod(DirectionStats& stats) // Function to check for trades for PatternGod and statistics Stats
{

	ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();

	CommonDirection generalDir = CheckDirection(stats, StatsType::General);
	CommonDirection firstDir = CheckDirection(stats, StatsType::First);
	CommonDirection highestDir = CheckDirection(stats, StatsType::Highest);

	OrderEntity* order = nullptr;
	OrderType ordType;


	if (highestDir == CommonDirection::UP)
		ordType = OrderType::LONG;
	else if (highestDir == CommonDirection::DOWN)
		ordType = OrderType::SHORT;
	else
		ordType = OrderType::NONE;

	double mean = stats.mean(StatsType::Highest);

	double stopLoss = 0;
	if (ordType != OrderType::NONE)
		stopLoss = stats.Get_PureMean(StatsType::Highest, highestDir == CommonDirection::UP ? CommonDirection::DOWN : CommonDirection::UP);

	
	
	if ((generalDir == highestDir || firstDir == highestDir) && ordType != OrderType::NONE)
	{
		order = new OrderEntity(ordType, stats.Get_TimePoint(), stats.Get_TimePoint(), CoinsPair::XAUUSD);
		order->Set_TakeProfit(static_cast<int>(mean));
		order->Set_StopLoss(static_cast<int>(stopLoss));
	}

	if (order)
	{
		OrderEntityCache::getInstance()->Add_NewOrder(order); // Add the order to the cache
		Send_TraddeSignal(order->Get_PairType_str(), int(order->Get_OrderType()), order->Get_OrderSeqNum(),
			order->Get_StopLoss(), order->Get_TakeProfit()); // Send the trade signal to the exchange
	}
	return order;
}
