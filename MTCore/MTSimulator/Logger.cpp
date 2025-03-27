#include "Logger.h"

/// <Logger>
/// Logger class function, For logging data
/// <Logger>


void Logger::WriteLog(std::string log, bool newSection,bool sendToLogServer, LogType logType)
{
    if (sendToLogServer)
        m_httpLogger->Send_Log(log,"text", httplib::Headers(), logType);
  
    std::string logToPrint = "";
    if (newSection)
        logToPrint += "\n\n\n";
    else
        logToPrint += "\n";

    logToPrint += GetFormattedCurrTime() + log;
    std::cout << logToPrint;
    *(m_logFile) << logToPrint;
    m_logFile->flush();
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
