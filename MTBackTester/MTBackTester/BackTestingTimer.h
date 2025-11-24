#pragma once
#include "PatternGodScanner.h"
#include "HttpServerSyncer.h"
#include "MetaTraderSimulator.h"

class BackTestingTimer
{
public:
	BackTestingTimer(std::vector<ChangeNode*>* backTestData, DBMap* backTestDataDBMap)
		: m_backTestData(backTestData),
		m_dateTimeToIdxMap(backTestDataDBMap)
	{
	}
	void Run()
	{
		MetaTraderSimulator mtSim(1, 10000.0, 0.1, 100.0, "XAUUSD");

		PatternGodScanner scanner;
		scanner.Set_ExternalCurrTimeLineQueue(true);

		std::thread scannerThread([&scanner]() {
			scanner.Run(); // Run the scanner in a separate thread
			});
		size_t totalBars = m_backTestData->size();

		Static_CurrentTimeLineRep* currTimeLineRep = Static_CurrentTimeLineRep::getInstance();
		ExchangeData dataExch;
		for (int i = 0; i < totalBars; i++)
		{
			dataExch.FutureVisionScan_ResetNewBarFlag(); // Reset the new bar flag before pushing a new bar
			
			currTimeLineRep->Set_NewBar(m_backTestData->at(i), true);
			mtSim.Set_NewCurrBarData(m_backTestData->at(i));
			dataExch.Notify_NewBar_M1(); // Notify the scanner of the new bar

			dataExch.Wait_ForScanToComplete(); // Wait for the scanner to process the new bar
			mtSim.Set_NewOrder(scanner.Get_LastPlacedOrder()); // If there is a new order, set it in the simulator
			
		}



		//OutputDebugStringA("ServerFlag: Future vision Resetting flag");
		//FutureVisionScan_ResetFlag(); // Reset the flag to false before waiting
		//OutputDebugStringA("ServerFlag: Handling new bar");
		//Handle_NewBar_M1(req.body); // Push new bar to queue and notify the scanner
		//OutputDebugStringA("ServerFlag: Waiting for scan to finish");
		//FutureVisionScanWait(); // Wait till notified that scan is done
		//OutputDebugStringA("ServerFlag: Checking results");
		//Handle_IsThereAnTrade(res); // Check if there is an trade to execute after the scan
		
	}
private:
	std::vector<ChangeNode*>* m_backTestData;
	DBMap* m_dateTimeToIdxMap;

private:
};

