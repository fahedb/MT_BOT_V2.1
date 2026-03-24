#include "RequestHandler_ComputeServer.h"
#include "ServucesDefComputeUnit.h"
#include <sstream>

void RequestHandler_ComputeServer::Handle_SetConfig(const std::string& label, const std::string& value)
{
	// lock mutex m_confVecMutex
	std::lock_guard<std::mutex> lock(m_confVecMutex);

	for (int i = 0; i < m_confVec.size(); i++)
	{
		if (m_confVec.at(i).label == label)
		{
			m_confVec.at(i).value = value;
			m_confVec.at(i).need_To_Update = true;
			return;
		}
	}
	m_confVec.push_back({ label, value, "", "", 0, 0, true }); // If the config is not found, add it to the vector with need_To_Update set to true
}

void RequestHandler_ComputeServer::Handle_SetConfigs(const std::string& body)
{
	// Assuming the body is in the format "label1=value1\nlabel2=value2\n..."
	std::istringstream stream(body);
	std::string line;
	while (std::getline(stream, line))
	{
		size_t delimiterPos = line.find('=');
		if (delimiterPos != std::string::npos)
		{
			std::string label = line.substr(0, delimiterPos);
			std::string value = line.substr(delimiterPos + 1);
			Handle_SetConfig(label, value); // Reuse the single config handler for each line
		}
	}
	m_configsLoadedNotifier.notify_one(); // Notify that the configs have been loaded and updated
}

void RequestHandler_ComputeServer::Handle_StartHistoryLoading(const std::string& body) {
	// Need to extrace from body path from body, assume format is: HTTP_PARAM_HistoryFilePath=path (body can contain other data but we need to extract only the path)
	std::string path;
	std::istringstream stream(body);
	std::string line;
	while (std::getline(stream, line))
	{
		size_t delimiterPos = line.find('=');
		if (delimiterPos != std::string::npos)
		{
			std::string key = line.substr(0, delimiterPos);
			std::string value = line.substr(delimiterPos + 1);
			if (key == HTTP_PARAM_HistoryFilePath)
			{
				path = value;
				break;
			}
		}
	}
	m_historyFilePath = path;
	m_startHistoryLoading.notify_one();
}


// Program Side
std::vector<ConfigStruct> RequestHandler_ComputeServer::Get_AllConfigs(bool waitForConfigs) {
	if (waitForConfigs)
	{
		std::unique_lock<std::mutex> lock(m_confVecMutex);
		m_configsLoadedNotifier.wait(lock, [this] { return m_isConfigsLoaded; }); // Wait until the configs are loaded
	}
	return m_confVec;
}

ConfigStruct RequestHandler_ComputeServer::Get_Config(const std::string& label)
{
	// lock mutex m_confVecMutex
	std::lock_guard<std::mutex> lock(m_confVecMutex);

	for (int i = 0; i < m_confVec.size(); i++)
	{
		if (m_confVec.at(i).label == label)
		{
			return m_confVec.at(i);
		}
	}
	return ConfigStruct(); // Return an empty ConfigStruct if the config is not found
}

void RequestHandler_ComputeServer::Notify_ScanResult_For_NewBar(const std::string& scanResult) // This function will be used to notify the scanner that the scan result for the new M1 bar is ready and pass the result in the scanResult parameter
{
	{
		std::lock_guard<std::mutex> lock(m_scanResultMutex);
		m_scanResultForNewBar = scanResult;
		m_scanResultReady = true;
	}
	m_scanResultNotifier.notify_one();
}

void RequestHandler_ComputeServer::Wait_For_NewBar_M1() {
	std::unique_lock<std::mutex> lock(m_newBar_M1_MTX);
	m_newBarNotifier.wait(lock, [this] { return newBar_M1_Flag; }); // Wait until a new M1 bar is received
	newBar_M1_Flag = false; // Reset the flag for the next use
}

// This function will be used to wait until a new M1 bar is received and ready for scanning, it will return when the new bar is ready and the flag is set to true
std::string RequestHandler_ComputeServer::Get_NextCandleStick_M1_History() { 
	
	std::lock_guard<std::mutex> lock(m_currQueue_locker); 
	
	if (m_candlesBodies_history.empty()) return ""; 
	
	std::string body = m_candlesBodies_history.front();
	m_candlesBodies_history.pop();
	return body; }

void RequestHandler_ComputeServer::Handle_NewBar_M1_AndWaitForScanResult(const std::string& body, std::string& scanResult)
{
	Notify_NewBar_M1(body); // Notify the scanner that a new M1 bar is received and ready for scanning
	Wait_For_ScanResult(scanResult); // Wait until the scan result for the new M1 bar is ready and return it in the scanResult parameter
}

void RequestHandler_ComputeServer::Handle_NewBar_M1_NoWaitForScanResult(const std::string& body)
{
	Notify_NewBar_M1(body); // Notify the scanner that a new M1 bar is received and ready for scanning
}

void RequestHandler_ComputeServer::Notify_NewBar_M1(const std::string& bar)
{
	{
		std::lock_guard<std::mutex> lock(m_currQueue_locker);
		m_candlesBodies_history.push(bar); // Push new candle body to queue

		std::lock_guard<std::mutex> lock(m_newBar_M1_MTX);
		newBar_M1_Flag = true;
	}
	m_newBarNotifier.notify_one();
}

void RequestHandler_ComputeServer::Wait_For_ScanResult(std::string& scanResult)
{
	std::unique_lock<std::mutex> lock(m_scanResultMutex);
	m_scanResultNotifier.wait(lock, [this] { return m_scanResultReady; }); // Wait until the scan result is ready
	scanResult = m_scanResultForNewBar; // Get the scan result
	m_scanResultReady = false; // Reset the flag for the next use
}

