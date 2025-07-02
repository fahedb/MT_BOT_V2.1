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
    httplib::Headers header;
    header.insert(std::make_pair(HTTP_PARAM_TradeSeqNum, std::to_string(seqNum)));
    httplib::Result res = HTTPClient::getInstance().Send_Post(HTTP_SERVICE_MTSCanner_CloseOrder, "", "application/json", header);
}

int IsOrderClosed(int seqNum)
{
    httplib::Result res = HTTPClient::getInstance().Send_Get(HTTP_SERVICE_MTScanner_GetOrderData);
    if (!res)
    {
       Logger::getInstance().WriteLog("DLL Error while trying to send an HTTP request to server", false, true, LogType::SPECIAL_LOG);
       return 0;
    }
    OrderStatus status = OrderStatusUtils::Get_OrderStatus_FromString(res->get_header_value(HTTP_PARAM_STATUS));

    if (status == OrderStatus::CLOSED)
        return 1;
    else
        return 0;

}

int GetOrderDataBySeqNum(int seqNum,DLL_OrderStruct_P order)
{
    httplib::Result res = HTTPClient::getInstance().Send_Get(HTTP_SERVICE_MTScanner_GetOrderData);

    if(!res)
    { 
       Logger::getInstance().WriteLog("DLL Error while trying to send an HTTP request to server", false, true, LogType::SPECIAL_LOG);
       return 0;
    }
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

    return 1;
}

int CheckForNewOrders_HTTP(DLL_OrderStruct_P order)
{
    httplib::Result res = HTTPClient::getInstance().Send_Get(HTTP_SERVICE_IsThereAnTrade);
    if (!res)
    {
        Logger::getInstance().WriteLog("DLL Error while trying to send an HTTP request to server" ,false,true,LogType::SPECIAL_LOG);
        return 0;
    }
    std::string type = res->get_header_value(HTTP_PARAM_TradeType);
    order->type = DLL_OrderTypeUtils::Get_OrderType_FromString(type);
    if(order->type != DLL_ORDER_TYPE_LONG && order->type != DLL_ORDER_TYPE_SHORT)
		return 0; // No new order found
    Logger::getInstance().WriteLog("Order type recieve:" + type, false, true, LogType::SPECIAL_LOG);
    std::string symbol = res->get_header_value(HTTP_PARAM_Symbol);
    std::string seqNum = res->get_header_value(HTTP_PARAM_TradeSeqNum);
    std::string stopLoss = res->get_header_value(HTTP_PARAM_OrderStopLoss);
    std::string takeProfit = res->get_header_value(HTTP_PARAM_OrderTakeProfit);

    order->pair = DLL_CoinsPairUtils::Get_CoinsPair_FromString(symbol);
    order->type = DLL_OrderTypeUtils::Get_OrderType_FromString(type);
    order->seqNum = Converts::ToInt(seqNum);
    order->status = DLL_OrderStatus::DLL_ORDER_STATUS_OPEN;
    order->stopLoss = Converts::ToInt(stopLoss);
    order->stopLoss = Converts::ToInt(takeProfit);
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
    std::ofstream file("C:\\MTDB\\JsonBackUp_XAUUSD.json", std::ios::app);
    // TODO: insert new fresh record to current table
    nlohmann::json j = JasonHandler::Build_Json_ForCandle(std::string(timePoint), *data);
    std::string body = j.dump();
    file << body << "\n";
    file.close();
    return true;

}

double Get_MainTrade_Size()
{
    httplib::Headers headers;
    headers.insert(std::make_pair(HTTP_PARAM_ConfigName, "MAIN_ORDER_SIZE_IN_LOT"));
    httplib::Result res = HTTPClient::getInstance().Send_Get(HTTP_SERVICE_MTScanner_GetConfig, headers);
    
    if (res && res->status == 200)
    {
        std::string value = res->get_header_value(HTTP_PARAM_ConfigValue);
        return std::stod(value);
    }
    return -1;
}
double Get_SupportTrade_Size()
{
    httplib::Headers headers;
    headers.insert(std::make_pair(HTTP_PARAM_ConfigName, "SUPPORT_ORDER_SIZE_IN_LOT"));
    httplib::Result res = HTTPClient::getInstance().Send_Get(HTTP_SERVICE_MTScanner_GetConfig, headers);

    if (res && res->status == 200)
    {
        std::string value = res->get_header_value(HTTP_PARAM_ConfigValue);
        return std::stod(value);
    }
    return -1;
}
int GetFutureVision()
{
    httplib::Headers headers;
    headers.insert(std::make_pair(HTTP_PARAM_ConfigName, "FUTURE_VISION_CANDLES"));
    httplib::Result res = HTTPClient::getInstance().Send_Get(HTTP_SERVICE_MTScanner_GetConfig, headers);

    if (res && res->status == 200)
    {
        std::string value = res->get_header_value(HTTP_PARAM_ConfigValue);
        return std::stoi(value);
    }
    return 10;
}

int Get_SL_Start()
{

    httplib::Headers headers;
    headers.insert(std::make_pair(HTTP_PARAM_ConfigName, "STOP_LOSS_START"));
    httplib::Result res = HTTPClient::getInstance().Send_Get(HTTP_SERVICE_MTScanner_GetConfig, headers);

    if (res && res->status == 200)
    {
        std::string value = res->get_header_value(HTTP_PARAM_ConfigValue);
        return std::stoi(value);
    }
    return 200;
}

int Get_TP_Start()
{
    httplib::Headers headers;
    headers.insert(std::make_pair(HTTP_PARAM_ConfigName, "TAKE_PROFIT_START"));
    httplib::Result res = HTTPClient::getInstance().Send_Get(HTTP_SERVICE_MTScanner_GetConfig, headers);

    if (res && res->status == 200)
    {
        std::string value = res->get_header_value(HTTP_PARAM_ConfigValue);
        return std::stoi(value);
    }
    return 200;
}


int Get_SL_Diff()
{

    httplib::Headers headers;
    headers.insert(std::make_pair(HTTP_PARAM_ConfigName, "STOP_LOSS_DIFF"));
    httplib::Result res = HTTPClient::getInstance().Send_Get(HTTP_SERVICE_MTScanner_GetConfig, headers);

    if (res && res->status == 200)
    {
        std::string value = res->get_header_value(HTTP_PARAM_ConfigValue);
        return Converts::ToInt(value);
    }
    return 200;
}

int Get_TP_Diff()
{
    httplib::Headers headers;
    headers.insert(std::make_pair(HTTP_PARAM_ConfigName, "TAKE_PROFIT_DIFF"));
    httplib::Result res = HTTPClient::getInstance().Send_Get(HTTP_SERVICE_MTScanner_GetConfig, headers);

    if (res && res->status == 200)
    {
        std::string value = res->get_header_value(HTTP_PARAM_ConfigValue);
        return Converts::ToInt(value);
    }
    return 200;
}


