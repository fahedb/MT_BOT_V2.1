#include "HTTPServer.h"
#include <nlohmann/json.hpp>
#include <string>
#pragma once


class ExchangeData
{
	HTTPServer* m_httpsrv;
public:
	ExchangeData()
	{
		m_httpsrv = HTTPServer::getInstance();
	}

	void Set_NewOrder(const std::string& symbol, int long_short,int seqNum, int stopLoss, int takeProfit) { m_httpsrv->Get_RequestHandler()->Set_NewOrder(symbol, long_short,seqNum,stopLoss,takeProfit); }
	std::string Get_NextCandleStick_M1() { return m_httpsrv->Get_RequestHandler()->Get_NextCandle(); }
	std::string Get_NextCandleStick_M1_JSON_DB() { return m_httpsrv->Get_RequestHandler()->Get_Next_DBCandle_M1(); }

	bool IsScanningAllowed() { return m_httpsrv->Get_RequestHandler()->GetStatus_Scanning() == HTTP_PARAM_VALUE_Status_Running ? true : false; }
	void DoneLoadingData() { m_httpsrv->Get_RequestHandler()->Set_LoadingData(false); }
	static void Start_DataExchange() { RUN_HTTPSRV();}

	bool IsScanningRunning() { return m_httpsrv->Get_RequestHandler()->Get_IsScanning(); }
	void SetScanningRunning(bool status) { m_httpsrv->Get_RequestHandler()->Set_IsScanning(status); }
	
	void WaitFor_NewBar_M1() { m_httpsrv->Get_RequestHandler()->NewBar_M1_Wait(); }
	void Reset_NewBar_M1_Flag() { m_httpsrv->Get_RequestHandler()->NewBar_M1_ResetFlag(); }

	void Notify_DoneScanning() { m_httpsrv->Get_RequestHandler()->FutureVisionScan_NotifyOne(); }

	void Push_NewConfig(std::vector<std::string> confvec) { m_httpsrv->Get_RequestHandler()->PushNewConfig(confvec); }
	std::pair<std::string, std::string> Get_ConfToUpdate() { return m_httpsrv->Get_RequestHandler()->GetNextConfigToUpdate(); }

	std::string Get_BKTST_SubmissionName() { return m_httpsrv->Get_RequestHandler()->Get_BKTST_SubmitName(); }
	std::string Get_BKTST_StartExporters() { return m_httpsrv->Get_RequestHandler()->Get_BKTST_StartExporters(); }

private:
	static void RUN_HTTPSRV()
	{
		HTTPServer* httpsrv = HTTPServer::getInstance();
		httpsrv->RunServer();
		httpsrv = nullptr;
	}

};

