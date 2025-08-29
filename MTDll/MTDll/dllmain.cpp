// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>
#include <fstream>
#include <string>
#include <codecvt>
#include <vector>
#include "ExtentionF.h"
#include "DataBaseConnection.h"
#include "OrdersToExecute.h"
#include "CandleData.h"
#include "XAUUSD_CurrentTable.h"
#include "XAUUSD_OneMinTable.h"
#include "Logger.h"
#include "JasonHandler.h"
#include "HTTPClient.h"
#include "DLL_OrderStruct.h"

std::string GetTableName(std::string symbol);

// MTSimulator Logics
extern "C" __declspec(dllimport) int CheckForNewOrders_HTTP(DLL_OrderStruct_P order);
extern "C" __declspec(dllimport) int GetOrderDataBySeqNum(int seqNum, DLL_OrderStruct_P order);

extern "C" __declspec(dllimport) bool TruncCurrTableForFreshStart(char* symbol);
extern "C" __declspec(dllimport) bool InsertNewBar_HTTP_M1(char* timePoint, char* symbol, CandleStruct data);
extern "C" __declspec(dllimport) bool InsertNewBar_HTTP_M1_JSON_DB(char* timePoint, char* symbol, CandleStruct data);
extern "C" __declspec(dllimport) bool InsertNewBar_M1_JSON_DB(char* timePoint, char* symbol, CandleStruct data);
extern "C" __declspec(dllimport) double Get_MainTrade_Size();
extern "C" __declspec(dllimport) double Get_SupportTrade_Size();
extern "C" __declspec(dllimport) int GetFutureVision();
extern "C" __declspec(dllimport) void CloseOrder(int seqNum);

extern "C" __declspec(dllimport) int Get_TP_Diff();
extern "C" __declspec(dllimport) int Get_SL_Diff();
extern "C" __declspec(dllimport) int Get_TP_Start();
extern "C" __declspec(dllimport) int Get_SL_Start();
extern "C" __declspec(dllimport) int IsOrderClosed(int seqNum);


/// Start of MTSimulator Functions
bool TruncCurrTableForFreshStart(char* symbol)
{
    //TODO: Function to truncate current tables 
    DataBaseConnection db;
    XAUUSD_CurrentTable table(db.Get_EnvHandler(), db.Get_DBConnection());
    std::string command = "truncate table " + table.Get_TableName();
    return table.FreeStyleCommand(command);

}

void CloseOrder(int seqNum)
{
	Logger& logger = Logger::getInstance();
	logger.WriteToDebug("CloseOrder called with seqNum: " + std::to_string(seqNum));
	logger.WriteToDebug("CloseOrder: Sending HTTP request to close order with seqNum: " + std::to_string(seqNum));
    httplib::Headers header;
    header.insert(std::make_pair(HTTP_PARAM_TradeSeqNum, std::to_string(seqNum)));
    httplib::Result res = HTTPClient::getInstance().Send_Post(HTTP_SERVICE_MTSCanner_CloseOrder, "", "application/json", header);
    if (!res)
    {
        logger.WriteLog("DLL Error while trying to send an HTTP request to server", false, true, LogType::SPECIAL_LOG);
        return;
	}
    logger.WriteToDebug("CloseOrder: Received response from server, checking status");
    if (res->status == 200)
    {
        logger.WriteToDebug("CloseOrder: Order closed successfully");
    }
    else
    {
        logger.WriteToDebug("CloseOrder: Failed to close order, status code: " + std::to_string(res->status));
    }
}

int IsOrderClosed(int seqNum)
{
	Logger& logger = Logger::getInstance();
	logger.WriteToDebug("IsOrderClosed called with seqNum: " + std::to_string(seqNum));
	logger.WriteToDebug("IsOrderClosed: Sending HTTP request to check order status for seqNum: " + std::to_string(seqNum));
	httplib::Headers header;

	header.insert(std::make_pair(HTTP_PARAM_TradeSeqNum, std::to_string(seqNum)));
    httplib::Result res = HTTPClient::getInstance().Send_Get(HTTP_SERVICE_MTScanner_GetOrderData, header);
    if (!res)
    {
       Logger::getInstance().WriteLog("DLL Error while trying to send an HTTP request to server", false, true, LogType::SPECIAL_LOG);
       return 0;
    }
	logger.WriteToDebug("IsOrderClosed: Received response from server, checking status");
    OrderStatus status = OrderStatusUtils::Get_OrderStatus_FromString(res->get_header_value(HTTP_PARAM_STATUS));
	logger.WriteToDebug("IsOrderClosed: Order status received: " + (res->get_header_value(HTTP_PARAM_STATUS)));

    if (status == OrderStatus::CLOSED)
    {
		logger.WriteToDebug("IsOrderClosed: Order is closed, returning 1");
        return 1;
    }
    else
    {
		logger.WriteToDebug("IsOrderClosed: Order is not closed, returning 0");
        return 0;
    }
}

