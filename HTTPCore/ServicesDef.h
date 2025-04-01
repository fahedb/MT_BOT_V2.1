#pragma once

// Services 
#define HTTP_SERVICE_MTScanner_Status	"/RunningStatus"
#define HTTP_SERVICE_MTScanner_Run		"/RunScanner"
#define HTTP_SERVICE_MTScanner_Stop		"/StopScanner"

#define HTTP_SERVICE_MTScanner_GetConfig		"/GetConfig"
#define HTTP_SERVICE_MTScanner_GetAllConfigsNames "/GetAllConfigsNames"
#define HTTP_SERVICE_MTScanner_SetConfigValue	"/SetConfigValue"

#define HTTP_SERVICE_IsThereAnTrade		"/IsThereAnTrade"
#define HTTP_SERVICE_NewBar_M1			"/NewBar_M1"
#define HTTP_SERVICE_NewBar_JSON_DB_M1	"/NewBar_JSON_DB_M1"

#define HTTP_SERVICE_Get_OrdersSummey_CSV "/OrdersSummery_CSV"

#define HTTP_SERVICE_TruncateCurrTimeLine_M1 "/TruncateCurrTimeLine_M1" // TODO: Need to implement a logic for this

// Headers
#define HTTP_PARAM_TradeType "TradeType"
#define HTTP_PARAM_Symbol "Symbol"
#define HTTP_PARAM_STATUS "Status"

#define HTTP_PARAM_ConfigName "ConfigName"
#define HTTP_PARAM_ConfigValue "ConfigValue"
#define HTTP_PARAM_ConfigExplain "ConfigExplain"
#define HTTP_PARAM_ConfigPermission "ConfigPermission"

// Headers Values
#define HTTP_PARAM_VALUE_TradeType_Long "Long"
#define HTTP_PARAM_VALUE_TradeType_Short "Short"

#define HTTP_PARAM_VALUE_Status_LoadingData "Loading_Data"
#define HTTP_PARAM_VALUE_Status_Stopped "Stopped"
#define HTTP_PARAM_VALUE_Status_Running "Running"
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

#define HTTP_BODY_MEMBER_IND_LEN_30		"L30"
#define HTTP_BODY_MEMBER_IND_LEN_25		"L25"
#define HTTP_BODY_MEMBER_IND_LEN_20		"L20"
#define HTTP_BODY_MEMBER_IND_LEN_15		"L15"
#define HTTP_BODY_MEMBER_IND_LEN_10		"L10"
