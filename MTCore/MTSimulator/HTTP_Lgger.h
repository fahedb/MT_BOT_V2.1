#include "httplib.h"
#include <string>
#pragma once

const std::string logService = "/LogMessage";
const std::string SpecialLogService = "/SpecialLogMessage";
const std::string ErrLogService = "/ErrMessage";
const std::string PatternsExportService = "/PatternsExport";


enum class LogType
{
    ERR = 0,
    LOG = 1,
    SPECIAL_LOG = 2
};

class HTTP_Logger
{
private:

    httplib::Client m_client = httplib::Client("localhost:8081");
    int m_port = 0;
    std::string m_host;
    HTTP_Logger()
    {
        m_client.set_keep_alive(true);
    }
public:

    ~HTTP_Logger()
    {
        m_client.stop();
    }
    HTTP_Logger(const HTTP_Logger&) = delete;
    HTTP_Logger& operator=(const HTTP_Logger&) = delete;

    httplib::Result Send_Log(std::string body = "{}", std::string content_type = "text", httplib::Headers headers = httplib::Headers(),LogType logType = LogType::LOG)
    {
        std::string logSrv;
        if (logType == LogType::LOG)
            logSrv = logService;
        else if (logType == LogType::SPECIAL_LOG)
            logSrv = SpecialLogService;
        else if (logType == LogType::ERR)
            logSrv = ErrLogService;
        else
            logSrv = logService;

        return  m_client.Post(logSrv, headers, body, content_type);
    }

    httplib::Result Export_Patterns_Log(std::string body = "{}", std::string content_type = "text", httplib::Headers headers = httplib::Headers())
    {
        return  m_client.Post(PatternsExportService, headers, body, content_type);
    }
    // Static method to get the instance of the Singleton
    static HTTP_Logger* getInstance() {
        static HTTP_Logger instance; // Guaranteed to be destroyed and instantiated on first use
        return &instance;
    }

};
