#include "nlohmann/json.hpp"
#include "CandleData.h"
#include "../../HTTPCore/ServicesDef.h"

#pragma once
class JasonHandler
{
public:
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
	static std::string GetLenString(int len) { return "L" + std::to_string(len); }
	static void Add_Indicator_ToJson(nlohmann::json_abi_v3_11_3::json* j, std::string timePoint, std::string indT, IndicatorNode indNode);
};