int GetOrderDataBySeqNum(int seqNum,DLL_OrderStruct_P order)
{
	Logger& logger = Logger::getInstance();
	logger.WriteToDebug("GetOrderDataBySeqNum called with seqNum: " + std::to_string(seqNum));
	logger.WriteToDebug("GetOrderDataBySeqNum: Sending HTTP request to get order data for seqNum: " + std::to_string(seqNum));

    httplib::Result res = HTTPClient::getInstance().Send_Get(HTTP_SERVICE_MTScanner_GetOrderData);

    if(!res)
    { 
       Logger::getInstance().WriteLog("DLL Error while trying to send an HTTP request to server", false, true, LogType::SPECIAL_LOG);
       return 0;
    }
	logger.WriteToDebug("GetOrderDataBySeqNum: Received response from server, checking status");

    std::string symbol = res->get_header_value(HTTP_PARAM_Symbol);
    std::string type = res->get_header_value(HTTP_PARAM_TradeType);
    std::string seqNumStr = res->get_header_value(HTTP_PARAM_TradeSeqNum);
    std::string status = res->get_header_value(HTTP_PARAM_STATUS);

    std::string stopLoss = res->get_header_value(HTTP_PARAM_OrderStopLoss);
    std::string takeProfit = res->get_header_value(HTTP_PARAM_OrderTakeProfit);

    order->pair = DLL_CoinsPairUtils::Get_CoinsPair_FromString(symbol);
    order->type = DLL_OrderTypeUtils::Get_OrderType_FromString(type);
    order->status = DLL_OrderStatusUtils::Get_OrderStatus_FromString(status);
    order->seqNum = Converts::ToInt(seqNumStr);
    order->stopLoss = Converts::ToInt(stopLoss);
    order->stopLoss = Converts::ToInt(takeProfit);
	logger.WriteToDebug("GetOrderDataBySeqNum: Order data retrieved successfully");

    return 1;
}

int CheckForNewOrders_HTTP(DLL_OrderStruct_P order)
{
	Logger& logger = Logger::getInstance();

	logger.WriteToDebug("CheckForNewOrders_HTTP called");
	logger.WriteToDebug("Sending HTTP request to check for new trades");

    httplib::Result res = HTTPClient::getInstance().Send_Get(HTTP_SERVICE_IsThereAnTrade);
    if (!res)
    {
        Logger::getInstance().WriteLog("DLL Error while trying to send an HTTP request to server" ,false,true,LogType::SPECIAL_LOG);
        return 0;
    }
	logger.WriteToDebug("Received response from server, checking status");

    std::string type = res->get_header_value(HTTP_PARAM_TradeType);
    order->type = DLL_OrderTypeUtils::Get_OrderType_FromString(type);
    if(order->type != DLL_ORDER_TYPE_LONG && order->type != DLL_ORDER_TYPE_SHORT)
		return 0; // No new order found
	
    logger.WriteToDebug("Order type received: " + type);

    Logger::getInstance().WriteLog("Order type recieve:" + type, false, true, LogType::SPECIAL_LOG);
    std::string symbol = res->get_header_value(HTTP_PARAM_Symbol);
    std::string seqNum = res->get_header_value(HTTP_PARAM_TradeSeqNum);
    std::string stopLoss = res->get_header_value(HTTP_PARAM_OrderStopLoss);
    std::string takeProfit = res->get_header_value(HTTP_PARAM_OrderTakeProfit);
	logger.WriteToDebug("Symbol: " + symbol + ", SeqNum: " + seqNum + ", StopLoss: " + stopLoss + ", TakeProfit: " + takeProfit);
    
    order->pair = DLL_CoinsPairUtils::Get_CoinsPair_FromString(symbol);
    order->type = DLL_OrderTypeUtils::Get_OrderType_FromString(type);
    order->seqNum = Converts::ToInt(seqNum);
    order->status = DLL_OrderStatus::DLL_ORDER_STATUS_OPEN;
    order->stopLoss = Converts::ToInt(stopLoss);
    order->takeProfit = Converts::ToInt(takeProfit);
    
    return 1;
}
bool InsertNewBar_HTTP_M1(char* timePoint, char* symbol, CandleStruct data)
{
    // TODO: insert new fresh record to current table
    nlohmann::json j = JasonHandler::Build_Json_ForCandle(std::string(timePoint), *data);
    std::string body = j.dump();
    httplib::Result res = HTTPClient::getInstance().Send_Post(HTTP_SERVICE_NewBar_M1, body);
    
    if (!res)
    {
        Logger::getInstance().WriteLog("DLL Error while trying to send an HTTP request to server", false, true, LogType::SPECIAL_LOG);
        return false;
    }

    if (res && res->status == 200)
        return true;
    return false;
}

