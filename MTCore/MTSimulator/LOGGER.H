#define _CRT_SECURE_NO_WARNINGS

#pragma once
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "Configrations.h"
#include "HTTP_Lgger.h"

/// <Logger>
/// Logger class function, For logging data
/// <Logger>

class Logger
{
public:
    // Deleted copy constructor and assignment operator to prevent copies
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // Static method to get the instance of the Singleton
    static Logger& getInstance() {
        static Logger instance; // Guaranteed to be destroyed and instantiated on first use
        return instance;
    }
    void WriteLog(std::string log, bool newSection = false, bool sendToLogServer = false, LogType logType = LogType::LOG);
    void LogPatternPngs(std::vector<std::string> paths,std::string barP,std::string indL)
    {
        std::string body = "";
        for (int i = 0; i < paths.size(); i++) { body += paths.at(i) + "\n"; }

        httplib::Headers headers;
        headers.insert(std::make_pair("IndicatorLen", indL));
        headers.insert(std::make_pair("BarPeriod", barP));
        m_httpLogger->Export_Patterns_Log(body, "text", headers);
    }
private:
    std::string GetFormattedCurrTime();
    std::ofstream* m_logFile;
    HTTP_Logger* m_httpLogger;
    Logger()
    {
        std::string fileName = Configrations::getInstance()->Get_ConfigValue(CONFIG_DEF_LOG_FILE_NAME);
        m_logFile = new std::ofstream(fileName, std::ios::app);
        m_httpLogger = HTTP_Logger::getInstance();
    }
    ~Logger()
    {
        m_logFile->close();
        delete m_logFile;
    }

};