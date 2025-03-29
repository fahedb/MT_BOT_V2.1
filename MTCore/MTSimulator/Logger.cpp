#include "Logger.h"

/// <Logger>
/// Logger class function, For logging data
/// <Logger>


void Logger::WriteLog(std::string log, bool newSection,bool sendToLogServer, LogType logType)
{
    if (sendToLogServer)
    {
        std::thread(&Logger::Http_Send_Log, this, log, logType).detach();
       // m_httpLogger->Send_Log(log, "text", httplib::Headers(), logType);
    }
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
