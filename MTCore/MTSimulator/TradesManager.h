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
	OrderEntity* Get_Order(std::string timePoint) { return m_ordersCache->Get_Order(timePoint); }
	int Get_OrdersCount() { return m_ordersCache->Get_OrdersCount(); }
	
	OrderEntity* CheckForTrade_PatternGod(DirectionStats& stats);

	CommonDirection CalcCommonDirection(DirectionStats& stats)
	{
		if (!stats.IsActive())
			return CommonDirection::BLUR;

		CommonDirection general = CheckDirection(stats, StatsType::General);
		CommonDirection first = CheckDirection(stats, StatsType::First);
		CommonDirection highest= CheckDirection(stats, StatsType::Highest);
		
		if (highest == first)
			return highest;
		if (general == highest)
			return highest;
		return CommonDirection::BLUR;
	}
	

private: 

	CommonDirection CheckDirection(DirectionStats& stats,StatsType stype)
	{
		ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();

		int generalCount = stats.Get_TotalCount(stype);
		double upPercent = (double(stats.Get_UpCount(stype)) / generalCount) * 100.0;
		double downPercent = (double(stats.Get_DownCount(stype)) / generalCount) * 100.0;
		double blurPercent = (double(stats.Get_BlurCount(stype)) / generalCount) * 100.0;

		double meanPips = stats.mean(StatsType::General);

		if (generalCount >= policy->Patter_Min_Patterns_ToTake_Action)
		{
			bool isStrongUp = upPercent > policy->Pattern_Min_Percent_To_ExecuteOrder
				&& (meanPips) >= policy->Min_MeanPips_ToExecute_Order;

			bool isStrongDown = downPercent > policy->Pattern_Min_Percent_To_ExecuteOrder
				&& (meanPips) <= -policy->Min_MeanPips_ToExecute_Order;

			if (isStrongUp)
			{
				return CommonDirection::UP;
			}
			else if (isStrongDown)
			{
				return CommonDirection::DOWN;
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
		m_ordersCache = OrderEntityCache::getInstance();
    }
    ~TradesManager()
    {

    }

};

