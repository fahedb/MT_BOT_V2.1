#pragma once
#include "ExchangeData.h"
#include "XAUUSD_OneMinTable.h"

class HttpServerSyncer
{
public:
	static std::string GetNewBKTSTName() { return ExchangeData().Get_BKTST_StartExporters(); }
	static bool ShouldSubmitLayersExportersData();
	static void UpdateCurrentTimeLine(BasePricesTable* table); // Function to keep the current timeline
	static void UpdateConfigs(); // Updating the config value in the DB through HTTP queue 
	static void LoadAllConfigsToHttpCore(); // Send the configs values to the HTTPCores cache
	static void Set_IsScanning(bool status) { ExchangeData().SetScanningRunning(status); }
	static bool Get_IsScanning() { return ExchangeData().IsScanningRunning(); }
	static void Wait_ForNewBarM1(); // Wait until new M1 bar is available
	static void Notify_DoneScanning() { ExchangeData().Notify_DoneScanning(); }
};

