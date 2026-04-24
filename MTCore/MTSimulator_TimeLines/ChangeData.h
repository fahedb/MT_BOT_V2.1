#pragma once
#include <string>
#include <vector>
#include <math.h>
#include "IndicatorStructres.h"
#include "IndicatorMap.h"
#include "nlohmann/json.hpp"
#include "../../HTTPCore/ServicesDef.h"
#include <iostream>
#include <sstream>

#define INVALID_DATA -1;

class ChangeNode
{
private:
	_Points m_PointPlace;

public:
	double m_openPrice;
	double m_closePrice;
	double m_highPrice;
	double m_lowPrice;
	int m_nodeIdx;
	long long point_DateTimeLong = 0; // For fast access
	int	Pip_Change=0;

	bool Increase_Decrease = false;
	bool activePoint=false;

	std::string symbol;

	ChangeNode();
	ChangeNode(IndicatorStorage* indStorage, int changeNodeIndex);
	~ChangeNode();
	std::string point_DateTime;

public:
	void SetPoints(_Points points) { m_PointPlace = points; }
	
	_Points GetPoints() { return m_PointPlace; }
	void Set_Indicator(IndicatorType type, IndicatorLen len, IndicatorValues val, bool forceInsert = false) { m_indMap.Set_Indicator(type, len, val,forceInsert); }
	BaseIndicator* Get_Indicator_Pointer(IndicatorType type, IndicatorLen len) { return m_indMap.Get_Indicator(type, len); }
	IndicatorMap* Get_Indicators_Map() { return &m_indMap; }
private:
	IndicatorMap m_indMap;
};

class Http_Utils
{
public:
	static IndicatorType Get_Enum_IndicatorType(const std::string& type)
	{
		if (type == HTTP_BODY_MEMBER_IND_EMA)	return IndicatorType::EMA;
		if (type == HTTP_BODY_MEMBER_IND_RSI)	return IndicatorType::RSI;
		if (type == HTTP_BODY_MEMBER_IND_ZLSMA) return IndicatorType::ZLSMA;
		if (type == HTTP_BODY_MEMBER_IND_VOLUME) return IndicatorType::VOLUME;

		return IndicatorType::NONE;
	}

	static std::string Get_String_IndicatorType(IndicatorType type)
	{
		switch (type)
		{
		case IndicatorType::EMA: return HTTP_BODY_MEMBER_IND_EMA;
		case IndicatorType::RSI: return HTTP_BODY_MEMBER_IND_RSI;
		case IndicatorType::ZLSMA: return HTTP_BODY_MEMBER_IND_ZLSMA;
		case IndicatorType::VOLUME: return HTTP_BODY_MEMBER_IND_VOLUME;
		default:
			return "NONE"; // NONE is considered as an 0 length
		}
	}

	static IndicatorLen Get_Enum_IdicatorLen(const std::string& len)
	{
		if (len == HTTP_BODY_MEMBER_IND_LEN_10) return IndicatorLen::L10;
		if (len == HTTP_BODY_MEMBER_IND_LEN_15) return IndicatorLen::L15;
		if (len == HTTP_BODY_MEMBER_IND_LEN_20) return IndicatorLen::L20;
		if (len == HTTP_BODY_MEMBER_IND_LEN_25) return IndicatorLen::L25;
		if (len == HTTP_BODY_MEMBER_IND_LEN_30) return IndicatorLen::L30;
		if (len == HTTP_BODY_MEMBER_IND_LEN_NONE) return IndicatorLen::NONE;

		return IndicatorLen::NONE;
	}
	static IndicatorValueType Get_Enum_IndicatorValueType(const std::string& valType)
	{
		if (valType == HTTP_BODY_MEMBER_IND_VALUES) return IndicatorValueType::Value;
		if (valType == HTTP_BODY_MEMBER_IND_SLOPES) return IndicatorValueType::Slope;
		return IndicatorValueType::NONE;
	}

	static BarPeriod Get_Enum_BarPeriod(const std::string& barPeriod)
	{
		if (barPeriod == HTTP_BODY_MEMBER_Period_M1)	return BarPeriod::M1;
		if (barPeriod == HTTP_BODY_MEMBER_Period_M5)	return BarPeriod::M5;
		if (barPeriod == HTTP_BODY_MEMBER_Period_M15)	return BarPeriod::M15;
		if (barPeriod == HTTP_BODY_MEMBER_Period_M30)	return BarPeriod::M30;
		if (barPeriod == HTTP_BODY_MEMBER_Period_H1)	return BarPeriod::H1;
		if (barPeriod == HTTP_BODY_MEMBER_Period_H4)	return BarPeriod::H4;

		return BarPeriod::NONE;
	}

	static void ExtractAllValues(const nlohmann::json& j, std::map<std::string, std::string>& resultMap, std::string& keyBuffer) {
		if (j.is_object()) {
			for (auto it = j.begin(); it != j.end(); ++it) {
				size_t oldSize = keyBuffer.size();
				if (!keyBuffer.empty()) keyBuffer += ".";
				keyBuffer += it.key();
				ExtractAllValues(it.value(), resultMap, keyBuffer);
				keyBuffer.resize(oldSize); // מחזיר את keyBuffer למצב הקודם
			}
		}
		else if (j.is_array()) {
			for (size_t i = 0; i < j.size(); ++i) {
				size_t oldSize = keyBuffer.size();
				keyBuffer += "[" + std::to_string(i) + "]";
				ExtractAllValues(j[i], resultMap, keyBuffer);
				keyBuffer.resize(oldSize);
			}
		}
		else {
			resultMap[keyBuffer] = j.dump();
		}
	}

	static std::vector<std::string> SplitKey(const std::string& key, char delimiter = '.') {
		std::vector<std::string> result;
		result.reserve(15); 

		size_t start = 0;
		while (true) {
			size_t end = key.find(delimiter, start);
			if (end == std::string::npos) {
				result.emplace_back(key.substr(start));
				break;
			}
			result.emplace_back(key.substr(start, end - start));
			start = end + 1;
		}
		return result;
	}

	static std::string ExtractValuesFrom(const std::string& value, char container = '\"') {
		
		if (value.empty()) return value;

		size_t start = 0;
		size_t end = value.size();

		if (value.front() == container)
			start = 1;
		if (end > start && value.back() == container)
			end--;

		return value.substr(start, end - start);
	}
};