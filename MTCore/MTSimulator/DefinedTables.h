#pragma once


// Tables Names			   ////////////
//		Authour: Fahed				//
//		MasterPeice Soft LTD.		//
//////////////////////////////////////

#define DEF_TABLE_EURUSD_TimeLinePrices "EURUSD_TimeLinePrices"
#define DEF_TABLE_EURUSD_OneMinTimeLinePrices "EURUSD_OneMinTimeLinePrices"
#define DEF_TABLE_EURUSD_CurrentPrices "EURUSD_CurrentPrices"

#define DEF_TABLE_XAUUSD_TimeLinePrices "XAUUSD_TimeLinePrices"
#define DEF_TABLE_XAUUSD_OneMinTimeLinePrices "XAUUSD_OneMinTimeLinePrices"
#define DEF_TABLE_XAUUSD_CurrentPrices "XAUUSD_CurrentPrices"

#define DEF_TABLE_SyncJobs "SyncJobs"
#define DEF_TABLE_OrdersToExecute "OrdersToExecute"

#define DEF_TABLE_Configs "configs"

// Columns Names		  ////////////
//		Authour: Fahed				//
//		MasterPeice Soft LTD.		//
//////////////////////////////////////

#define DEF_COLUMN_PriceDateTime "PriceDateTime"
#define DEF_COLUMN_OpenPrice "OpenPrice"
#define DEF_COLUMN_ClosePrice "ClosePrice"
#define DEF_COLUMN_HighPrice "HighPrice"
#define DEF_COLUMN_LowPrice "LowPrice"

#define DEF_COLUMN_EMAM1 "EMA_M1"
#define DEF_COLUMN_EMAM5 "EMA_M5"
#define DEF_COLUMN_EMAM15 "EMA_M15"
#define DEF_COLUMN_EMAM30 "EMA_M30"
#define DEF_COLUMN_EMAH1 "EMA_H1"
#define DEF_COLUMN_EMAH4 "EMA_H4"

#define DEF_COLUMN_EMAM1_SLOPE "EMA_M1_SLOPE"
#define DEF_COLUMN_EMAM5_SLOPE "EMA_M5_SLOPE"
#define DEF_COLUMN_EMAM15_SLOPE "EMA_M15_SLOPE"
#define DEF_COLUMN_EMAM30_SLOPE "EMA_M30_SLOPE"
#define DEF_COLUMN_EMAH1_SLOPE "EMA_H1_SLOPE"
#define DEF_COLUMN_EMAH4_SLOPE "EMA_H4_SLOPE"

#define DEF_COLUMN_RSIM1 "RSI_M1"
#define DEF_COLUMN_RSIM5 "RSI_M5"
#define DEF_COLUMN_RSIM15 "RSI_M15"
#define DEF_COLUMN_RSIM30 "RSI_M30"
#define DEF_COLUMN_RSIH1 "RSI_H1"
#define DEF_COLUMN_RSIH4 "RSI_H4"

#define DEF_COLUMN_RSIM1_SLOPE "RSI_M1_SLOPE"
#define DEF_COLUMN_RSIM5_SLOPE "RSI_M5_SLOPE"
#define DEF_COLUMN_RSIM15_SLOPE "RSI_M15_SLOPE"
#define DEF_COLUMN_RSIM30_SLOPE "RSI_M30_SLOPE"
#define DEF_COLUMN_RSIH1_SLOPE "RSI_H1_SLOPE"
#define DEF_COLUMN_RSIH4_SLOPE "RSI_H4_SLOPE"

#define DEF_COLUMN_ZLSMAM1 "ZLSMA_M1"
#define DEF_COLUMN_ZLSMAM5 "ZLSMA_M5"
#define DEF_COLUMN_ZLSMAM15 "ZLSMA_M15"
#define DEF_COLUMN_ZLSMAM30 "ZLSMA_M30"
#define DEF_COLUMN_ZLSMAH1 "ZLSMA_H1"
#define DEF_COLUMN_ZLSMAH4 "ZLSMA_H4"

#define DEF_COLUMN_ZLSMAM1_SLOPE "ZLSMA_M1_SLOPE"
#define DEF_COLUMN_ZLSMAM5_SLOPE "ZLSMA_M5_SLOPE"
#define DEF_COLUMN_ZLSMAM15_SLOPE "ZLSMA_M15_SLOPE"
#define DEF_COLUMN_ZLSMAM30_SLOPE "ZLSMA_M30_SLOPE"
#define DEF_COLUMN_ZLSMAH1_SLOPE "ZLSMA_H1_SLOPE"
#define DEF_COLUMN_ZLSMAH4_SLOPE "ZLSMA_H4_SLOPE"

#define DEF_COLUMN_Gravity_Total "Gravity_TotalTicks"
#define DEF_COLUMN_Gravity_Upper "Gravity_UpperTicks"
#define DEF_COLUMN_Gravity_Lower "Gravity_LowerTicks"

#define DEF_COLUMN_Symbol "Symbol"
#define DEF_COLUMN_SyncStatus "SyncStatus"
#define DEF_COLUMN_SyncToTable "SyncToTable"

#define DEF_COLUMN_Long_Short "Long_Short"
#define DEF_COLUMN_ActionSubmitted "ActionSubmitted"

#define DEF_COLUMN_ID "id"
#define DEF_COLUMN_ConfigName "config_name"
#define DEF_COLUMN_ConfigValue "config_value"
#define DEF_COLUMN_ConfigExplain "config_explain"
#define DEF_COLUMN_ConfigPermission "config_permission"


// Columns num			   ////////////
//		Authour: Fahed				//
//		MasterPeice Soft LTD.		//
//////////////////////////////////////


#define DEF_COLUMN_NUM_DEFUALT 4;

#define DEF_COLUMN_NUM_XAUUSD_CURR 45;
#define DEF_COLUMN_NUM_XAUUSD_M1 4;
#define DEF_COLUMN_NUM_XAUUSD_M1_FILE 4;

#define DEF_COLUMN_NUM_EURUSD_M1 4;
#define DEF_COLUMN_NUM_EURUSD_M1_FILE 4;



// Files Columns Indexing ////////////
//		Authour: Fahed				//
//		MasterPeice Soft LTD.		//
//////////////////////////////////////

#define DEF_TimePoint_Index 1
#define DEF_Symbol_Index 2

#define DEF_OpenPrice_Index 3
#define DEF_ClosePrice_Index 4

#define DEF_HighPrice_Index 5
#define DEF_LowPrice_Index 6

#define DEF_RSI_M1_Index 7
#define DEF_RSI_M5_Index 8
#define DEF_RSI_M15_Index 9
#define DEF_RSI_M30_Index 10
#define DEF_RSI_H1_Index 11
#define DEF_RSI_H4_Index 12

#define DEF_EMA_M1_Index 13
#define DEF_EMA_M5_Index 14
#define DEF_EMA_M15_Index 15
#define DEF_EMA_M30_Index 16
#define DEF_EMA_H1_Index 17
#define DEF_EMA_H4_Index 18

#define DEF_ZLSMA_M1_Index 19
#define DEF_ZLSMA_M5_Index 20
#define DEF_ZLSMA_M15_Index 21
#define DEF_ZLSMA_M30_Index 22
#define DEF_ZLSMA_H1_Index 23
#define DEF_ZLSMA_H4_Index 24

#define DEF_GRAVITY_M1_Index 25