bool InsertNewBar_HTTP_M1_JSON_DB(char* timePoint, char* symbol, CandleStruct data)
{
    // TODO: insert new fresh record to current table
    nlohmann::json j = JasonHandler::Build_Json_ForCandle(std::string(timePoint), *data);
    std::string body = j.dump();

    httplib::Result res = HTTPClient::getInstance().Send_Post(HTTP_SERVICE_NewBar_JSON_DB_M1, body);
    if (!res)
    {
        Logger::getInstance().WriteLog("DLL Error while trying to send an HTTP request to server", false, true, LogType::SPECIAL_LOG);
        return -1;
    }
    if (res && res->status == 200)
        return true;
    return false;
}

bool InsertNewBar_M1_JSON_DB(char* timePoint, char* symbol, CandleStruct data)
{
	Logger& logger = Logger::getInstance();
	logger.WriteToDebug("InsertNewBar_M1_JSON_DB called with timePoint: " + std::string(timePoint) + " and symbol: " + std::string(symbol));
    std::ofstream file("C:\\MTDB\\JsonBackUp_XAUUSD.json", std::ios::app);
    
    if (!file.is_open())
    {
        logger.WriteToDebug("InsertNewBar_M1_JSON_DB: Failed to open file for writing: C:\\MTDB\\JsonBackUp_XAUUSD.json");
        return false;
	}

    logger.WriteToDebug("InsertNewBar_M1_JSON_DB: Building a json for candle data");
    nlohmann::json j = JasonHandler::Build_Json_ForCandle(std::string(timePoint), *data);
	logger.WriteToDebug("InsertNewBar_M1_JSON_DB: JSON built successfully");

    std::string body = j.dump();
    file << body << "\n";
    file.close();

	logger.WriteToDebug("InsertNewBar_M1_JSON_DB: Data written to file successfully");
    return true;

}

double Get_MainTrade_Size()
{
    Logger& logger = Logger::getInstance();

	logger.WriteToDebug("Get_MainTrade_Size called");
	logger.WriteToDebug("Sending HTTP request to get MAIN_ORDER_SIZE_IN_LOT");

    httplib::Headers headers;
    headers.insert(std::make_pair(HTTP_PARAM_ConfigName, "MAIN_ORDER_SIZE_IN_LOT"));
    httplib::Result res = HTTPClient::getInstance().Send_Get(HTTP_SERVICE_MTScanner_GetConfig, headers);
	logger.WriteToDebug("HTTP request sent, waiting for response");

    if (res && res->status == 200)
    {
        std::string value = res->get_header_value(HTTP_PARAM_ConfigValue);
		logger.WriteToDebug("Received response: " + value);
        return std::stod(value);
    }
	logger.WriteToDebug("Failed to get MAIN_ORDER_SIZE_IN_LOT, returning -1");
    return -1;
}

double Get_SupportTrade_Size()
{
	Logger& logger = Logger::getInstance();

	logger.WriteToDebug("Get_SupportTrade_Size: Get_SupportTrade_Size called");
	logger.WriteToDebug("Get_SupportTrade_Size: Sending HTTP request to get SUPPORT_ORDER_SIZE_IN_LOT");
    httplib::Headers headers;
    headers.insert(std::make_pair(HTTP_PARAM_ConfigName, "SUPPORT_ORDER_SIZE_IN_LOT"));
    httplib::Result res = HTTPClient::getInstance().Send_Get(HTTP_SERVICE_MTScanner_GetConfig, headers);

	logger.WriteToDebug("Get_SupportTrade_Size: HTTP request sent, waiting for response");

    if (res && res->status == 200)
    {
        std::string value = res->get_header_value(HTTP_PARAM_ConfigValue);
		logger.WriteToDebug("Get_SupportTrade_Size: Received response: " + value);
        return Converts::ToDouble(value);
		
    }
	logger.WriteToDebug("Get_SupportTrade_Size: Failed to get SUPPORT_ORDER_SIZE_IN_LOT, returning -1");
    return -1;
}
int GetFutureVision()
{
	Logger& logger = Logger::getInstance();
	logger.WriteToDebug("GetFutureVision: GetFutureVision called");
	logger.WriteToDebug("GetFutureVision: Sending HTTP request to get FUTURE_VISION_CANDLES");

    httplib::Headers headers;
    headers.insert(std::make_pair(HTTP_PARAM_ConfigName, "FUTURE_VISION_CANDLES"));
    httplib::Result res = HTTPClient::getInstance().Send_Get(HTTP_SERVICE_MTScanner_GetConfig, headers);

    if (res && res->status == 200)
    {
        std::string value = res->get_header_value(HTTP_PARAM_ConfigValue);
		logger.WriteToDebug("GetFutureVision: Received response: " + value);
        return Converts::ToInt(value);
    }
	logger.WriteToDebug("GetFutureVision: Failed to get FUTURE_VISION_CANDLES, returning default value 10");
    return 10;
}

