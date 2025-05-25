#include "httplib.h"
#include <string>
#pragma once

const std::string logService = "/LogMessage";
const std::string SpecialLogService = "/SpecialLogMessage";
const std::string ErrLogService = "/ErrMessage";
const std::string TradeTraceLogService = "/TraceTradeLogMessage";
const std::string PatternsExportService = "/PatternsExport";
const std::string BestDirectionSummeryService = "/BestDirectionSummery";


enum class LogType
{
    ERR = 0,
    LOG = 1,
    SPECIAL_LOG = 2,
    TRADE_TRACE_LOG = 3,
    SUMMERY_LOG = 4
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
        std::string logSrv = GetService(logType);
        return  m_client.Post(logSrv, headers, body, content_type);
    }

    httplib::Result Export_Patterns_Log(std::string body = "{}", std::string content_type = "text", httplib::Headers headers = httplib::Headers())
    {
        return  m_client.Post(PatternsExportService, headers, body, content_type);
    }

    std::string GetService(LogType logType)
    {
        switch (logType)
        {
        case LogType::ERR:
            return ErrLogService;

        case LogType::LOG:
            return logService;

        case LogType::SPECIAL_LOG:
            return SpecialLogService;

        case LogType::TRADE_TRACE_LOG:
            return TradeTraceLogService;

        case LogType::SUMMERY_LOG:
            return BestDirectionSummeryService;

        default:
            return logService;
        }
        return logService;
    }

    // Static method to get the instance of the Singleton
    static HTTP_Logger* getInstance() {
        static HTTP_Logger instance; // Guaranteed to be destroyed and instantiated on first use
        return &instance;
    }

};
