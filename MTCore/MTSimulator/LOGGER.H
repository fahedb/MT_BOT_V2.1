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
    void Http_Send_Log(std::string log, LogType logType) { m_httpLogger->Send_Log(log, "text", httplib::Headers(), logType); }
    void Http_Send_Patterns(std::vector<std::string> paths, std::string barP, std::string indL);
    void LogPatternPngs(std::vector<std::string> paths, std::string barP, std::string indL) { std::thread(&Logger::Http_Send_Patterns, this, paths, barP, indL).detach(); }
private:
    std::string GetFormattedCurrTime();
    std::ofstream* m_logFile;
    HTTP_Logger* m_httpLogger;
	std::mutex m_logMutex; // Mutex for thread-safe logging
    Logger()
    {
        std::string fileName = Configrations::getInstance()->Get_Config_Main(CONFIG_DEF_LOG_FILE_NAME);
        m_logFile = new std::ofstream(fileName, std::ios::app);
        m_httpLogger = HTTP_Logger::getInstance();
    }
    ~Logger()
    {
        m_logFile->close();
        delete m_logFile;
    }

};