#include "nlohmann/json.hpp"
#include "AnalyzerGraphsPackage.h"

#pragma once
class JsonHandler_GraphContainer
{
	// Json Example for Graphs Package:
	//{
	//	"Id": "XAUUSD_M1_20250703_1452",
	//		"FilterType" : "EMA_RSI",
	//		"BarsPngFile" : "charts/XAUUSD_M1_20250703_1452_bars.png",
	//		"LinearPngFile" : "charts/XAUUSD_M1_20250703_1452_linear.png",
	//		"BarsStructure" : {
	//		"points": [
	//			[0.0, 1.0],
	//			[1.0, 0.8],
	//			[2.0, 1.1]
	//		],
	//			"splitter" : 1
	//	},
	//		"LinearStructure": {
	//		"points": [
	//			[0.0, 1.05],
	//			[1.0, 0.95],
	//			[2.0, 1.2]
	//		],
	//			"splitter" : 1
	//	}
	//}


	static nlohmann::json Build_GraphsPackageJson(const AnalyzerGraphsPackage& package)
	{
		nlohmann::json j;
		j[stats.Get_TimePoint()] =
		{
			{JSON_BODY_MEMBER_PATTERNS_COUNT,std::to_string(stats.Get_TotalCount(StatsType::General))}
		};

		auto allmeans_perIndex = stats.GetAllMeansUpTo(CommonDirection::NONE);
		for (int i = 0; i < allmeans_perIndex.size(); i++)
			j[stats.Get_TimePoint()]["StepsMeans"].push_back(std::make_pair(std::to_string(i + 1), allmeans_perIndex.at(i)));

		Add_Calcs_toJson_Stats(&j, "First", stats);
		Add_Calcs_toJson_Stats(&j, "General", stats);
		Add_Calcs_toJson_Stats(&j, "Highest", stats);
		return j;
	}
};

