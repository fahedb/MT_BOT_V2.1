#include "ChangeDataBuilder.h"
#include "LOGGER.H"
#include "ProcessData.h"
#include "Converts.h"

bool ChangeDataBuilder::BuildChangeNode(JsonSelectionNode* selectionJ,ChangeNode* chN, const std::string& symbol)
{
	Logger& logger = Logger::getInstance();
	std::map<std::string, std::string> mapy;
	std::string keyBuffer;
	Http_Utils::ExtractAllValues(*(selectionJ->Get_Json()), mapy, keyBuffer);
	bool timePointSwitch = false;
	for (auto it = mapy.begin(); it != mapy.end(); ++it) {

		std::vector<std::string> CurrKey = Http_Utils::SplitKey(it->first);
		std::string cleanedValue = Http_Utils::ExtractValuesFrom(it->second);

		if (CurrKey.size() < 2)
			continue;

		if (CurrKey[1] == HTTP_BODY_MEMBER_IND_FATHER)
		{
			if (CurrKey.size() < 6)
			{
				logger.WriteLog("Error in Json Structure !!!\nExit building Node !!!", false, true);
				logger.WriteLog("Error in Json Structure !!!\nExit building Node !!!", false, true, LogType::SPECIAL_LOG);
				return false;
			}
			auto indT = Http_Utils::Get_Enum_IndicatorType(CurrKey[2]);
			auto indL = Http_Utils::Get_Enum_IdicatorLen(CurrKey[3]);
			auto indVT = Http_Utils::Get_Enum_IndicatorValueType(CurrKey[4]);
			auto barP = Http_Utils::Get_Enum_BarPeriod(CurrKey[5]);
			if (indVT == IndicatorValueType::Slope)
				chN->Get_Indicators_Map()->Set_Ind_SingleSlope(indT, indL, barP, Converts::ToDouble(cleanedValue));
			else if (indVT == IndicatorValueType::Value)
				chN->Get_Indicators_Map()->Set_Ind_SingleValue(indT, indL, barP, Converts::ToDouble(cleanedValue));
		}
		else if (CurrKey[1] == HTTP_BODY_MEMBER_Symbol)
		{
			chN->symbol = cleanedValue;
		}
		else {

			if (CurrKey[1] == HTTP_BODY_MEMBER_OpenPrice)
				chN->m_openPrice = Converts::ToDouble(cleanedValue);
			else if (CurrKey[1] == HTTP_BODY_MEMBER_ClosePrice)
				chN->m_closePrice = Converts::ToDouble(cleanedValue);
			else if (CurrKey[1] == HTTP_BODY_MEMBER_HighPrice)
				chN->m_highPrice = Converts::ToDouble(cleanedValue);
			else if (CurrKey[1] == HTTP_BODY_MEMBER_LowPrice)
				chN->m_lowPrice = Converts::ToDouble(cleanedValue);

			if (!timePointSwitch)
			{
				chN->point_DateTime = CurrKey.at(0);
				chN->point_DateTimeLong = Converts::ToLongLong(chN->point_DateTime);
				timePointSwitch = true;
			}
		}

	}
	chN->activePoint = true;
	chN->Pip_Change = ProcessData::roundToDecimalPlaces(chN->m_closePrice, 2)
		- ProcessData::roundToDecimalPlaces(chN->m_openPrice, 2);

	chN->symbol = symbol;
	chN->SetPoints(_PointsUtils::GetPointPlaceFromSymbol(symbol));
	return chN;
}

int ChangeDataBuilder::ExtractMonth(long long tp) {
	// Better month extraction for timestamp format YYYYMMDDHHMMSSSSS
	// Extract month from position 5-6 (0-based)
	int month = ((tp / 100000000000LL) % 100);
	// Ensure month is in valid range 1-12, then convert to 0-11
	if (month < 1 || month > 12) {
		Logger::getInstance().WriteLog("Invalid month extracted: " + std::to_string(month) + " from timestamp: " + std::to_string(tp), false, true, LogType::ERR);
		return 0; // Default to January if invalid
	}
	return month - 1; // Convert to 0-based index
}
