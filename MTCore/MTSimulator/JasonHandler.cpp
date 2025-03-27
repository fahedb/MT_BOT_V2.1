#include "JasonHandler.h"

void JasonHandler::Add_Indicator_ToJson(nlohmann::json_abi_v3_11_3::json* j, std::string timePoint, std::string indT, IndicatorNode indNode)
{
	(*j)[timePoint][HTTP_BODY_MEMBER_IND_FATHER][indT][GetLenString(indNode.len)][HTTP_BODY_MEMBER_IND_VALUES] =
	{
			{HTTP_BODY_MEMBER_Period_M1,std::string(indNode.vals.M1,  7)},
			{HTTP_BODY_MEMBER_Period_M5,std::string(indNode.vals.M5,  7)},
			{HTTP_BODY_MEMBER_Period_M15,std::string(indNode.vals.M15,7)},
			{HTTP_BODY_MEMBER_Period_M30,std::string(indNode.vals.M30,7)},
			{HTTP_BODY_MEMBER_Period_H1,std::string(indNode.vals.H1,  7)},
			{HTTP_BODY_MEMBER_Period_H4,std::string(indNode.vals.H4,  7)}
	};
	(*j)[timePoint][HTTP_BODY_MEMBER_IND_FATHER][indT][GetLenString(indNode.len)][HTTP_BODY_MEMBER_IND_SLOPES] =
	{
			{HTTP_BODY_MEMBER_Period_M5, std::string(indNode.vals.M5_Slope ,7)},
			{HTTP_BODY_MEMBER_Period_M1, std::string(indNode.vals.M1_Slope ,7)},
			{HTTP_BODY_MEMBER_Period_M15,std::string(indNode.vals.M15_Slope,7)},
			{HTTP_BODY_MEMBER_Period_M30,std::string(indNode.vals.M30_Slope,7)},
			{HTTP_BODY_MEMBER_Period_H1, std::string(indNode.vals.H1_Slope ,7)},
			{HTTP_BODY_MEMBER_Period_H4, std::string(indNode.vals.H4_Slope ,7)}
	};
}