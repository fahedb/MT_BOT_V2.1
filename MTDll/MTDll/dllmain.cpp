// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>
#include <fstream>
#include <string>
#include <codecvt>
#include <vector>
#include "ExtentionF.h"
#include "DataBaseConnection.h"
#include "SynchronizeCurrentDBBase.h"
#include "OrdersToExecute.h"
#include "CandleData.h"
#include "XAUUSD_CurrentTable.h"
#include "XAUUSD_OneMinTable.h"
#include "Logger.h"
#include "JasonHandler.h"
#include "HTTPClient.h"

std::string GetTableName(std::string symbol);

// MTSimulator Logics
extern "C" __declspec(dllimport) int CheckForNewOrders_DB();
extern "C" __declspec(dllimport) int CheckForNewOrders_HTTP();
extern "C" __declspec(dllimport) bool SendNewBarDetailsToFileExtended(char* timePoint, char* symbol, CandleStruct data);
extern "C" __declspec(dllimport) bool TruncCurrTableForFreshStart(char* symbol);
extern "C" __declspec(dllimport) bool InsertNewPriceRecord(char* timePoint, char* symbol, CandleStruct data);
extern "C" __declspec(dllimport) bool InsertNewBar_HTTP_M1(char* timePoint, char* symbol, CandleStruct data);
extern "C" __declspec(dllimport) bool InsertNewBar_HTTP_M1_JSON_DB(char* timePoint, char* symbol, CandleStruct data);
extern "C" __declspec(dllimport) bool InsertNewBar_M1_JSON_DB(char* timePoint, char* symbol, CandleStruct data);
extern "C" __declspec(dllimport) double Get_MainTrade_Size();
extern "C" __declspec(dllimport) double Get_SupportTrade_Size();
extern "C" __declspec(dllimport) int GetFutureVision();

extern "C" __declspec(dllimport) int Get_TP_Diff();
extern "C" __declspec(dllimport) int Get_SL_Diff();
extern "C" __declspec(dllimport) int Get_TP_Start();
extern "C" __declspec(dllimport) int Get_SL_Start();


/// Start of MTSimulator Functions

bool InsertNewPriceRecord(char* timePoint, char* symbol, CandleStruct data)
{
    // TODO: insert new fresh record to current table
    DataBaseConnection db;
    XAUUSD_CurrentTable table(db.Get_EnvHandler(), db.Get_DBConnection());

    SyncJobStructure job;
    job.data = CandleData(*data);
    job.priceDateTime = timePoint;
    job.symbol = symbol;
    job.tableName = table.Get_TableName();
    SynchronizeCurrentDBBase syncer(db.Get_EnvHandler(), db.Get_DBConnection());
    bool res = syncer.InsertionIntoTables(&table, &job);
    db.CloseConnection();
    return res;
}


bool TruncCurrTableForFreshStart(char* symbol)
{
    //TODO: Function to truncate current tables 
    DataBaseConnection db;
    XAUUSD_CurrentTable table(db.Get_EnvHandler(), db.Get_DBConnection());
    std::string command = "truncate table " + table.Get_TableName();
    return table.FreeStyleCommand(command);
}

int CheckForNewOrders_DB()
{
    DataBaseConnection db_Work;
    OrdersToExecute ods(db_Work.Get_EnvHandler(), db_Work.Get_DBConnection());
    NewOrderStructure noData = ods.GetNextOrder();
    db_Work.CloseConnection();
    
    if (!noData.priceDateTime.length())
        return -1;
    return noData.Long_Short;
}


int CheckForNewOrders_HTTP()
{
    httplib::Result res = HTTPClient::getInstance().Send_Get(HTTP_SERVICE_IsThereAnTrade);
    if (!res)
    {
        Logger::getInstance().WriteLog("DLL Error while trying to send an HTTP request to server" ,false,true,LogType::SPECIAL_LOG);
        return -1;
    }
    std::string symbol = res->get_header_value(HTTP_PARAM_Symbol);
    std::string type = res->get_header_value(HTTP_PARAM_TradeType);
    
    if (!symbol.length())
        return -1;
    if (type == HTTP_PARAM_VALUE_TradeType_Long)
        return 0;
    if (type == HTTP_PARAM_VALUE_TradeType_Short)
        return 1;
    return -1;
}
bool InsertNewBar_HTTP_M1(char* timePoint, char* symbol, CandleStruct data)
{
    // TODO: insert new fresh record to current table
    nlohmann::json_abi_v3_11_3::json j = JasonHandler::Build_Json_ForCandle(std::string(timePoint), *data);
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
    nlohmann::json_abi_v3_11_3::json j = JasonHandler::Build_Json_ForCandle(std::string(timePoint), *data);
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
    nlohmann::json_abi_v3_11_3::json j = JasonHandler::Build_Json_ForCandle(std::string(timePoint), *data);
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
        return std::stoi(value);
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
        return std::stoi(value);
    }
    return 200;
}