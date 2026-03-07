#include "Logger.h"
#include "ThreadPool.h"
#include <Windows.h>

/// <Logger>
/// Logger class function, For logging data
/// <Logger>
int m_glob_logCounter = 0;
void PrintMemoryUsage();

void Logger::LogPatternPngs(std::vector<std::string> paths, std::string barP, std::string indL) {
    if (!m_logsEnabled)
        return;
    std::thread(&Logger::Http_Send_Patterns, this, paths, barP, indL).detach();
}

void Logger::WriteLog(const std::string& log, bool newSection,bool sendToLogServer, LogType logType)
{
    //m_glob_logCounter++;
    //if(m_glob_logCounter% 100 != 0) // Just to avoid too many logs
	//	return; // temporarily disabled
	if (!m_logsEnabled)
		return;

    if (sendToLogServer && false) // temporarily disabled
    {
        MTThreadPool::getInstance()->enqueueTask([log, logType,this]() {
            try {
                this->Http_Send_Log(log, logType);
            }
            catch (const std::exception& ex) { // Catch any standard exceptions
                // Handle exceptions and log them
                std::cerr << "Exception while sending log to server: " << ex.what() << std::endl;
            }
            }, TaskPriority::Low);
    }
    std::string logToPrint = "";
    if (newSection)
        logToPrint += "\n\n\n";
    else
        logToPrint += "\n";

    logToPrint += GetFormattedCurrTime() + log;
    
    std::lock_guard<std::mutex> lock(m_logMutex);
    OutputDebugStringA(logToPrint.c_str());
    std::cout << logToPrint;
    
    if(m_glob_logCounter > 1000000)
		m_glob_logCounter = 0;
#ifdef PRINT_MEMORY_USAGE
    if (m_glob_logCounter % 100 == 0) // Just to avoid too many logs
        PrintMemoryUsage();
	m_glob_logCounter++;
#endif // PRINT_MEMORY_USAGE

    
}

void Logger::Http_Send_Patterns(std::vector<std::string> paths, std::string barP, std::string indL)
{
    std::string body = "";
    for (int i = 0; i < paths.size(); i++) { body += paths.at(i) + "\n"; }

    httplib::Headers headers;
    headers.insert(std::make_pair("IndicatorLen", indL));
    headers.insert(std::make_pair("BarPeriod", barP));
    m_httpLogger->Export_Patterns_Log(body, "text", headers);
}

std::string Logger::GetFormattedCurrTime()
{
    // Get current time
    std::time_t t = std::time(nullptr);
    std::tm tm; // Thread-safe structure for time
    errno_t err = localtime_s(&tm, &t); // Use localtime_s
    if (err != 0) {
        throw std::runtime_error("Failed to get local time");
    }

    // Format time as HH:MM:SS
    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S : ");
    return oss.str();
}

#ifdef PRINT_MEMORY_USAGE


#include <psapi.h>
#include <iostream>

#pragma comment(lib, "psapi.lib")
std::ofstream* memoryOutFile;

void PrintMemoryUsage()
{
    if(!memoryOutFile)
    {
        memoryOutFile = new std::ofstream();
		memoryOutFile->open("c:\\Temp\\mtScanner_memory_usage.txt", std::ios_base::app);
	}

    PROCESS_MEMORY_COUNTERS_EX pmc;

    if (GetProcessMemoryInfo(GetCurrentProcess(),
        (PROCESS_MEMORY_COUNTERS*)&pmc,
        sizeof(pmc)))
    {
        SIZE_T workingSetMB = pmc.WorkingSetSize / 1024 / 1024;
        SIZE_T privateMB = pmc.PrivateUsage / 1024 / 1024;

		*memoryOutFile << "Working Set: " << workingSetMB << " MB, Private Usage: " << privateMB << " MB" << std::endl;
    }
}
#endif // PRINT_MEMORY_USAGE
