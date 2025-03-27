#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <codecvt>
#include <vector>
#include "SyncJobs_Table.h"
#include <sstream>

#define COMMA ","
const char* FILE_NAME = "C:\\Test\\data.csv";
const char* REGISTERED_ORDER = "VV";
const char* TO_CANCELL_ORDER = "XX";
const char* CANCEL_REGISTERED = "XV";
std::string GetTableName(std::string symbol);

bool OverWriteFile(std::string fileName, std::vector<std::string> data)
{
    std::ofstream file(fileName, std::ios::out | std::ios::trunc);
    if (!file.is_open())
        return false;
    for (int i = 0; i < data.size(); i++)
    {
        file << data.at(i) << std::endl;
    }
    return true;

}

std::vector<std::string> CopyFileContent(std::string fileName, char* buffer)
{
    std::ifstream file(fileName);
    std::string line;
    std::vector<std::string> fileContent;
    if (!file.is_open())
        return fileContent;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            fileContent.push_back(line);
        }
    }
    file.close();
    return fileContent;
}

bool FindAndUpdateLine(std::vector<std::string>* vec, std::string id, char* buffer, int ticketNum)
{
    std::vector<std::string>& A = *vec;

    for (int i = 0; i < A.size(); i++)
    {
        int index = A.at(i).find(id);
        if (index >= 0)
        {
            int vvIdx = A.at(i).find("VV");
            if (vvIdx < 0)
                A.at(i) += ",VV," + std::to_string(ticketNum);
            return true;
        }
    }
    return false;
}

std::string ReadLastOrder(const char* fileName)
{
    std::ifstream file(fileName);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << std::endl;
        return "";
    }

    std::string lastLine;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            int index = line.find("VV");
            if (index < 0)
            {
                lastLine = line;
                break;
            }
        }
    }

    file.close();
    return lastLine;
}

bool findOrder(std::string orderNum, const char* fileName)
{
    std::ifstream file(fileName);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << std::endl;
        return "";
    }
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            int index = line.find(orderNum);
            if (index >= 0)
            {
                return true;
            }
        }
    }

    file.close();
    return false;
}

bool insetOrder(std::string orderLine, const char* fileName)
{
    std::ofstream file(fileName, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << std::endl;
        return false;
    }

    file << orderLine << "\n";
    file.close();
    return true;
}

bool FindCanceledOrder(std::string orderNum)
{
    std::ifstream file(FILE_NAME);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << std::endl;
        return "";
    }
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            int index = line.find(orderNum);
            if (index >= 0)
            {
                int regIdx = line.find(CANCEL_REGISTERED);
                if (int(line.find(TO_CANCELL_ORDER)) >= 0 && (regIdx < 0))
                    return true;
                else
                    return false;
            }
        }
    }

    file.close();
    return false;
}
void RegisterCancel(int index)
{
    std::vector<std::string> fileContect;

    char ticketNum[] = "";
    fileContect = CopyFileContent(FILE_NAME, ticketNum);
    if (fileContect.size() <= index)
        return;
    fileContect[index] = fileContect[index] + "," + CANCEL_REGISTERED;
    OverWriteFile(FILE_NAME, fileContect);
}
std::string CanceledOrder()
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
            int index = line.find(TO_CANCELL_ORDER);
            if (index >= 0 && int(line.find(CANCEL_REGISTERED)) < 0)
            {
                lastLine = line;
                break;
            }
            lineNum++;
        }
    }
    file.close();
    RegisterCancel(lineNum);

    return lastLine;
}


bool UpdateCancelOrder(std::vector<std::string>* vec, std::string id)
{
    std::vector<std::string>& A = *vec;

    for (int i = 0; i < A.size(); i++)
    {
        int index = A.at(i).find(id);
        if (index >= 0)
        {
            int xxIdx = A.at(i).find("XX");
            if (xxIdx < 0)
                A.at(i) += ",XX";
            return true;
        }
    }
    return false;
}

