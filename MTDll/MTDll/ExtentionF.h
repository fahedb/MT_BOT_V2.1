#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <codecvt>
#include <vector>
#include <sstream>
#include "DefinedTables.h"

std::string GetTableName(std::string symbol);
//
//bool WriteJobToFileExtended(std::string filePath, SyncJobStructure* priceN)
//{
//    
//    std::ofstream file(filePath, std::ios::app);
//    if (!file.is_open())
//        return false;
//    file
//        << priceN->priceDateTime
//        << "," << std::string(priceN->data.openPrice, 7)
//        << "," << std::string(priceN->data.closePrice, 7)
//        << "," << std::string(priceN->data.highPrice, 7)
//        << "," << std::string(priceN->data.lowPrice, 7)
//
//        << "," << std::string(priceN->data.ema_M1, 7)
//        << "," << std::string(priceN->data.ema_M5, 7)
//        << "," << std::string(priceN->data.ema_M15, 7)
//        << "," << std::string(priceN->data.ema_M30, 7)
//        << "," << std::string(priceN->data.ema_H1, 7)
//        << "," << std::string(priceN->data.ema_H4, 7)
//
//        << "," << std::string(priceN->data.ema_slope_M1, 7)
//        << "," << std::string(priceN->data.ema_slope_M5, 7)
//        << "," << std::string(priceN->data.ema_slope_M15, 7)
//        << "," << std::string(priceN->data.ema_slope_M30, 7)
//        << "," << std::string(priceN->data.ema_slope_H1, 7)
//        << "," << std::string(priceN->data.ema_slope_H4, 7)
//
//
//        << "," << std::string(priceN->data.rsi_M1, 7)
//        << "," << std::string(priceN->data.rsi_M5, 7)
//        << "," << std::string(priceN->data.rsi_M15, 7)
//        << "," << std::string(priceN->data.rsi_M30, 7)
//        << "," << std::string(priceN->data.rsi_H1, 7)
//        << "," << std::string(priceN->data.rsi_H4, 7)
//
//        << "," << std::string(priceN->data.rsi_slope_M1, 7)
//        << "," << std::string(priceN->data.rsi_slope_M5, 7)
//        << "," << std::string(priceN->data.rsi_slope_M15, 7)
//        << "," << std::string(priceN->data.rsi_slope_M30, 7)
//        << "," << std::string(priceN->data.rsi_slope_H1, 7)
//        << "," << std::string(priceN->data.rsi_slope_H4, 7)
//
//        << "," << ProcessData::TrimSpaces(std::string(priceN->data.zlsma_M1, 7))
//        << "," << ProcessData::TrimSpaces(std::string(priceN->data.zlsma_M5, 7))
//        << "," << ProcessData::TrimSpaces(std::string(priceN->data.zlsma_M15, 7))
//        << "," << ProcessData::TrimSpaces(std::string(priceN->data.zlsma_M30, 7))
//        << "," << ProcessData::TrimSpaces(std::string(priceN->data.zlsma_H1, 7))
//        << "," << ProcessData::TrimSpaces(std::string(priceN->data.zlsma_H4, 7))
//
//        << "," << ProcessData::TrimSpaces(std::string(priceN->data.zlsma_slope_M1, 7))
//        << "," << ProcessData::TrimSpaces(std::string(priceN->data.zlsma_slope_M5, 7))
//        << "," << ProcessData::TrimSpaces(std::string(priceN->data.zlsma_slope_M15, 7))
//        << "," << ProcessData::TrimSpaces(std::string(priceN->data.zlsma_slope_M30, 7))
//        << "," << ProcessData::TrimSpaces(std::string(priceN->data.zlsma_slope_H1, 7))
//        << "," << ProcessData::TrimSpaces(std::string(priceN->data.zlsma_slope_H4, 7))
//
//
//        << "," << std::to_string(priceN->data.totalTicks)
//        << "," << std::to_string(priceN->data.upperTicks)
//        << "," << std::to_string(priceN->data.lowerTicks) << "\n";
//    file.close();
//    return true;
//
//}

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