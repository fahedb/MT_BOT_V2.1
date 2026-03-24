#pragma once
#include "ConfigStruct.h"
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>

class RequestHandler_ComputeServer
{
	std::vector<ConfigStruct> m_confVec;
	std::mutex m_confVecMutex; // Mutex to protect access to m_confVec
	bool m_isConfigsLoaded = false;
	std::condition_variable m_configsLoadedNotifier;


	std::string m_historyFilePath;
	std::condition_variable m_startHistoryLoading;

	std::queue<std::string> m_candlesBodies_history;
	std::mutex m_currQueue_locker;

	bool newBar_M1_Flag = false;
	std::mutex m_newBar_M1_MTX;
	std::condition_variable m_newBarNotifier;

	bool m_scanResultReady = false;
	std::string m_scanResultForNewBar;
	std::mutex m_scanResultMutex;
	std::condition_variable m_scanResultNotifier;

	bool m_isReadyToScan = false;
	std::mutex m_readyToScanMutex;


public: // HTTP Server Side
	void Handle_SetConfig(const std::string& label, const std::string& value);
	void Handle_SetConfigs(const std::string& body);

	void Handle_StartHistoryLoading(const std::string& body);

	void Handle_NewBar_M1_AndWaitForScanResult(const std::string& body, std::string& scanResult); // This function will be called when a new M1 bar is received, it will wait for the scan result and return it in the scanResult parameter
	void Handle_NewBar_M1_NoWaitForScanResult(const std::string& body); // This function will be called when a new M1 bar is received, it will not wait for the scan result and return immediately
	
	bool Handle_IsReadyToScan() { std::lock_guard<std::mutex> lock(m_readyToScanMutex); return m_isReadyToScan; } // This function will be used to check if the server is ready to scan after receiving the new bar, it will return true if the server is ready to scan and false otherwise
public: // Program Side
	std::vector<ConfigStruct> Get_AllConfigs(bool waitForConfigs) {
		if (waitForConfigs)
		{
			std::unique_lock<std::mutex> lock(m_confVecMutex);
			m_configsLoadedNotifier.wait(lock, [this] { return m_isConfigsLoaded; }); // Wait until the configs are loaded
		}
		return m_confVec;
	}
	ConfigStruct Get_Config(const std::string& label);

	void Notify_ScanResult_For_NewBar(const std::string& scanResult); // This function will be used to notify the scanner that the scan result for the new M1 bar is ready and pass the result in the scanResult parameter
	void Wait_For_NewBar_M1(); // This function will be used to wait until a new M1 bar is received and ready for scanning, it will return when the new bar is ready and the flag is set to true
	std::string Get_NextCandleStick_M1_History(); // This function will be used to get the next M1 bar from the history queue, it will return an empty string if the queue is empty
	void Set_isReadyToScan(bool status) { std::lock_guard<std::mutex> lock(m_readyToScanMutex); m_isReadyToScan = status; } // This function will be used to set the server ready to scan after receiving the new bar, it will set the flag to true if the server is ready to scan and false otherwise
private:
	void Notify_NewBar_M1(const std::string& bar); // This function will be used to notify the scanner that a new M1 bar is received and ready for scanning
	void Wait_For_ScanResult(std::string& scanResult); // This function will be used to wait until the scan result for the new M1 bar is ready and return it in the scanResult parameter
};

