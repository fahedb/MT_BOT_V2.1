#include <string>
#include <vector>
#include <math.h>
#include "IndicatorStructres.h"
#include "nlohmann/json.hpp"
#include "../../HTTPCore/ServicesDef.h"
#include <iostream>
#include <sstream>

#pragma once

#define INVALID_DATA -1;


class ChangeNode
{
public:
	ChangeNode()
	{
		point_DateTime = "";
		Increase_Decrease = false;
		m_PointPlace = _Points::_Points_DEFUALT;
		m_gravity = nullptr;
		Increase_Decrease = false;
		activePoint = false;
	}
	~ChangeNode();
	std::string point_DateTime;

	double Get_Entity(std::string key)
	{
		auto it = m_entitiesMap.find(key);
		if (it != m_entitiesMap.end()) {
			return it->second;
		}
		return -1.0;
	}
	void Set_Entity(const std::string& key, double val) {
		m_entitiesMap[key] = val;
	}
	std::map<std::string, double> m_entitiesMap;
	std::string symbol;
	bool Increase_Decrease = false;
	int	Pip_Change=0;

	bool activePoint=false;
	_Points m_PointPlace;
public:
	void SetGravity(Gravity curr) { m_gravity = new Gravity_Ind(Get_Entity(HTTP_BODY_MEMBER_OpenPrice), Get_Entity(HTTP_BODY_MEMBER_ClosePrice), curr); }
	void SetPoints(_Points points) { m_PointPlace = points; }
	Gravity_Ind* GetGravityInd();
	
	_Points GetPoints() { return m_PointPlace; }
	void Set_Indicator(IndicatorType type, IndicatorLen len, IndicatorValues val, bool forceInsert = false) { m_indMap.Set_Indicator(type, len, val,forceInsert); }
	BaseIndicator* Get_Indicator_Pointer(IndicatorType type, IndicatorLen len) { return m_indMap.Get_Indicator(type, len); }
	IndicatorMap* Get_Indicators_Map() { return &m_indMap; }
private:
	IndicatorMap m_indMap;
	Gravity_Ind* m_gravity;
};

class Http_Utils
{
public:
	static IndicatorType Get_Enum_IndicatorType(const std::string& type)
	{
		if (type == HTTP_BODY_MEMBER_IND_EMA)	return IndicatorType::EMA;
		if (type == HTTP_BODY_MEMBER_IND_RSI)	return IndicatorType::RSI;
		if (type == HTTP_BODY_MEMBER_IND_ZLSMA) return IndicatorType::ZLSMA;

		return IndicatorType::NONE;
	}

	static IndicatorLen Get_Enum_IdicatorLen(const std::string& len)
	{
		if (len == HTTP_BODY_MEMBER_IND_LEN_10) return IndicatorLen::L10;
		if (len == HTTP_BODY_MEMBER_IND_LEN_15) return IndicatorLen::L15;
		if (len == HTTP_BODY_MEMBER_IND_LEN_20) return IndicatorLen::L20;
		if (len == HTTP_BODY_MEMBER_IND_LEN_25) return IndicatorLen::L25;
		if (len == HTTP_BODY_MEMBER_IND_LEN_30) return IndicatorLen::L30;

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

	static void ExtractAllValues(const nlohmann::json_abi_v3_11_3::json& j, std::map<std::string, std::string>& resultMap, const std::string& parentKey = "") {
		if (j.is_object()) {
			for (auto it = j.begin(); it != j.end(); ++it) {
				const std::string& key = it.key();   // המפתח
				const nlohmann::json_abi_v3_11_3::json& value = it.value();     // הערך
				ExtractAllValues(value, resultMap, parentKey.empty() ? key : parentKey + "." + key);
			}
		}
		else if (j.is_array()) {
			for (size_t i = 0; i < j.size(); ++i) {
				ExtractAllValues(j[i], resultMap, parentKey + "[" + std::to_string(i) + "]");
			}
		}
		else {
			resultMap[parentKey] = j.dump(); 
		}
	}
	static std::vector<std::string> SplitKey(const std::string& key, char delimiter = '.')
	{
		std::vector<std::string> result;
		size_t start = 0;
		size_t end = 0;

		while ((end = key.find(delimiter, start)) != std::string::npos) {
			result.emplace_back(key.substr(start, end - start));
			start = end + 1;
		}

		if (start < key.size()) {
			result.emplace_back(key.substr(start));
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