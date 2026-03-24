#pragma once
#pragma once

#include "httplib.h"
#include "ServucesDefComputeUnit.h"
#include "RequestHandler_ComputeServer.h"
#include <codecvt>
#include <cctype>
class HTTPComputeServer
{
private:
    RequestHandler_ComputeServer m_requests;
    httplib::Server m_srv;
    int m_port = 0;
public:
    HTTPComputeServer(int port)
    {
        // m_srv.set_payload_max_length(10 * 1024 * 1024);
        m_port = port;
        m_srv.Get("/", [](const httplib::Request&, httplib::Response& res) {
            res.set_content("Welcome to MTScanner HTTP Server!\nIf You're not Authorized Kindly Report this on Whatsapp +972 522861739 <3", "text/plain"); // LOLLL !!
            });
    }

public:
    RequestHandler_ComputeServer* Get_RequestHandler() { return &m_requests; }

    bool RunServer()
    {
        m_srv.Post(HTTP_COMPUTE_SERVICE_PostConfigurations, [this](const httplib::Request& req, httplib::Response& res) {
            Post_Configurations(req, res);
            });

        m_srv.Post(HTTP_COMPUTE_SERVICE_StartLoadingHistoryData, [this](const httplib::Request& req, httplib::Response& res) {
			StartLoadingHistoryData(req, res);
			});

        m_srv.Post(HTTP_COMPUTE_SERVICE_Send_NewBar_M1_AndWaitForScanResult, [this](const httplib::Request& req, httplib::Response& res) {
			Send_NewBar_M1_AndWaitForScanResult(req, res);
            });

        m_srv.Post(HTTP_COMPUTE_SERVICE_Send_NewBar_M1_NoWaitForScanResult, [this](const httplib::Request& req, httplib::Response& res) {
            Send_NewBar_M1_NoWaitForScanResult(req, res);
            });
        m_srv.Get(HTTP_COMPUTE_SERVICE_IsReadyToScan, [this](const httplib::Request& req, httplib::Response& res) {
			Get_IsReadyToScan(req, res);
            });

        if (!m_srv.listen("0.0.0.0", m_port))
        {
			std::cerr << "Failed to start server on port " << m_port << std::endl;
            return false;
        }
		return true;
    }

    HTTPComputeServer(const HTTPComputeServer&) = delete;
    HTTPComputeServer& operator=(const HTTPComputeServer&) = delete;


public:
    void Post_Configurations(const httplib::Request& req, httplib::Response& res);
	void StartLoadingHistoryData(const httplib::Request& req, httplib::Response& res);
	void Send_NewBar_M1_AndWaitForScanResult(const httplib::Request& req, httplib::Response& res);
	void Send_NewBar_M1_NoWaitForScanResult(const httplib::Request& req, httplib::Response& res);
	void Get_IsReadyToScan(const httplib::Request& req, httplib::Response& res);
};

void HTTPComputeServer::Post_Configurations(const httplib::Request& req, httplib::Response& res)
{
    m_requests.Handle_SetConfigs(req.body);
    res.set_header("Status", "Success");
}

void HTTPComputeServer::StartLoadingHistoryData(const httplib::Request& req, httplib::Response& res)
{
    m_requests.Handle_StartHistoryLoading(req.body);
    res.set_header("Status", "Success");
}

void HTTPComputeServer::Send_NewBar_M1_AndWaitForScanResult(const httplib::Request& req, httplib::Response& res)
{
    m_requests.Handle_NewBar_M1_AndWaitForScanResult(req.body, res.body);
    res.set_header("Status", "Success");
}
void HTTPComputeServer::Send_NewBar_M1_NoWaitForScanResult(const httplib::Request& req, httplib::Response& res)
{
    m_requests.Handle_NewBar_M1_NoWaitForScanResult(req.body);
    res.set_header("Status", "Success");
}
void HTTPComputeServer::Get_IsReadyToScan(const httplib::Request& req, httplib::Response& res)
{
    bool isReady = m_requests.Handle_IsReadyToScan();
	res.body = isReady ? "True" : "False";
	res.set_header("Status", "Success");
}
