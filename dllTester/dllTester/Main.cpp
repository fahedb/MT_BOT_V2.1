#define _CRT_SECURE_NO_WARNINGS
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include <codecvt>

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
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "ExtentionF.h"
#include "DataBaseConnection.h"
#include "SynchronizeCurrentDBBase.h"
#include "OrdersToExecute.h"
#include "CandleData.h"
#include "XAUUSD_CurrentTable.h"
#include "HTTPClient.h"
#include "JasonHandler.h"
double Get_MainTrade_Size();
double Get_SupportTrade_Size();

bool SendNewBarDetails(char* timePoint, char* symbol, char* openPrice, char* closePrice);
bool SendNewBarDetailsToFile(char* timePoint, char* symbol, char* openPrice, char* closePrice, char* highPrice, char* lowPrice, char* rsi_M1, char* rsi_M5, char* rsi_M15, char* rsi_M30, char* rsi_H1, char* rsi_H4, char* ema_M1, char* ema_M5, char* ema_M15, char* ema_M30, char* ema_H1, char* ema_H4, char* zlsma_M1, char* zlsma_M5, char* zlsma_M15, char* zlsma_M30, char* zlsma_H1, char* zlsma_H4, char* gravity_M1);
bool SendNewBarDetailsToFileExtended(char* timePoint, char* symbol, CandleStruct data);
bool TruncCurrTableForFreshStart(char* symbol);
bool InsertNewPriceRecord(char* timePoint, char* symbol, CandleStruct data);
bool InsertNewBar_HTTP_M1_JSON_DB(char* timePoint, char* symbol, CandleStruct data);
const wchar_t* CheckForNewOrders_HTTP();


void main()
{
    double mainS = Get_MainTrade_Size();
    double suppS = Get_SupportTrade_Size();

}
/// External functions to call
// dllmain.cpp : Defines the entry point for the DLL application.

/// Start of MTSimulator Functions

bool InsertNewBar_HTTP_M1(char* timePoint, char* symbol, CandleStruct data)
{
    // TODO: insert new fresh record to current table
    nlohmann::json_abi_v3_11_3::json j = JasonHandler::Build_Json_ForCandle(std::string(timePoint), *data);
    std::string body = j.dump();
    httplib::Result res = HTTPClient::getInstance().Send_Post(HTTP_SERVICE_NewBar_M1, body);
    if (res && res->status == 200)
        return true;
    return false;
}

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


bool SendNewBarDetailsToFileExtended(char* timePoint, char* symbol, CandleStruct data)
{
    DataBaseConnection db;
    XAUUSD_CurrentTable table(db.Get_EnvHandler(), db.Get_DBConnection());

    SyncJobStructure job;
    job.data = CandleData(*data);
    job.priceDateTime = timePoint;
    job.symbol = symbol;
    job.tableName = table.Get_TableName();

    SynchronizeCurrentDBBase syncer(db.Get_EnvHandler(), db.Get_DBConnection());
    bool res = syncer.SyncCurrentTables(&table, &job);
    db.CloseConnection();
    return res;
}
bool SendNewBarDetails(char* timePoint, char* symbol, char* openPrice, char* closePrice, char* rsi)
{
    DataBaseConnection Db_Work;
    SynchronizeCurrentDBBase syncer(Db_Work.Get_EnvHandler(), Db_Work.Get_DBConnection());
    SyncJobStructure job;
    job.priceDateTime = timePoint;
    job.symbol = symbol;
    job.OpenPrice = openPrice;
    job.ClosePrice = closePrice;
    job.tableName = GetTableName(symbol);
    //job.rsi. = std::stod(std::string(rsi));// Need to implement this in the DB and the insert // will do that when i move to the tree section
    bool res = syncer.FeedDataToCurrentTablesJobs(job);
    Db_Work.CloseConnection();
    return res;
}

bool SendNewBarDetailsToFile(char* timePoint, char* symbol, char* openPrice, char* closePrice, char* highPrice, char* lowPrice, char* rsi_M1, char* rsi_M5, char* rsi_M15, char* rsi_M30, char* rsi_H1, char* rsi_H4, char* ema_M1, char* ema_M5, char* ema_M15, char* ema_M30, char* ema_H1, char* ema_H4, char* zlsma_M1, char* zlsma_M5, char* zlsma_M15, char* zlsma_M30, char* zlsma_H1, char* zlsma_H4, char* gravity_M1)
{

    SyncJobStructure job = InializeJobObject(timePoint, symbol, openPrice, closePrice, highPrice, lowPrice, rsi_M1, rsi_M5, rsi_M15, rsi_M30, rsi_H1, rsi_H4, ema_M1, ema_M5, ema_M15, ema_M30, ema_H1, ema_H4, zlsma_M1, zlsma_M5, zlsma_M15, zlsma_M30, zlsma_H1, zlsma_H4, gravity_M1);
    std::string lineToWrite = InitLineToWrite(job);

    std::string fileName = Configrations::getInstance()->Get_DB_FileDir();
    fileName = fileName + symbol + "_Report";
    std::ofstream file(fileName, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << std::endl;
        return false;
    }
    file << lineToWrite << "\n";
    file.close();
    return true;
}

const wchar_t* CheckForNewOrders_HTTP()
{
    httplib::Result res = HTTPClient::getInstance().Send_Get(HTTP_SERVICE_IsThereAnTrade);
    if (res)
    {
        std::string symbol = res->get_header_value(HTTP_PARAM_Symbol);
        std::string type = res->get_header_value(HTTP_PARAM_TradeType);
        std::string strToReturn;
    
        if (symbol.length())
            strToReturn = "1," + symbol + "," + type;
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        static std::wstring last_message;
        last_message = converter.from_bytes(strToReturn);
        return last_message.c_str();
    }
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    static std::wstring last_message;
    last_message = converter.from_bytes("");
    return last_message.c_str();
}

