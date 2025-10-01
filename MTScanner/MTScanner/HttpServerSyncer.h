#include "ExchangeData.h"
#include "CONFIGRATIONS.H"
#include "ChangeData.h"
#include "XAUUSD_CurrentTable.h"
#include "XAUUSD_OneMinTable.h"
#include "Static_CurrentTimeLineRep.h"

#pragma once
class HttpServerSyncer
{
public:
	static void UpdateCurrentTimeLine(BasePricesTable* table); // Function to keep the current timeline
	static void UpdateConfigs(); // Updating the config value in the DB through HTTP queue 
	static void LoadAllConfigsToHttpCore(); // Send the configs values to the HTTPCores cache
	static void Set_IsScanning(bool status) { ExchangeData().SetScanningRunning(status); }
	static bool Get_IsScanning() { return ExchangeData().IsScanningRunning(); }
};

