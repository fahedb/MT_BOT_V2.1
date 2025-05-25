#pragma once

#include <nlohmann/json.hpp>
#include "ServicesDef.h"
#include "httplib.h"
#include <queue>
#include <stack>
#include "..\MTCore\MTSimulator\Converts.h"

struct Http_OrderStruct
{
	std::string symbol;
	int long_short = 0;
	int orderSeqNum = 0;
};

struct ConfigStruct
{
	std::string label;
	std::string value;
	std::string explain;
	int m_permission;
	int m_catagory;
	bool need_To_Update;
};


class RequestHandler
{
private:
	std::queue<std::string> m_candlesBodies;
	std::queue<std::string> m_candlesBodies_history;
	std::vector<ConfigStruct> m_confVec;

	std::stack<Http_OrderStruct> m_ordersToExecute;
	std::mutex m_CurrQueue_locker;
	std::mutex m_HistQueue_locker;
	
	std::mutex m_tradeQueueMutex;


	bool m_runningStatus = true;
	bool m_loadingDataStatus = true;
public:

	int Get_ConfigIdx(std::string config)
	{
		for (int i = 0; i < m_confVec.size(); i++)
		{
			if (m_confVec.at(i).label == config)
				return i;
		}
		return -1;
	}
	std::vector<ConfigStruct> Get_AllConfigs() { return m_confVec; }

	ConfigStruct Get_Config(std::string config)
	{
		int idx = Get_ConfigIdx(config);
		if (idx < 0)
			return ConfigStruct();
		return m_confVec.at(idx);
	}

	bool UpdateConfig(std::string config, std::string value)
	{
		int idx = Get_ConfigIdx(config);
		if (idx >= 0)
		{
			m_confVec.at(idx).value = value;
			m_confVec.at(idx).need_To_Update = true;
			return true;
		}
		return false;
	}

	std::pair<std::string, std::string> GetNextConfigToUpdate()
	{
		for (int i = 0; i < m_confVec.size(); i++)
		{
			if (m_confVec.at(i).need_To_Update)
			{
				m_confVec.at(i).need_To_Update = false;
				return std::make_pair(m_confVec.at(i).label, m_confVec.at(i).value);
			}
		}
		return std::make_pair("", "");
	}	

	void PushNewConfig(std::vector<std::string> conf)
	{
		if (conf.size() < 3)
			return;

		ConfigStruct configStruct;
		configStruct.label = conf.at(0);
		configStruct.value = conf.at(1);
		configStruct.explain = conf.at(2);
		configStruct.m_permission = Converts::ToInt(conf.at(3));
		configStruct.m_catagory = Converts::ToInt(conf.at(4));
		configStruct.need_To_Update = false;

		m_confVec.push_back(configStruct);
	}

	void Handle_NewBar_M1(std::string body)
	{
		if (!body.length())
			return;
		std::lock_guard<std::mutex> lock(m_CurrQueue_locker);
		m_candlesBodies.push(body);
	}

	void Handle_JSON_DB_NewBar_M1(std::string body)
	{
		if (!body.length())
			return;
		std::lock_guard<std::mutex> lock(m_HistQueue_locker);
		m_candlesBodies_history.push(body);
	}

	std::string Get_NextCandle()
	{
		if (!m_candlesBodies.size())
			return "";
		std::lock_guard<std::mutex> lock(m_CurrQueue_locker);

		std::string body = m_candlesBodies.front();
		m_candlesBodies.pop();
		return body;
	}

	std::string Get_Next_DBCandle_M1()
	{
		if (!m_candlesBodies_history.size())
			return "";

		std::lock_guard<std::mutex> lock(m_HistQueue_locker);
		
		std::string body = m_candlesBodies_history.front();
		m_candlesBodies_history.pop();
		return body;
	}

	std::string GetStatus_Scanning() { 
		if (m_loadingDataStatus)
			return HTTP_PARAM_VALUE_Status_LoadingData;
		if (m_runningStatus)
			return HTTP_PARAM_VALUE_Status_Running;
		return HTTP_PARAM_VALUE_Status_Stopped;
	}
	void Set_RunningStatus(bool status) { m_runningStatus = status; }
	void Set_LoadingData(bool status) { m_loadingDataStatus = status; }

	void Handle_IsThereAnTrade(httplib::Response& res) // Returns 
	{
		std::lock_guard<std::mutex> lock(m_tradeQueueMutex);
		if (!m_ordersToExecute.size())
			return;
		Http_OrderStruct newOrder = m_ordersToExecute.top();
	
		while (!m_ordersToExecute.empty()) // i'm using queue for futher usage, but currently i need only the front item
			m_ordersToExecute.pop();
		
		
		res.set_header(HTTP_PARAM_Symbol, newOrder.symbol);
		res.set_header(HTTP_PARAM_TradeType, newOrder.long_short ? HTTP_PARAM_VALUE_TradeType_Short : HTTP_PARAM_VALUE_TradeType_Long);
	}
	void Set_NewOrder(std::string symbol, int long_short)
	{
		std::lock_guard<std::mutex> lock(m_tradeQueueMutex);
		Http_OrderStruct newOrder;
		newOrder.long_short = long_short;
		newOrder.symbol = symbol;
		m_ordersToExecute.push(newOrder);
	}
};

