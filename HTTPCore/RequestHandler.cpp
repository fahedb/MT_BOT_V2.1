#include "RequestHandler.h"
//
//static std::string GetIndicatorPeriodAccess(const BarPeriod& barPeriod)
//{
//	switch (barPeriod)
//	{
//	case BarPeriod::M1:
//		return HTTP_BODY_MEMBER_Period_M1;
//	case BarPeriod::M5:
//		return HTTP_BODY_MEMBER_Period_M5;
//	case BarPeriod::M15:
//		return HTTP_BODY_MEMBER_Period_M15;
//	case BarPeriod::M30:
//		return HTTP_BODY_MEMBER_Period_M30;
//		break;
//	case BarPeriod::H1:
//		return HTTP_BODY_MEMBER_Period_H1;
//	case BarPeriod::H4:
//		return HTTP_BODY_MEMBER_Period_H4;
//	default:
//		return "";
//	}
//	return "";
//}
//
//static std::string GetIndicatorTypeAccess(IndicatorType ind)
//{
//	switch (ind)
//	{
//	case IndicatorType::EMA:
//		return HTTP_BODY_MEMBER_IND_EMA;
//	case IndicatorType::RSI:
//		return HTTP_BODY_MEMBER_IND_RSI;
//	case IndicatorType::ZLSMA:
//		return HTTP_BODY_MEMBER_IND_ZLSMA;
//	default:
//		return "";
//	}
//}
//
//static std::string GetIndicatorLenAccess(IndicatorLen len)
//{
//	switch (len)
//	{
//	case IndicatorLen::L10:
//		return HTTP_BODY_MEMBER_IND_LEN_10;
//	case IndicatorLen::L15:
//		return HTTP_BODY_MEMBER_IND_LEN_15;
//	case IndicatorLen::L20:
//		return HTTP_BODY_MEMBER_IND_LEN_20;
//	case IndicatorLen::L25:
//		return HTTP_BODY_MEMBER_IND_LEN_25;
//	case IndicatorLen::L30:
//		return HTTP_BODY_MEMBER_IND_LEN_30;
//	default:
//		return "";
//	}
//	return "";
//}
//
//static double GetIndicatorValue(nlohmann::json* http_body, IndicatorType ind, BarPeriod barPeriod, IndicatorLen len)
//{
//	if (!http_body)
//		return -1;
//	std::string accessInd = RequestHandler::GetIndicatorTypeAccess(ind);
//	std::string accessLen = RequestHandler::GetIndicatorLenAccess(len);
//	std::string accessValue = RequestHandler::GetIndicatorPeriodAccess(barPeriod);
//
//	if (http_body->contains(accessInd) && (*http_body)[accessInd].contains(accessLen) && (*http_body)[accessInd][accessLen].contains(accessValue))
//		return (*http_body)[accessInd][accessLen][accessValue];
//	return -1;
//}


