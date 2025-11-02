#pragma once

// Services 
#define HTTP_SERVICE_MTScanner_Status	"/RunningStatus"
#define HTTP_SERVICE_MTScanner_Run		"/RunScanner"
#define HTTP_SERVICE_MTScanner_Stop		"/StopScanner"

#define HTTP_SERVICE_MTScanner_GetOrderData "/GetOrderData"
#define HTTP_SERVICE_MTSCanner_CloseOrder		"/CloseOrder"

#define HTTP_SERVICE_MTScanner_GetConfig		"/GetConfig"
#define HTTP_SERVICE_MTScanner_GetAllConfigsNames "/GetAllConfigsNames"
#define HTTP_SERVICE_MTScanner_SetConfigValue	"/SetConfigValue"

#define HTTP_SERVICE_IsThereAnTrade		"/IsThereAnTrade"
#define HTTP_SERVICE_NewBar_M1			"/NewBar_M1"
#define HTTP_SERVICE_NewBar_JSON_DB_M1	"/NewBar_JSON_DB_M1"
#define HTTP_SERVICE_IsScanOnGoing		"/IsScanOnGoing"
#define HTTP_SERVICE_Get_FutureVision	"/GetFutureVision"

#define HTTP_SERVICE_Get_OrdersSummey_CSV "/OrdersSummery_CSV"

#define HTTP_SERVICE_TruncateCurrTimeLine_M1 "/TruncateCurrTimeLine_M1" // TODO: Need to implement a logic for this

// Headers
#define HTTP_PARAM_TradeType "TradeType"
#define HTTP_PARAM_Symbol "Symbol"
#define HTTP_PARAM_STATUS "Status"
#define HTTP_PARAM_TradeSeqNum "TradeSeqNum"
#define HTTP_PARAM_OrderStopLoss "OrderStopLoss"
#define HTTP_PARAM_OrderTakeProfit "OrderTakeProfit"
#define HTTP_PARAM_OrderStatus "OrderStatus"
#define HTTP_PARAM_IsScanOnGoing "IsScanOnGoing"
#define HTTP_PARAM_ConfigName "ConfigName"
#define HTTP_PARAM_ConfigValue "ConfigValue"
#define HTTP_PARAM_ConfigExplain "ConfigExplain"
#define HTTP_PARAM_ConfigPolicyCategory "ConfigPolicyCategory"
#define HTTP_PARAM_ConfigPermission "ConfigPermission"
#define HTTP_PARAM_ConfigCatagory "ConfigCatagory"

// Headers Values
#define HTTP_PARAM_VALUE_TradeType_Long "Long"
#define HTTP_PARAM_VALUE_TradeType_Short "Short"

#define HTTP_PARAM_VALUE_OrderStatus_Active "OPEN"
#define HTTP_PARAM_VALUE_OrderStatus_Closed "CLOSED"

#define HTTP_PARAM_VALUE_Status_LoadingData "Loading_Data"
#define HTTP_PARAM_VALUE_Status_Stopped "Stopped"
#define HTTP_PARAM_VALUE_Status_Running "Running"

#define HTTP_PARAM_VALUE_True "True"
#define HTTP_PARAM_VALUE_False "False"

#define HTTP_PARAM_MEMBER_Indicator_Len	"IndicatorLen"
#define HTTP_PARAM_MEMBER_Bar_Period	"BarPeriod"

#define HTTP_BODY_MEMBER_Symbol			"symbol"
#define HTTP_BODY_MEMBER_OpenPrice		"OpenPrice"
#define HTTP_BODY_MEMBER_ClosePrice		"ClosePrice"

#define HTTP_BODY_MEMBER_HighPrice		"HighPrice"
#define HTTP_BODY_MEMBER_LowPrice		"LowPrice"

#define HTTP_BODY_MEMBER_Period_M1		"M1"
#define HTTP_BODY_MEMBER_Period_M5		"M5"
#define HTTP_BODY_MEMBER_Period_M15		"M15"
#define HTTP_BODY_MEMBER_Period_M30		"M30"
#define HTTP_BODY_MEMBER_Period_H1		"H1"
#define HTTP_BODY_MEMBER_Period_H4		"H4"

#define HTTP_BODY_MEMBER_IND_FATHER		"indicators"

#define HTTP_BODY_MEMBER_IND_VALUES		"Values"
#define HTTP_BODY_MEMBER_IND_SLOPES		"Slopes"

#define HTTP_BODY_MEMBER_IND_EMA		"EMA"
#define HTTP_BODY_MEMBER_IND_RSI		"RSI"
#define HTTP_BODY_MEMBER_IND_ZLSMA		"ZLSMA"
#define HTTP_BODY_MEMBER_IND_VOLUME		"VOLUME"

#define HTTP_BODY_MEMBER_IND_LEN_30		"L30"
#define HTTP_BODY_MEMBER_IND_LEN_25		"L25"
#define HTTP_BODY_MEMBER_IND_LEN_20		"L20"
#define HTTP_BODY_MEMBER_IND_LEN_15		"L15"
#define HTTP_BODY_MEMBER_IND_LEN_10		"L10"

#define HTTP_BODY_MEMBER_IND_LEN_NONE	"NONE"


#define JSON_BODY_MEMBER_TIMEPOINT		"TimePoint"

#define JSON_BODY_MEMBER_PATTERNS_COUNT		"PatternsCount"


#define JSON_BODY_MEMBER_GENERAL		"General"
#define JSON_BODY_MEMBER_FIRST			"First"

#define JSON_BODY_MEMBER_UP			"UP"
#define JSON_BODY_MEMBER_DOWN		"DOWN"
#define JSON_BODY_MEMBER_NOISE		"NOISE"
#define JSON_BODY_MEMBER_MEAN		"MEAN"
#define JSON_BODY_MEMBER_STDDEV		"StdDev"
#define JSON_BODY_MEMBER_MEAN_INDEXES "MEAN_INDEXES"

#define JSON_BODY_MEMBER_CANDLE_INDEX_DETAILS "CANLE_INDEX_DETAILS"