bool SendTickDetails(char* timePoint, char* symbol, char* openPrice, char* closePrice)
{
    DataBaseConnection Db_Work;
    SynchronizeCurrentDBBase syncer(Db_Work.Get_EnvHandler(), Db_Work.Get_DBConnection());
    SyncJobStructure job;
    job.priceDateTime = timePoint;
    job.symbol = symbol;
    job.OpenPrice = openPrice;
    job.ClosePrice = closePrice;
    job.tableName = GetTableName(symbol);
    syncer.FeedDataToCurrentTablesJobs(job);
    Db_Work.CloseConnection();
    return true;
}
bool SendTickDetailsToFile(char* timePoint, char* symbol, char* openPrice, char* closePrice, char* rsi)
{
    SyncJobStructure job;
    job.priceDateTime = timePoint;
    job.symbol = symbol;
    job.OpenPrice = openPrice;
    job.ClosePrice = closePrice;
    job.tableName = GetTableName(symbol);
    //job.rsi.M1 = std::stod(std::string(rsi));
    std::string lineToWrite = job.priceDateTime + "," + symbol + "," + openPrice + "," + closePrice + "," + rsi;

    std::string fileName = Configrations::getInstance()->Get_DB_FileDir();
    fileName = fileName + symbol + "_Report";
    std::ofstream file(fileName, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << std::endl;
        return false;
    }
    file << lineToWrite << "\n";
    file.close();
    return true;
}


bool InsertNewBar_HTTP_M1_JSON_DB(char* timePoint, char* symbol, CandleStruct data)
{
    std::ofstream file("C:\\MTDB\\logdll.txt", std::ios::app);
    file << "OPEN PRICE " << std::string(data->openPrice, 7) << "\n";
    // TODO: insert new fresh record to current table
    nlohmann::json_abi_v3_11_3::json j = JasonHandler::Build_Json_ForCandle(std::string(timePoint), *data);
    std::string body = j.dump();
    file << body << "\n";
    file.close();
    httplib::Result res = HTTPClient::getInstance().Send_Post(HTTP_SERVICE_NewBar_JSON_DB_M1, body);
    if (res && res->status == 200)
        return true;
    return false;
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





////////////////////////////////////////////
////////////////////////////////////////////
////////////OLD FUNCTIONS///////////////////
////////////////////////////////////////////
////////////////////////////////////////////


const wchar_t* ReadLastOrderFromFile()
{
    static std::wstring last_message;
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    last_message = converter.from_bytes(ReadLastOrder(FILE_NAME));
    return last_message.c_str();
}

bool UpdateLineFromFile(char* buffer, int ticketNum)
{

    std::vector<std::string> fileContect = CopyFileContent(FILE_NAME, buffer);
    std::string orderID(buffer);

    if (!FindAndUpdateLine(&fileContect, orderID, buffer, ticketNum))
    {
        return false;
    }
    if (!OverWriteFile(FILE_NAME, fileContect))
        return false;
    return true;
}

bool CopyNewOrder(char* orderLine, char* orderTicket)
{
    std::string orderNum(orderTicket);
    if (findOrder(orderNum, FILE_NAME))
        return true;
    std::string orderL(orderLine);
    return insetOrder(orderL, FILE_NAME);
}

bool SetCancledOrder(char* ticketNum)
{
    std::string orderNum(ticketNum);
    std::vector<std::string> fileContect;
    if (!FindCanceledOrder(orderNum))
    {
        std::string orderID(ticketNum);
        fileContect = CopyFileContent(FILE_NAME, ticketNum);
        if (!UpdateCancelOrder(&fileContect, orderNum))
            return false;
        if (!OverWriteFile(FILE_NAME, fileContect))
            return false;
    }

    return true;
}

const wchar_t* GetCanceledOrder()
{
    static std::wstring last_message;
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    last_message = converter.from_bytes(CanceledOrder());
    return last_message.c_str();
}

bool IsOrderOpen(char* ticketNum)
{
    std::ifstream file(FILE_NAME);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << std::endl;
        return "";
    }

    std::string lastLine = "";
    std::string line;
    int lineNum = 0;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            if (line.find(ticketNum) >= 0) {

                int index = line.find(TO_CANCELL_ORDER);
                if (index >= 0)
                {
                    return false;

                }
                else
                    return true;
                lineNum++;
            }
        }
    }
    file.close();
    return false;
}

const wchar_t* GetOrderByLine(int lineNum)
{
    std::ifstream file(FILE_NAME);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << std::endl;

        static std::wstring last_message;
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        last_message = converter.from_bytes("");
        return last_message.c_str();
    }

    std::string lastLine = "";
    std::string line;
    int currLineNum = 0;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            if (lineNum == currLineNum) {
                int idx = line.find("XX");
                if (idx >= 0)
                    break;
                lastLine = line;
                break;
            }
            currLineNum++;
        }
    }
    file.close();

    static std::wstring last_message;
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    last_message = converter.from_bytes(lastLine);
    return last_message.c_str();
}

int TotalOpenOrdersCount()
{

    std::ifstream file(FILE_NAME);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << std::endl;

        return -1;
    }

    std::string lastLine = "";
    std::string line;
    int currLineNum = 0;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            currLineNum++;
        }
    }
    file.close();
    return currLineNum;
}

