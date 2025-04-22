#include "ChangeData.h"
#include "LOGGER.H"
#include "ProcessData.h"
#include "SelectionNodes.h"
#include "DirectionStats.h"

#pragma once

class ChangeDataBuilder
{
public:
    static DirectionStatsJson BuildDirectionStatsJson(JsonSelectionNode* jsonNode) {
        DirectionStatsJson result;
        nlohmann::json_abi_v3_11_3::json j = *jsonNode->Get_Json();
        
        if (j.is_object() && !j.empty()) {
            result.timePoint = j.begin().key(); // Getting time point key
            const nlohmann::json_abi_v3_11_3::json& body = j.begin().value();

            auto parseGroup = [](const nlohmann::json_abi_v3_11_3::json& groupJson) -> DirectionGroupStatsJson {
                DirectionGroupStatsJson group;

                // CANLE_INDEX_DETAILS
                if (groupJson.contains("CANLE_INDEX_DETAILS")) {
                    for (const auto& entry : groupJson["CANLE_INDEX_DETAILS"]) {
                        if (entry.size() == 2) {
                            CandleIndexDetail detail;
                            detail.candleIdx = std::stoi(entry[0].get<std::string>());
                            
                            //Example "1 : 65"
                            std::string val = entry[1].get<std::string>();
                            size_t colonPos = val.find(':');
                            if (colonPos != std::string::npos) {
                                detail.indexInArray = std::stoi(val.substr(0, colonPos));
                                detail.pipChange = std::stoi(val.substr(colonPos + 1));
                            }

                            group.candleIndexDetails.push_back(detail);
                        }
                    }
                }

                group.up = groupJson.value("UP", "0.0") != "" ? std::stod(groupJson.value("UP", "0.0")) : 0.0;
                group.down = groupJson.value("DOWN", "0.0") != "" ? std::stod(groupJson.value("DOWN", "0.0")) : 0.0;
                group.noise = groupJson.value("NOISE", "0.0") != "" ? std::stod(groupJson.value("NOISE", "0.0")) : 0.0;
                group.mean = groupJson.value("MEAN", "0.0") != "" ? std::stod(groupJson.value("MEAN", "0.0")) : 0.0;
                group.stddev = groupJson.value("StdDev", "0.0") != "" ? std::stod(groupJson.value("StdDev", "0.0")) : 0.0;
                group.meanIndexes = groupJson.value("MEAN_INDEXES", "0.0") != "" ? std::stod(groupJson.value("MEAN_INDEXES", "0.0")) : 0.0;

                return group;
            };

            // Getting data from first and general
            if (body.contains("First")) 
                result.first = parseGroup(body["First"]);
            if (body.contains("General")) 
                result.general = parseGroup(body["General"]);

            // StepsMeans
            if (body.contains("StepsMeans")) {
                for (const auto& pair : body["StepsMeans"]) {
                    if (pair.size() == 2) {
                        int index = std::stoi(pair[0].get<std::string>());
                        double mean = pair[1].get<double>();
                        result.stepsMeans.emplace_back(index, mean);
                    }
                }
            }
        }

        return result;
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

