#include "nlohmann/json.hpp"
#include "CandleData.h"
#include "../../HTTPCore/ServicesDef.h"
#include "DirectionStats.h"
#pragma once
class JasonHandler
{
public:
	static nlohmann::json_abi_v3_11_3::json Build_Json_ForStats(DirectionStats& stats)
	{
		nlohmann::json_abi_v3_11_3::json j;
		j[stats.timePoint] =
		{
			{JSON_BODY_MEMBER_PATTERNS_COUNT,std::to_string(stats.Get_TotalCount(StatsType::General))}
		};

		auto allmeans_perIndex = stats.GetAllMeansUpTo();
		for (int i = 0; i < allmeans_perIndex.size(); i++)
			j[stats.timePoint]["StepsMeans"].push_back(std::make_pair(std::to_string(i + 1), allmeans_perIndex.at(i)));

		Add_Calcs_toJson_Stats(&j, "First", stats);
		Add_Calcs_toJson_Stats(&j, "General", stats);
		return j;
	}


	static nlohmann::json_abi_v3_11_3::json Build_Json_ForCandle(std::string timePoint,CandleData candle)
	{
		nlohmann::json_abi_v3_11_3::json j;
		j[timePoint] =
		{
			 {HTTP_BODY_MEMBER_OpenPrice, std::string(candle.openPrice ,7)}
			,{HTTP_BODY_MEMBER_ClosePrice,std::string(candle.closePrice,7)}
			,{HTTP_BODY_MEMBER_HighPrice, std::string(candle.highPrice ,7)}
			,{HTTP_BODY_MEMBER_LowPrice,  std::string(candle.lowPrice  ,7)}
		};

		for (int i = 0; i < INDICATOR_LENS_COUNT; i++)
		{
			Add_Indicator_ToJson(&j, timePoint, HTTP_BODY_MEMBER_IND_EMA, candle.Ema_nodes[i]);
			Add_Indicator_ToJson(&j, timePoint, HTTP_BODY_MEMBER_IND_RSI, candle.Rsi_nodes[i]);
			Add_Indicator_ToJson(&j, timePoint, HTTP_BODY_MEMBER_IND_ZLSMA, candle.Zlsma_nodes[i]);
		}
		return j;
	}
private:

	static void Add_Calcs_toJson_Stats(nlohmann::json_abi_v3_11_3::json* j,std::string son,DirectionStats& stats)
	{
		DirectionGroupStats grpStats = stats.GetGroupStatsByName(son);
		(*j)[stats.timePoint][son] =
		{
			{JSON_BODY_MEMBER_UP, std::to_string( (static_cast<double>(grpStats.UpCount) / static_cast<double>(grpStats.Total)) * 100.0 )},
			{JSON_BODY_MEMBER_DOWN, std::to_string((static_cast<double>(grpStats.DownCount) / static_cast<double>(grpStats.Total)) * 100.0) },
			{JSON_BODY_MEMBER_NOISE, std::to_string((static_cast<double>(grpStats.BlurCount) / static_cast<double>(grpStats.Total)) * 100.0)},

			{JSON_BODY_MEMBER_MEAN, std::to_string(grpStats.mean())},
			{JSON_BODY_MEMBER_STDDEV, std::to_string(grpStats.stddev())},
			{JSON_BODY_MEMBER_MEAN_INDEXES, std::to_string(grpStats.meanIndexes())}
		};

		auto indexesVec = grpStats.get_Indexes_and_changePips();
		for (int i = 0; i < indexesVec.size(); i++)
		{
			std::string idxStr = std::to_string(indexesVec.at(i).first) + " : " + std::to_string(indexesVec.at(i).second) ;
			(*j)[stats.timePoint][son][JSON_BODY_MEMBER_CANDLE_INDEX_DETAILS].push_back(std::make_pair(std::to_string(i), idxStr ));
		}

	}

	static std::string GetLenString(int len) { return "L" + std::to_string(len); }
	static void Add_Indicator_ToJson(nlohmann::json_abi_v3_11_3::json* j, std::string timePoint, std::string indT, IndicatorNode indNode);
};

