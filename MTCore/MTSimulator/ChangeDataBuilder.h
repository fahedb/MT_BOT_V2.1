#include "ChangeData.h"
#include "LOGGER.H"
#include "ProcessData.h"
#include "SelectionNodes.h"
#include "DirectionStats.h"

#pragma once

class ChangeDataBuilder
{
public:
	static DirectionStats BuildDirectionStats(JsonSelectionNode* selectionJ)
	{
		Logger& logger = Logger::getInstance();
		std::map<std::string, std::string> mapy;
		Http_Utils::ExtractAllValues(*(selectionJ->Get_Json()), mapy);
		for (auto it = mapy.begin(); it != mapy.end(); ++it) {

			std::vector<std::string> CurrKey = Http_Utils::SplitKey(it->first);
			std::string cleanedValue = Http_Utils::ExtractValuesFrom(it->second);
		
		}
	}

	static ChangeNode* BuildChangeNode(JsonSelectionNode* selectionJ)
	{
		Logger& logger = Logger::getInstance();
		ChangeNode* chN = new ChangeNode();
		std::map<std::string, std::string> mapy;
		Http_Utils::ExtractAllValues(*(selectionJ->Get_Json()), mapy);
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
					return nullptr;
				}
				auto indT = Http_Utils::Get_Enum_IndicatorType(CurrKey[2]);
				auto indL = Http_Utils::Get_Enum_IdicatorLen(CurrKey[3]);
				auto indVT = Http_Utils::Get_Enum_IndicatorValueType(CurrKey[4]);
				auto barP = Http_Utils::Get_Enum_BarPeriod(CurrKey[5]);
				if (indVT == IndicatorValueType::Slope)
					chN->Get_Indicators_Map()->Set_Ind_SingleSlope(indT, indL, barP, std::stod(cleanedValue));
				else if (indVT == IndicatorValueType::Value)
					chN->Get_Indicators_Map()->Set_Ind_SingleValue(indT, indL, barP, std::stod(cleanedValue));
			}
			else if (CurrKey[1] == HTTP_BODY_MEMBER_Symbol)
			{
				chN->symbol = cleanedValue;
			}
			else {
				chN->Set_Entity(CurrKey[1], std::stod(cleanedValue));
				if (!timePointSwitch)
				{
					chN->point_DateTime = CurrKey.at(0);
					timePointSwitch = true;
				}
			}

		}
		chN->activePoint = true;
		chN->Pip_Change = ProcessData::roundToDecimalPlaces(chN->Get_Entity(HTTP_BODY_MEMBER_ClosePrice), 2)
			- ProcessData::roundToDecimalPlaces(chN->Get_Entity(HTTP_BODY_MEMBER_OpenPrice), 2);
		return chN;
	}

};

