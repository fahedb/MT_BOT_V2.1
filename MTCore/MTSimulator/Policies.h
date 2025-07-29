#include <string>
#include "ThreadPool.h"

#pragma once

struct InfraConfigrationsPolicy {
    std::string http_server_port;
};


struct PolynomPolicy
{
    double weight_eval = 0.5; // Weight for evaluation score
    double weight_rmse = 0.3; // Weight for RMSE score
    double weight_coeff = 0.2; // Weight for coefficient similarity score

    int PolynomLen_ForScan = 5; // Length of the polynomial to scan
};


struct ConfigrationsPolicy {

    double Pattern_upperGravity_Check = 0.0;
    double Pattern_lowerGravity_Check = 0.0;

    double Pattern_Gravity_Range_Check = 0.0;

    double Score_Min_TotalScore_ToAct_PerCandle = 0;
    int Score_Future_Vision_Candles = 0;


    double Patter_Min_Patterns_ToTake_Action = 0.0;
    double Patter_Max_Patterns_To_Find = 0.0;
    double Pattern_TopScore_ThresholdRatio = 0.0;

    double Pattern_Min_Percent_To_ExecuteOrder = 0.0;

    int Pattern_Activate_Alternative_ExecuteOrder = 0;
    double Pattern_Diff_Percent_To_ExecuteOrder_Alternative = 0.0;
    double Pattern_Max_Noise_To_ExecuteOrder_Alternative = 0.0;

    double Min_MeanPips_ToExecute_Order = 0.0;
    double Pattern_Min_PipsChange_ToCount = 0.0;
    int Pattern_CurrentTimeLineLen = 0;

    int SavedDirectionStaticsLen = 0;

    int Max_Orders_InRow = 0;
    int Sleep_Minuts_after_Max_Orders_InRow = 0;
    int EMA_len_for_Pattern_Export = 0;
    int RSI_Wieght = 0;
    int EMA_Wieght = 0;
    int ZLSMA_Wieght = 0;
    int VOLUME_Wieght = 0;
    int SL_start = 0;
    int TP_start = 0;

    int TP_diff = 0;
    int SL_diff = 0;

    int TimeBetweenScans = 0;

    bool Export_Patterns_PNG_To_Telegram = false;
    bool Enable_Patterns_PNG_Export = false;
    int maxScorePossibleForChangeNodes = 0;

    bool EnableTradeTracing = false;

    bool EnableDynamicTimeLine_TradeTracing = false;

    int TraceCurrTimeLineLen = 0;

    bool EnableScoreCaching = true;
    long ScoreResultsCacheSizePerThread = 100000; // Default size for each thread's cache
    int ThreadsCount = ThreadPool::getInstance()->Get_ThreadsNum(); // Default to number of hardware threads minus one
    int CleanEveryXScans = 10; // Default to clean cache every 10 scans

    bool EnablePolynomScan = 0;

};