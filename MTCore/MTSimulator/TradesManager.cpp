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
		stopLoss = stats.Get_PureMean(StatsType::Highest, highestDir == CommonDirection::UP ? CommonDirection::DOWN : CommonDirection::UP) + 30; // Need to implement a policy factor instead of the hardcoded factor

	
	
	if ((generalDir == highestDir || firstDir == highestDir) && ordType != OrderType::NONE)
	{
		order = new OrderEntity(ordType, stats.Get_TimePoint(), stats.Get_TimePoint(), CoinsPair::XAUUSD);
		order->Set_TakeProfit(mean);
	}


	return order;
}
