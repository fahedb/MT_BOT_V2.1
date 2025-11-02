#pragma once

#include <nlohmann/json.hpp>
#include "ServicesDef.h"
#include "httplib.h"
#include <queue>
#include <stack>

struct Http_OrderStruct
{
	std::string symbol;
	int long_short = 0;
	int orderSeqNum = 0;
	int stopLoss = 0;
	int takeProfit = 0;
};

struct ConfigStruct
{
	std::string label;
	std::string value;
	std::string explain;
	std::string policyCategory;
	int m_permission =0 ;
	int m_catagory = 0;
	bool need_To_Update = false;
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

	bool futureVisionScanFlag = false;
	std::mutex futureVisionScanMTX;
	std::condition_variable futureVisionScanCV;

	bool newBar_M1_Flag = false;
	std::mutex newBar_M1_MTX;
	std::condition_variable newBar_M1_CV;


	bool m_runningStatus = true;
	bool m_loadingDataStatus = true;
	std::atomic<bool> m_isScanning = false;

public:
	void FutureVisionScanWait(); // Wait until notified
	void FutureVisionScan_NotifyOne(); // Notify one waiting thread
	void FutureVisionScan_NotifyAll(); // Notify all waiting threads
	void FutureVisionScan_ResetFlag(); // Reset the flag to false

	void Handle_NewBar_M1(const std::string& body);
	void NewBar_M1_Wait(); // Wait until notified
	void NewBar_M1_NotifyOne(); // Notify one waiting thread
	void NewBar_M1_NotifyAll(); // Notify all waiting threads
	void NewBar_M1_ResetFlag(); // Reset the flag to false


	bool Set_IsScanning(bool status) { return m_isScanning.exchange(status); }
	bool Get_IsScanning() { return m_isScanning.load(); }

	std::vector<ConfigStruct> Get_AllConfigs() { return m_confVec; }
	ConfigStruct Get_Config(std::string& config);
	std::pair<std::string, std::string> GetNextConfigToUpdate();
	int Get_ConfigIdx(std::string& config);
	std::string Get_NextCandle();
	std::string Get_Next_DBCandle_M1();
	std::string GetStatus_Scanning();

	bool UpdateConfig(std::string& config, std::string& value);
	void PushNewConfig(std::vector<std::string>& conf);
	
	void Set_RunningStatus(bool status) { m_runningStatus = status; }
	void Set_LoadingData(bool status) { m_loadingDataStatus = status; }
	void Set_NewOrder(const std::string& symbol, int long_short,int seqNum, int stopLoss=0, int takeProfit=0);
	
	void Handle_JSON_DB_NewBar_M1(const std::string& body);
	void Handle_IsThereAnTrade(httplib::Response& res);
	void Handle_CloseOrder(const httplib::Request& req, httplib::Response& res);
	void Handle_GetOrderData(const httplib::Request& req, httplib::Response& res);
	void Handle_IsScanOnGoing(httplib::Response& res);
	void Handle_FutureVisionScan(const httplib::Request& req, httplib::Response& res);

};

