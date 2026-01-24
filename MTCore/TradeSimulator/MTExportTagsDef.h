#pragma once

#define Tag_SimulationID "SimulationID" // int

#define DealTag_Symbol "symbol" // string
#define DealTag_Ticket "ticket" // long
#define DealTag_OpenTime "openTime" // long long (YYYYMMDDHHMMSSMMM)
#define DealTag_CloseTime "closeTime" // long long (YYYYMMDDHHMMSSMMM) 
#define DealTag_LotSize "lotSize" // double
#define DealTag_OpenPrice "openPrice" // double
#define DealTag_ClosePrice "closePrice" // double
#define DealTag_StopLossPrice "StopLossPrice" // double
#define DealTag_TakeProfitPrice "TakeProfitPrice" // double
#define DealTag_Type "type" // string
#define DealTag_Profit "profit" // double
#define DealTag_ProfitInPips "profitInPips" // int
#define DealTag_CloseReason "closeReason" // string
#define DealTag_NumOfBarsHeld "numOfBarsHeld" // int

#define AccountStateTag_AccountBalance "AccountBalance" // double
#define AccountStateTag_CurrTimePoint "CurrTimePoint" //long long
#define AccountStateTag_EquityBalance "EquityBalance" // double
#define AccountStateTag_UsedMargin "UsedMargin" // double
#define AccountStateTag_FreeMargin "FreeMargin" // double
#define AccountStateTag_FloatingPnL "FloatingPnL" // double
#define AccountStateTag_CurrProfit "CurrProfit" // double



#define DirectionStatsTag_TimePoint "TimePoint" // string
#define DirectionStatsTag_Pair "Pair" // string
#define DirectionStatsTag_TotalVotes "TotalVotes" // int
#define DirectionStatsTag_General_VotedPercentages "General_VotedPercentages" // object
#define DirectionStatsTag_First_VotedPercentages "First_VotedPercentages" // object
#define DirectionStatsTag_Highest_VotedPercentages "Highest_VotedPercentages" // object
/*/////////*/ #define DirectionStatsTag_VeotedPercentages_TotalVotes "TotalVotes" // double
/*/////////*/ #define DirectionStatsTag_VeotedPercentages_Mean "Mean" // double
/*/////////*/ #define DirectionStatsTag_VeotedPercentages_PureMean_UP "PureMean_UP" // double
/*/////////*/ #define DirectionStatsTag_VeotedPercentages_PureMean_DOWN "PureMean_DOWN" // double
/*/////////*/ #define DirectionStatsTag_VeotedPercentages_StdDev "StdDev" // double
/*/////////*/ #define DirectionStatsTag_VeotedPercentages_MeanIndexes "MeanIndexes" // double
#define DirectionStatsTag_SimilarPatterns "SimilarPatterns" // array of objects
/*/////////*/ #define DirectionStatsTag_SimilarPattern_PatternTimePoint "PatternTimePoint" // long long
#define DirectionStatsTag_TradeResult "TradeResult" // bool
