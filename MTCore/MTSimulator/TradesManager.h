#pragma once

#include "..\..\HTTPCore\ExchangeData.h"
#include "CONFIGRATIONS.H"
#include "DirectionStats.h"
#include "OrderStruct.h"

/// <summary>
/// This Class is responsible for send a trades 
/// Though DirectionStats object that we get from StatisticsDecession Class
/// </summary>

class TradesManager
{
private:
	int m_orderCode;
	int Cur_Orders_InRow;
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
	OrderStruct CheckForTrade_PatternGod(DirectionStats& stats) // Function to check for trades for PatternGod and statistics Stats
    {

		ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();

		int generalCount = stats.Get_TotalCount(StatsType::General);
		double upPercent = (double(stats.Get_UpCount(StatsType::General)) / generalCount) * 100.0;
		double downPercent = (double(stats.Get_DownCount(StatsType::General)) / generalCount) * 100.0;
		double blurPercent = (double(stats.Get_BlurCount(StatsType::General)) / generalCount) * 100.0;

		double meanPips = stats.mean(StatsType::General);
		OrderStruct order;

		if (generalCount >= policy->Patter_Min_Patterns_ToTake_Action)
		{
			bool isStrongUp = upPercent > policy->Pattern_Min_Percent_To_ExecuteOrder
				&& (meanPips) >= policy->Min_MeanPips_ToExecute_Order;

			bool isStrongDown = downPercent > policy->Pattern_Min_Percent_To_ExecuteOrder
				&& (meanPips) <= -policy->Min_MeanPips_ToExecute_Order;

			if (isStrongUp)
			{
				Send_TraddeSignal("XAUUSD", 0);
				stats.Set_TradeRes(true);
				order.ActivateOrder(OrderType::LONG, stats.timePoint, stats.timePoint, CoinsPair::XAUUSD);
			}
			else if (isStrongDown)
			{
				Send_TraddeSignal("XAUUSD", 1);
				stats.Set_TradeRes(true);
				order.ActivateOrder(OrderType::SHORT, stats.timePoint, stats.timePoint, CoinsPair::XAUUSD);
			}
			else if (policy->Pattern_Activate_Alternative_ExecuteOrder)
			{
				bool isAlternativeAllowed = blurPercent <= policy->Pattern_Max_Noise_To_ExecuteOrder_Alternative
					&& std::fabs(downPercent - upPercent) >= policy->Pattern_Diff_Percent_To_ExecuteOrder_Alternative;

				if (isAlternativeAllowed)
				{
					bool isAlternativeUp = upPercent > downPercent && (meanPips) >= (policy->Min_MeanPips_ToExecute_Order + 10);
					bool isAlternativeDown = downPercent > upPercent && (meanPips) <= -(policy->Min_MeanPips_ToExecute_Order + 10);

					if (isAlternativeUp)
					{
						Send_TraddeSignal("XAUUSD", 0);
						stats.Set_TradeRes(true);
						order.ActivateOrder(OrderType::LONG, stats.timePoint, stats.timePoint, CoinsPair::XAUUSD);
					}
					else if (isAlternativeDown)
					{
						Send_TraddeSignal("XAUUSD", 1);
						stats.Set_TradeRes(true);
						order.ActivateOrder(OrderType::SHORT, stats.timePoint, stats.timePoint, CoinsPair::XAUUSD);
					}
				}
			}
		}

		Cur_Orders_InRow = 0;
		stats.Set_TradeRes(false);
		return order;

    }

private: 

	void Send_TraddeSignal(std::string pair, int type)
	{
		ExchangeData().Set_NewOrder("XAUUSD", type);
		m_orderCode++; Cur_Orders_InRow++;
	}

private:
    TradesManager()
    {
		Cur_Orders_InRow = 0;
		m_orderCode = 0; // TODO: Need to emplement a logic to get orderCode from seqnum table
    }
    ~TradesManager()
    {

    }

};

