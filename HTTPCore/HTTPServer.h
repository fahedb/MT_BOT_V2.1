
#include "httplib.h"
#include "ServicesDef.h"
#include "RequestHandler.h"
#include <codecvt>
#include <cctype>
#pragma once
class HTTPServer
{
private:
    RequestHandler m_requests;
    httplib::Server m_srv;
    int m_port = 0;
	HTTPServer()
	{
       // m_srv.set_payload_max_length(10 * 1024 * 1024);
        m_port = 8080;
        m_srv.Get("/", [](const httplib::Request&, httplib::Response& res) {
            res.set_content("Welcome to MTScanner HTTP Server!\nIf You're not Authorized Kindly Report this on Whatsapp +972 522861739 <3", "text/plain"); // LOLLL !!
            });
	}

    void Get_ScanningStatus(const httplib::Request& req, httplib::Response& res) {
        res.set_header(HTTP_PARAM_STATUS, m_requests.GetStatus_Scanning());
    }
    void Stop_Scanning(const httplib::Request& req, httplib::Response& res) { m_requests.Set_RunningStatus(false); }
    void Run_Scanning(const httplib::Request& req, httplib::Response& res) { m_requests.Set_RunningStatus(true); }

    void Handle_Trade(const httplib::Request& req, httplib::Response& res) { m_requests.Handle_IsThereAnTrade(res); }

    void Get_AllConfigsNames_HTTP(const httplib::Request& req, httplib::Response& res) { 
        std::vector<ConfigStruct> confVec =  m_requests.Get_AllConfigs(); 
        std::string body="";
        for (int i = 0; i < confVec.size(); i++)
        {
            if (confVec.at(i).label.size())
                body += confVec.at(i).label + "\n";
        }
        res.body = body;
    }
    void Get_Config_HTTP(const httplib::Request& req, httplib::Response& res)
    {
        std::string configName = req.get_header_value(HTTP_PARAM_ConfigName);
        ConfigStruct confSt = m_requests.Get_Config(configName);
        if (!confSt.label.length())
            return;
        res.set_header(HTTP_PARAM_ConfigName, confSt.label);
        res.set_header(HTTP_PARAM_ConfigValue, confSt.value);
        std::string explain_utf8= convert_windows1255_to_utf8(confSt.explain);
        res.body = explain_utf8;

        res.set_header("Content-Type", "text/plain; charset=UTF-8");
        res.set_header(HTTP_PARAM_ConfigPermission, std::to_string(confSt.permission));

    }

    std::string convert_windows1255_to_utf8(const std::string& input) {
        int wide_size = MultiByteToWideChar(1255, 0, input.c_str(), -1, NULL, 0);
        if (wide_size == 0) return "";

        std::wstring wide_string(wide_size, 0);
        MultiByteToWideChar(1255, 0, input.c_str(), -1, &wide_string[0], wide_size);

        int utf8_size = WideCharToMultiByte(CP_UTF8, 0, wide_string.c_str(), -1, NULL, 0, NULL, NULL);
        if (utf8_size == 0) return "";

        std::string utf8_string(utf8_size, 0);
        WideCharToMultiByte(CP_UTF8, 0, wide_string.c_str(), -1, &utf8_string[0], utf8_size, NULL, NULL);

        return utf8_string;
    }

    void Set_Config_HTTP(const httplib::Request& req, httplib::Response& res)
    {
        std::string label = req.get_header_value(HTTP_PARAM_ConfigName);
        std::string value = req.get_header_value(HTTP_PARAM_ConfigValue);
        std::string result = m_requests.UpdateConfig(label, value) ? "True" : "False";
        res.set_header(HTTP_PARAM_STATUS, result);
    }

    void Handle_NewBarM1(const httplib::Request& req, httplib::Response& res)
    {
        m_requests.Handle_NewBar_M1(req.body);
        res.set_header("Status", "Success");
    }
    void Handle_JSON_DB_NewBarM1(const httplib::Request& req, httplib::Response& res)
    {
        while (m_requests.IsLocked_HistQueue())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Sleep and try again
        }
        m_requests.Handle_JSON_DB_NewBar_M1(req.body);
        res.set_header("Status", "Success");
    }


    void Handle_Get_CSV_OrdersSummery(const httplib::Request& req, httplib::Response& res)
    {
        res.body = "C:\\MTDB\\ordersSummery.csv";
    }
    std::string fix_encoding(const std::string& str) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        std::wstring wide_str = converter.from_bytes(str); // המרה ל-Wide String
        return converter.to_bytes(wide_str); // המרה חזרה ל-UTF-8
    }
public:
    RequestHandler* Get_RequestHandler() { return &m_requests; }
    void RunServer()
    {
        m_srv.Get(HTTP_SERVICE_MTScanner_GetConfig, [this](const httplib::Request& req, httplib::Response& res) {
            this->Get_Config_HTTP(req, res);
            });
                m_srv.Get(HTTP_SERVICE_MTScanner_GetAllConfigsNames, [this](const httplib::Request& req, httplib::Response& res) {
            this->Get_AllConfigsNames_HTTP(req, res);
            });

        m_srv.Post(HTTP_SERVICE_MTScanner_SetConfigValue, [this](const httplib::Request& req, httplib::Response& res) {
            this->Set_Config_HTTP(req, res);
            });


        m_srv.Get(HTTP_SERVICE_MTScanner_Status, [this](const httplib::Request& req, httplib::Response& res) {
            this->Get_ScanningStatus(req,res);
            });

        m_srv.Post(HTTP_SERVICE_MTScanner_Run, [this](const httplib::Request& req, httplib::Response& res) {
            this->Run_Scanning(req, res);
            });

        m_srv.Post(HTTP_SERVICE_MTScanner_Stop, [this](const httplib::Request& req, httplib::Response& res) {
            this->Stop_Scanning(req, res);
            });

        m_srv.Get(HTTP_SERVICE_IsThereAnTrade, [this](const httplib::Request& req, httplib::Response& res) {
            this->Handle_Trade(req, res); // Getting the trade if exists
            });

        m_srv.Post(HTTP_SERVICE_NewBar_M1, [this](const httplib::Request& req, httplib::Response& res) {
            this->Handle_NewBarM1(req, res); // Get new current time line bar
            });   

        m_srv.Post(HTTP_SERVICE_NewBar_JSON_DB_M1, [this](const httplib::Request& req, httplib::Response& res) {
            this->Handle_JSON_DB_NewBarM1(req, res); // Get new history time line bar
            });

        m_srv.Get(HTTP_SERVICE_Get_OrdersSummey_CSV, [this](const httplib::Request& req, httplib::Response& res)
            {
                this->Handle_Get_CSV_OrdersSummery(req, res);
            });
        m_srv.listen("0.0.0.0", m_port);
    }
    HTTPServer(const HTTPServer&) = delete;
    HTTPServer& operator=(const HTTPServer&) = delete;

    // Static method to get the instance of the Singleton
    static HTTPServer* getInstance() {
        static HTTPServer instance; // Guaranteed to be destroyed and instantiated on first use
        return &instance;
    }

};