int Get_SL_Start()
{
	Logger& logger = Logger::getInstance();
	logger.WriteToDebug("Get_SL_Start: Get_SL_Start called");
	logger.WriteToDebug("Get_SL_Start: Sending HTTP request to get STOP_LOSS_START");

    httplib::Headers headers;
    headers.insert(std::make_pair(HTTP_PARAM_ConfigName, "STOP_LOSS_START"));
    httplib::Result res = HTTPClient::getInstance().Send_Get(HTTP_SERVICE_MTScanner_GetConfig, headers);


    if (res && res->status == 200)
    {

        std::string value = res->get_header_value(HTTP_PARAM_ConfigValue);
		logger.WriteToDebug("Get_SL_Start: Received response: " + value);
        return Converts::ToInt(value);
    }
	logger.WriteToDebug("Get_SL_Start: Failed to get STOP_LOSS_START, returning default value 200");
    return 200;
}

int Get_TP_Start()
{
	Logger& logger = Logger::getInstance();
	logger.WriteToDebug("Get_TP_Start: Get_TP_Start called");
	logger.WriteToDebug("Get_TP_Start: Sending HTTP request to get TAKE_PROFIT_START");

    httplib::Headers headers;
    headers.insert(std::make_pair(HTTP_PARAM_ConfigName, "TAKE_PROFIT_START"));
    httplib::Result res = HTTPClient::getInstance().Send_Get(HTTP_SERVICE_MTScanner_GetConfig, headers);

    if (res && res->status == 200)
    {
        std::string value = res->get_header_value(HTTP_PARAM_ConfigValue);
		logger.WriteToDebug("Get_TP_Start: Received response: " + value);
        return Converts::ToInt(value);
    }
	logger.WriteToDebug("Get_TP_Start: Failed to get TAKE_PROFIT_START, returning default value 200");
    return 200;
}


int Get_SL_Diff()
{
	Logger& logger = Logger::getInstance();
	logger.WriteToDebug("Get_SL_Diff: Get_SL_Diff called");
	logger.WriteToDebug("Get_SL_Diff: Sending HTTP request to get STOP_LOSS_DIFF");

    httplib::Headers headers;
    headers.insert(std::make_pair(HTTP_PARAM_ConfigName, "STOP_LOSS_DIFF"));
    httplib::Result res = HTTPClient::getInstance().Send_Get(HTTP_SERVICE_MTScanner_GetConfig, headers);

    if (res && res->status == 200)
    {
        std::string value = res->get_header_value(HTTP_PARAM_ConfigValue);
		logger.WriteToDebug("Get_SL_Diff: Received response: " + value);
        return Converts::ToInt(value);
    }
    return 200;
}

int Get_TP_Diff()
{
	Logger& logger = Logger::getInstance();
	logger.WriteToDebug("Get_TP_Diff: Get_TP_Diff called");
	logger.WriteToDebug("Get_TP_Diff: Sending HTTP request to get TAKE_PROFIT_DIFF");
    httplib::Headers headers;
    headers.insert(std::make_pair(HTTP_PARAM_ConfigName, "TAKE_PROFIT_DIFF"));
    httplib::Result res = HTTPClient::getInstance().Send_Get(HTTP_SERVICE_MTScanner_GetConfig, headers);

    if (res && res->status == 200)
    {
        std::string value = res->get_header_value(HTTP_PARAM_ConfigValue);
		logger.WriteToDebug("Get_TP_Diff: Received response: " + value);
        return Converts::ToInt(value);
    }
	logger.WriteToDebug("Get_TP_Diff: Failed to get TAKE_PROFIT_DIFF, returning default value 200");
    return 200;
}