SyncJobStructure InializeJobObject(char* timePoint, char* symbol, char* openPrice, char* closePrice, char* highPrice, char* lowPrice, char* rsi_M1, char* rsi_M5, char* rsi_M15, char* rsi_M30, char* rsi_H1, char* rsi_H4, char* ema_M1, char* ema_M5, char* ema_M15, char* ema_M30, char* ema_H1, char* ema_H4, char* zlsma_M1, char* zlsma_M5, char* zlsma_M15, char* zlsma_M30, char* zlsma_H1, char* zlsma_H4, char* gravity_M1)
{
    SyncJobStructure job;
    job.priceDateTime = timePoint;
    job.symbol = symbol;
    job.OpenPrice = openPrice;
    job.ClosePrice = closePrice;
    job.tableName = GetTableName(symbol);

  /*  job.HighPrice = highPrice;
    job.LowPrice = lowPrice;

    job.rsi.M1 = std::stod(std::string(rsi_M1));
    job.rsi.M5 = std::stod(std::string(rsi_M5));
    job.rsi.M15 = std::stod(std::string(rsi_M15));
    job.rsi.M30 = std::stod(std::string(rsi_M30));
    job.rsi.H1 = std::stod(std::string(rsi_H1));
    job.rsi.H4 = std::stod(std::string(rsi_H4));

    job.ema.M1 = std::stod(std::string(ema_M1));
    job.ema.M5 = std::stod(std::string(ema_M5));
    job.ema.M15 = std::stod(std::string(ema_M15));
    job.ema.M30 = std::stod(std::string(ema_M30));
    job.ema.H1 = std::stod(std::string(ema_H1));
    job.ema.H4 = std::stod(std::string(ema_H4));


    job.zlsma.M1 = std::stod(std::string(zlsma_M1));
    job.zlsma.M5 = std::stod(std::string(zlsma_M5));
    job.zlsma.M15 = std::stod(std::string(zlsma_M15));
    job.zlsma.M30 = std::stod(std::string(zlsma_M30));
    job.zlsma.H1 = std::stod(std::string(zlsma_H1));
    job.zlsma.H4 = std::stod(std::string(zlsma_H4));

    std::vector<std::string> gravityDetails = ProcessData::splitString(std::string(gravity_M1), ";");
    job.gravity.m_TotalTicks = std::stod(gravityDetails.at(0));
    job.gravity.m_UpperTicks = std::stod(gravityDetails.at(1));
    job.gravity.m_lowerTicks = std::stod(gravityDetails.at(2));*/
    return job;
}

std::string InitLineToWrite(SyncJobStructure job)
{
    std::ostringstream lineToWrite;
  /*  lineToWrite << job.priceDateTime << COMMA << job.symbol << COMMA << job.OpenPrice << COMMA << job.ClosePrice << COMMA;
    lineToWrite << job.HighPrice << COMMA << job.LowPrice << COMMA;
    lineToWrite << job.rsi.M1 << COMMA << job.rsi.M5 << COMMA << job.rsi.M15 << COMMA << job.rsi.M30 << COMMA << job.rsi.H1 << COMMA << job.rsi.H4 << COMMA;
    lineToWrite << job.ema.M1 << COMMA << job.ema.M5 << COMMA << job.ema.M15 << COMMA << job.ema.M30 << COMMA << job.ema.H1 << COMMA << job.ema.H4 << COMMA;
    lineToWrite << job.zlsma.M1 << COMMA << job.zlsma.M5 << COMMA << job.zlsma.M15 << COMMA << job.zlsma.M30 << COMMA << job.zlsma.H1 << COMMA << job.zlsma.H4 << COMMA;
    lineToWrite << job.gravity.m_TotalTicks << ";" << job.gravity.m_UpperTicks << ";" << job.gravity.m_lowerTicks;*/
    //lineToWrite << std::endl;
    return std::string(lineToWrite.str());
}


std::string GetTableName(std::string symbol)
{
    if (symbol == "EURUSD")
        return DEF_TABLE_EURUSD_CurrentPrices;
    if (symbol == "EURUSD_M1")
        return DEF_TABLE_EURUSD_OneMinTimeLinePrices;
    if (symbol == "EURUSD_TICK")
        return DEF_TABLE_EURUSD_TimeLinePrices;
    if (symbol == "XAUUSD")
        return DEF_TABLE_XAUUSD_CurrentPrices;
    if (symbol == "XAUUSD_M1")
        return DEF_TABLE_XAUUSD_OneMinTimeLinePrices;
    if (symbol == "XAUUSD_TICK")
        return DEF_TABLE_XAUUSD_TimeLinePrices;
    return "";
}