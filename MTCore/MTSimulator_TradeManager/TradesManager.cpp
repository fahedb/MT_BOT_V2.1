#include "TradesManager.h"
int maximum(int a, int b) { return (a > b) ? a : b; }

OrderEntity* TradesManager::CheckForTrade_PatternGod(DirectionStats& stats) // Function to check for trades for PatternGod and statistics Stats
{
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

	double mean = abs(stats.mean(StatsType::Highest));

	double pureMean = 0;
	if (ordType != OrderType::NONE)
		pureMean = abs(stats.Get_PureMean(StatsType::Highest, highestDir == CommonDirection::UP ? CommonDirection::DOWN : CommonDirection::UP));

	
	
	if ((generalDir == highestDir || firstDir == highestDir) && ordType != OrderType::NONE)
	{
		order = new OrderEntity(ordType, stats.Get_TimePoint(), stats.Get_TimePoint(), CoinsPair::XAUUSD);
		double orderStopLoss = m_ordersPolicy->Dynamic_SL_TP ? static_cast<int>(pureMean) : m_ordersPolicy->SL_start;
		orderStopLoss = maximum(std::abs(orderStopLoss), std::abs(m_ordersPolicy->SL_start)); 
		order->Set_StopLoss(orderStopLoss);

		double orderTakeProfit = m_ordersPolicy->Dynamic_SL_TP ? static_cast<int>(mean) : m_ordersPolicy->TP_start;
		orderTakeProfit = maximum(std::abs(orderTakeProfit), std::abs(m_ordersPolicy->TP_start));
		order->Set_TakeProfit(orderTakeProfit);
	}

	if (order)
	{
		OrderEntityCache::getInstance()->Add_NewOrder(order); // Add the order to the cache
		Send_TraddeSignal(order->Get_PairType_str(), int(order->Get_OrderType()), order->Get_OrderSeqNum(),
			order->Get_StopLoss(), order->Get_TakeProfit()); // Send the trade signal to the exchange
	}
	return order;
}

CommonDirection TradesManager::CalcCommonDirection(DirectionStats& stats)
{
	if (!stats.IsActive())
		return CommonDirection::BLUR;

	CommonDirection general = CheckDirection(stats, StatsType::General);
	CommonDirection first = CheckDirection(stats, StatsType::First);
	CommonDirection highest = CheckDirection(stats, StatsType::Highest);

	if (highest == first)
		return highest;
	if (general == highest)
		return highest;
	return CommonDirection::BLUR;
}

CommonDirection TradesManager::CalcCommonDirection_Pure(DirectionStats& stats)
{
	if (!stats.IsActive())
		return CommonDirection::BLUR;

	CommonDirection general = CheckDirection_Pure(stats, StatsType::General);
	CommonDirection first = CheckDirection_Pure(stats, StatsType::First);
	CommonDirection highest = CheckDirection_Pure(stats, StatsType::Highest);

	if (highest == first)
		return highest;
	if (general == highest)
		return highest;
	return CommonDirection::BLUR;
}

CommonDirection TradesManager::CheckDirection(DirectionStats& stats, StatsType stype)
{

	int patternsCount= stats.Get_Total(stype);
	auto directionMap = stats.Get_VotedPercentages(stype);

	double upPercent = directionMap[CommonDirection::UP];
	double downPercent = directionMap[CommonDirection::DOWN];
	double blurPercent = directionMap[CommonDirection::BLUR];

	double meanPips = stats.mean(StatsType::General);

	if (patternsCount >= m_tradeMngrPolicy->Pattern_Min_Patterns_ToTake_Action)
	{
		bool isStrongUp = upPercent > m_tradeMngrPolicy->Pattern_Min_Percent_To_ExecuteOrder
			&& (meanPips) >= m_tradeMngrPolicy->Min_MeanPips_ToExecute_Order;

		bool isStrongDown = downPercent > m_tradeMngrPolicy->Pattern_Min_Percent_To_ExecuteOrder
			&& (meanPips) <= -m_tradeMngrPolicy->Min_MeanPips_ToExecute_Order;

		if (isStrongUp)
		{
			return CommonDirection::UP;
		}
		else if (isStrongDown)
		{
			return CommonDirection::DOWN;
		}
		else if (m_tradeMngrPolicy->Pattern_Activate_Alternative_ExecuteOrder)
		{
			bool isAlternativeAllowed = blurPercent <= m_tradeMngrPolicy->Pattern_Max_Noise_To_ExecuteOrder_Alternative
				&& std::fabs(downPercent - upPercent) >= m_tradeMngrPolicy->Pattern_Diff_Percent_To_ExecuteOrder_Alternative;

			if (isAlternativeAllowed)
			{
				bool isAlternativeUp = upPercent > downPercent && (meanPips) >= (m_tradeMngrPolicy->Min_MeanPips_ToExecute_Order + 10);
				bool isAlternativeDown = downPercent > upPercent && (meanPips) <= -(m_tradeMngrPolicy->Min_MeanPips_ToExecute_Order + 10);

				if (isAlternativeUp)
				{
					return CommonDirection::UP;
				}
				else if (isAlternativeDown)
				{
					return CommonDirection::DOWN;
				}
			}
		}
	}

	return CommonDirection::BLUR;
}

CommonDirection TradesManager::CheckDirection_Pure(DirectionStats& stats, StatsType stype)
{
	int generalCount = stats.Get_Total(stype);
	auto directionMap = stats.Get_VotedPercentages(stype);

	double upPercent = directionMap[CommonDirection::UP];
	double downPercent = directionMap[CommonDirection::DOWN];
	double blurPercent = directionMap[CommonDirection::BLUR];

	double meanPips = stats.mean(StatsType::General);

	bool isUp = upPercent > m_tradeMngrPolicy->Pattern_Min_Percent_To_ExecuteOrder;
	bool isDown = downPercent > m_tradeMngrPolicy->Pattern_Min_Percent_To_ExecuteOrder;

	if (isUp)
	{
		return CommonDirection::UP;
	}
	else if (isDown)
	{
		return CommonDirection::DOWN;
	}
	else if (m_tradeMngrPolicy->Pattern_Activate_Alternative_ExecuteOrder)
	{
		bool isAlternativeAllowed = blurPercent <= m_tradeMngrPolicy->Pattern_Max_Noise_To_ExecuteOrder_Alternative
			&& std::fabs(downPercent - upPercent) >= m_tradeMngrPolicy->Pattern_Diff_Percent_To_ExecuteOrder_Alternative;

		if (isAlternativeAllowed)
		{
			bool isAlternativeUp = upPercent > downPercent;
			bool isAlternativeDown = downPercent > upPercent;

			if (isAlternativeUp)
			{
				return CommonDirection::UP;
			}
			else if (isAlternativeDown)
			{
				return CommonDirection::DOWN;
			}
		}
	}
	return CommonDirection::BLUR;
}
