#pragma once
#include "HistoryTimeLineLoader.h"
#include "Static_CurrentTimeLineRep.h"
#include "XAUUSD_OneMinTable.h"
#include "PatternGod.h"
#include "HTTPComputeServer.h"
#include "HTTPComputeServerSyncer.h"
#include "ScoreResultsCachePolynom.h"
#include "ThreadPool.h"

struct ComputeUnitConfigs
{
	std::string historyPathFile;
	std::string scanFilters;

	bool EnableScoreCaching = false;
	bool EnablePolynomTimeLineCaching;
	bool EnablePolynomScan;

	int currentTimeLineSize;
	int polyLen;
	double wEval, wRMSE, wCoeff;
};

class ComputeUnitRunner
{
public:
	void Run() {
		std::thread http_thread(RunServer);

		LoadAllConfigsFromHTTPServer(m_computeUnitConfigs); // Load all configurations from the HTTP server
		InitSingltons();

		std::vector<ChangeNode*> historyTimeLine;
		LoadHistoryData(historyTimeLine);

		std::vector<ChangeNode*> currentTimeLine;
		LoadCurrentTimeLine(currentTimeLine, true);

		PatternGod patternGod(&historyTimeLine, &currentTimeLine, m_computeUnitConfigs.scanFilters, m_computeUnitConfigs.polyLen, m_computeUnitConfigs.EnableScoreCaching, m_computeUnitConfigs.wEval, m_computeUnitConfigs.wRMSE, m_computeUnitConfigs.wCoeff);
		
		XAUUSD_CurrentTable table;
		while (true)
		{
			m_httpServer->Get_RequestHandler()->Wait_For_NewBar_M1(); // Wait for a new M1 bar to be received
			HTTPComputeServerSyncer::UpdateCurrentTimeLine(&table, m_httpServer); // Update the current time line with the new M1 bar
			currentTimeLine = *Static_CurrentTimeLineRep::getInstance()->Get_CurrTimeLine()->Get_TimeLine(); // Get the updated current time line
			patternGod.Set_CurrentTimeLine(&currentTimeLine); // Update the pattern scanner with the new current time line
			
			std::vector<std::pair<ChangeNode*, ScoresResult>> resultNodes2D;
			patternGod.ScanForOneMinPatterns(resultNodes2D); // Scan for patterns with the new current time line
			SubmitScanResult(currentTimeLine.back()->point_DateTime, resultNodes2D); // Submit the scan result for the new M1 bar
		}

		http_thread.join();
	}

private:
	int m_serverPort = 8081;
	HTTPComputeServer* m_httpServer = nullptr;
	ComputeUnitConfigs m_computeUnitConfigs;
	
	void InitSingltons()
	{
		Static_CurrentTimeLineRep::Init(m_computeUnitConfigs.currentTimeLineSize, m_computeUnitConfigs.currentTimeLineSize);
		PolynomTimeLineCache::Init(m_computeUnitConfigs.EnablePolynomTimeLineCaching, m_computeUnitConfigs.EnablePolynomScan, m_computeUnitConfigs.polyLen);
		ScoreResultsCachePolynom::Init(MTThreadPool::getInstance()->Get_ThreadsNum(), 100000);
	}

	void LoadAllConfigsFromHTTPServer(ComputeUnitConfigs& computeUnitConfigs);
	void LoadHistoryData(std::vector<ChangeNode*>& historyTimeLine);
	void LoadCurrentTimeLine(std::vector<ChangeNode*>& currentTimeLine, bool waitTillTimeLineIsFull);

	void SubmitScanResult(std::string currTP, std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D);

	void RunServer() { // Need to run the HTTP server in a separate thread to avoid blocking the main thread
		
		HTTPComputeServer httpsrv(m_serverPort);
		m_httpServer = &httpsrv;
		if (!httpsrv.RunServer()) {
			std::cout << "Failed to start the HTTP Compute Server on port " << m_serverPort << std::endl;
			std::exit(1); // Exit if the server fails to start
		}
	}
private:
	std::string Get_ConfigValue(const std::vector<ConfigStruct>& configs, const std::string& label, const std::string& defaultValue = "");

};

