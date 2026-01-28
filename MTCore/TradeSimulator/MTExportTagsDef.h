#pragma once


#define Tag_SimulationID "SimulationID" // int


/////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
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


#define Table_BackTest_Deals "BackTest_Deals"
// Columns:
/*
	SimulationID      INT            NOT NULL,
	symbol            NVARCHAR(32)    NOT NULL,
	ticket            BIGINT         NOT NULL,
	openTime          BIGINT         NOT NULL,
	closeTime         BIGINT         NOT NULL,
	lotSize           DECIMAL(18, 8)  NOT NULL,
	openPrice         DECIMAL(18, 8)  NOT NULL,
	closePrice        DECIMAL(18, 8)  NOT NULL,
	takeProfitPrice   DECIMAL(18, 8)  NOT NULL,
	stopLossPrice     DECIMAL(18, 8)  NOT NULL,
	dealType          NVARCHAR(16)    NOT NULL,
	profit            DECIMAL(18, 8)  NOT NULL,
	profitInPips      INT            NOT NULL,
	closeReason       NVARCHAR(32)    NOT NULL,
	numOfBarsHeld     INT              NOT NULL,
*/
#define Deals_Column_SimulationID "SimulationID"
#define Deals_Column_Symbol "symbol"
#define Deals_Column_Ticket "ticket"
#define Deals_Column_OpenTime "openTime"
#define Deals_Column_CloseTime "closeTime"
#define Deals_Column_LotSize "lotSize"
#define Deals_Column_OpenPrice "openPrice"
#define Deals_Column_ClosePrice "closePrice"
#define Deals_Column_TakeProfitPrice "takeProfitPrice"
#define Deals_Column_StopLossPrice "stopLossPrice"
#define Deals_Column_Type "dealType"
#define Deals_Column_Profit "profit"
#define Deals_Column_ProfitInPips "profitInPips"
#define Deals_Column_CloseReason "closeReason"
#define Deals_Column_NumOfBarsHeld "numOfBarsHeld"

// Combinations
#define Deals_Column_Json_Combo_SimulationID Deals_Column_SimulationID ,		{ Tag_SimulationID }
#define Deals_Column_Json_Combo_Symbol Deals_Column_Symbol ,					{ DealTag_Symbol   }
#define Deals_Column_Json_Combo_Ticket Deals_Column_Ticket ,					{ DealTag_Ticket   }
#define Deals_Column_Json_Combo_OpenTime Deals_Column_OpenTime ,				{ DealTag_OpenTime }
#define Deals_Column_Json_Combo_CloseTime Deals_Column_CloseTime ,				{ DealTag_CloseTime }
#define Deals_Column_Json_Combo_LotSize Deals_Column_LotSize ,					{ DealTag_LotSize }
#define Deals_Column_Json_Combo_OpenPrice Deals_Column_OpenPrice ,				{ DealTag_OpenPrice }
#define Deals_Column_Json_Combo_ClosePrice Deals_Column_ClosePrice ,			{ DealTag_ClosePrice }
#define Deals_Column_Json_Combo_TakeProfitPrice Deals_Column_TakeProfitPrice ,	{ DealTag_TakeProfitPrice }
#define Deals_Column_Json_Combo_StopLossPrice Deals_Column_StopLossPrice ,		{ DealTag_StopLossPrice }
#define Deals_Column_Json_Combo_Type Deals_Column_Type ,						{ DealTag_Type }
#define Deals_Column_Json_Combo_Profit Deals_Column_Profit ,					{ DealTag_Profit }
#define Deals_Column_Json_Combo_ProfitInPips Deals_Column_ProfitInPips ,		{ DealTag_ProfitInPips }
#define Deals_Column_Json_Combo_CloseReason Deals_Column_CloseReason ,			{ DealTag_CloseReason }
#define Deals_Column_Json_Combo_NumOfBarsHeld Deals_Column_NumOfBarsHeld ,		{ DealTag_NumOfBarsHeld }
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define AccountStateTag_AccountBalance "AccountBalance" // double
#define AccountStateTag_CurrTimePoint "CurrTimePoint" //long long
#define AccountStateTag_EquityBalance "EquityBalance" // double
#define AccountStateTag_UsedMargin "UsedMargin" // double
#define AccountStateTag_FreeMargin "FreeMargin" // double
#define AccountStateTag_FloatingPnL "FloatingPnL" // double
#define AccountStateTag_CurrProfit "CurrProfit" // double

#define Table_BackTest_AccountStateEvents "BackTest_AccountStateEvents"
// Columns:
/*
	SimulationID     INT            NOT NULL,
	CurrTimePoint    BIGINT         NOT NULL, -- YYYYMMDDHHMMSSMMM
	AccountBalance   DECIMAL(18, 8) NOT NULL,
	EquityBalance    DECIMAL(18, 8) NOT NULL,
	UsedMargin       DECIMAL(18, 8) NOT NULL,
	FreeMargin       DECIMAL(18, 8) NOT NULL,
	FloatingPnL      DECIMAL(18, 8) NOT NULL,
	CurrProfit       DECIMAL(18, 8) NOT NULL,

*/
#define AccountState_Column_SimulationID "SimulationID"
#define AccountState_Column_CurrTimePoint "CurrTimePoint"
#define AccountState_Column_AccountBalance "AccountBalance"
#define AccountState_Column_EquityBalance "EquityBalance"
#define AccountState_Column_UsedMargin "UsedMargin"
#define AccountState_Column_FreeMargin "FreeMargin"
#define AccountState_Column_FloatingPnL "FloatingPnL"
#define AccountState_Column_CurrProfit "CurrProfit"

// Combinations
#define AccountState_Column_Json_Combo AccountState_Column_SimulationID , { Tag_SimulationID } 
#define AccountState_Column_Json_Combo_CurrTimePoint AccountState_Column_CurrTimePoint , { AccountStateTag_CurrTimePoint }
#define AccountState_Column_Json_Combo_AccountBalance AccountState_Column_AccountBalance , { AccountStateTag_AccountBalance }
#define AccountState_Column_Json_Combo_EquityBalance AccountState_Column_EquityBalance , { AccountStateTag_EquityBalance }
#define AccountState_Column_Json_Combo_UsedMargin AccountState_Column_UsedMargin , { AccountStateTag_UsedMargin }
#define AccountState_Column_Json_Combo_FreeMargin AccountState_Column_FreeMargin , { AccountStateTag_FreeMargin }
#define AccountState_Column_Json_Combo_FloatingPnL AccountState_Column_FloatingPnL , { AccountStateTag_FloatingPnL }
#define AccountState_Column_Json_Combo_CurrProfit AccountState_Column_CurrProfit , { AccountStateTag_CurrProfit }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define DirectionStatsTag_TimePoint "TimePoint" // string
#define DirectionStatsTag_Pair "Pair" // string
#define DirectionStatsTag_TotalVotes "TotalVotes" // int
#define DirectionStatsTag_General_VotedPercentages "General_VotedPercentages" // object
#define DirectionStatsTag_First_VotedPercentages "First_VotedPercentages" // object
#define DirectionStatsTag_Highest_VotedPercentages "Highest_VotedPercentages" // object
/*/////////*/ #define DirectionStatsTag_VeotedPercentages_TotalVotes "TotalVotes" // double
/*/////////*/ #define DirectionStatsTag_VeotedPercentages_UP_Percent "UP_Percent" // double
/*/////////*/ #define DirectionStatsTag_VeotedPercentages_DOWN_Percent "DOWN_Percent" // double
/*/////////*/ #define DirectionStatsTag_VeotedPercentages_BLUR_Percent "BLUR_Percent" // double
/*/////////*/ #define DirectionStatsTag_VeotedPercentages_Mean "Mean" // double
/*/////////*/ #define DirectionStatsTag_VeotedPercentages_PureMean_UP "PureMean_UP" // double
/*/////////*/ #define DirectionStatsTag_VeotedPercentages_PureMean_DOWN "PureMean_DOWN" // double
/*/////////*/ #define DirectionStatsTag_VeotedPercentages_StdDev "StdDev" // double
/*/////////*/ #define DirectionStatsTag_VeotedPercentages_MeanIndexes "MeanIndexes" // double
#define DirectionStatsTag_SimilarPatterns "SimilarPatterns" // array of objects
/*/////////*/ #define DirectionStatsTag_SimilarPattern_PatternTimePoint "PatternTimePoint" // long long
#define DirectionStatsTag_TradeResult "TradeResult" // bool


#define Table_BackTest_DirectionStats "BackTest_DirectionStats"
// Columns:
/*
SimulationID int not null,
TimePoint BIGINT NOT NULL,
CoinsPair NVARCHAR(7) NOT NULL,
VoteType NVARCHAR(10) not null,
TotalVotes int not null,
UpPercentage DECIMAL(18, 8) NOT NULL,
DownPercentage DECIMAL(18, 8) NOT NULL,
BlurPercentage DECIMAL(18, 8) NOT NULL,
Mean DECIMAL(18, 8) NOT NULL,
PureMean_Up DECIMAL(18, 8) NOT NULL,
PureMean_Down DECIMAL(18, 8) NOT NULL,
StdDev DECIMAL(18, 8) NOT NULL,
MeanIndexes DECIMAL(18, 8) NOT NULL,
TradeResult int not null
*/

#define DirectionStats_Column_SimulationID "SimulationID"
#define DirectionStats_Column_TimePoint "TimePoint"
#define DirectionStats_Column_CoinsPair "CoinsPair"
#define DirectionStats_Column_VoteType "VoteType" // General / First / Highest
#define DirectionStats_Column_TotalVotes "TotalVotes"
#define DirectionStats_Column_UpPercentage "UpPercentage"
#define DirectionStats_Column_DownPercentage "DownPercentage"
#define DirectionStats_Column_BlurPercentage "BlurPercentage"
#define DirectionStats_Column_Mean "Mean"
#define DirectionStats_Column_PureMean_Up "PureMean_Up"
#define DirectionStats_Column_PureMean_Down "PureMean_Down"
#define DirectionStats_Column_StdDev "StdDev"
#define DirectionStats_Column_MeanIndexes "MeanIndexes"
#define DirectionStats_Column_TradeResult "TradeResult"

// Combinations
#define DirectionStats_Column_Json_Combo_SimulationID DirectionStats_Column_SimulationID , { Tag_SimulationID }
#define DirectionStats_Column_Json_Combo_TimePoint DirectionStats_Column_TimePoint , { DirectionStatsTag_TimePoint }
#define DirectionStats_Column_Json_Combo_CoinsPair DirectionStats_Column_CoinsPair , { DirectionStatsTag_Pair }
#define DirectionStats_Column_Json_Combo_TotalVotes DirectionStats_Column_TotalVotes , { DirectionStatsTag_TotalVotes }

#define DirectionStats_Column_Json_Combo_General_VotedPercentages_TotalVotes DirectionStats_Column_TotalVotes , { DirectionStatsTag_General_VotedPercentages, DirectionStatsTag_VeotedPercentages_TotalVotes }
#define DirectionStats_Column_Json_Combo_General_VotedPercentages_UP_Percent DirectionStats_Column_UpPercentage , { DirectionStatsTag_General_VotedPercentages, DirectionStatsTag_VeotedPercentages_UP_Percent }
#define DirectionStats_Column_Json_Combo_General_VotedPercentages_DOWN_Percent DirectionStats_Column_DownPercentage , { DirectionStatsTag_General_VotedPercentages, DirectionStatsTag_VeotedPercentages_DOWN_Percent }
#define DirectionStats_Column_Json_Combo_General_VotedPercentages_BLUR_Percent DirectionStats_Column_BlurPercentage , { DirectionStatsTag_General_VotedPercentages, DirectionStatsTag_VeotedPercentages_BLUR_Percent }
#define DirectionStats_Column_Json_Combo_General_VotedPercentages_Mean DirectionStats_Column_Mean , { DirectionStatsTag_General_VotedPercentages, DirectionStatsTag_VeotedPercentages_Mean }
#define DirectionStats_Column_Json_Combo_General_VotedPercentages_PureMean_UP DirectionStats_Column_PureMean_Up , { DirectionStatsTag_General_VotedPercentages, DirectionStatsTag_VeotedPercentages_PureMean_UP }
#define DirectionStats_Column_Json_Combo_General_VotedPercentages_PureMean_DOWN DirectionStats_Column_PureMean_Down , { DirectionStatsTag_General_VotedPercentages, DirectionStatsTag_VeotedPercentages_PureMean_DOWN }
#define DirectionStats_Column_Json_Combo_General_VotedPercentages_StdDev DirectionStats_Column_StdDev , { DirectionStatsTag_General_VotedPercentages, DirectionStatsTag_VeotedPercentages_StdDev }
#define DirectionStats_Column_Json_Combo_General_VotedPercentages_MeanIndexes DirectionStats_Column_MeanIndexes , { DirectionStatsTag_General_VotedPercentages, DirectionStatsTag_VeotedPercentages_MeanIndexes }


#define DirectionStats_Column_Json_Combo_First_VotedPercentages_TotalVotes DirectionStats_Column_TotalVotes , { DirectionStatsTag_First_VotedPercentages, DirectionStatsTag_VeotedPercentages_TotalVotes }
#define DirectionStats_Column_Json_Combo_First_VotedPercentages_UP_Percent DirectionStats_Column_UpPercentage , { DirectionStatsTag_First_VotedPercentages, DirectionStatsTag_VeotedPercentages_UP_Percent }
#define DirectionStats_Column_Json_Combo_First_VotedPercentages_DOWN_Percent DirectionStats_Column_DownPercentage, { DirectionStatsTag_First_VotedPercentages, DirectionStatsTag_VeotedPercentages_DOWN_Percent }
#define DirectionStats_Column_Json_Combo_First_VotedPercentages_BLUR_Percent DirectionStats_Column_BlurPercentage , { DirectionStatsTag_First_VotedPercentages, DirectionStatsTag_VeotedPercentages_BLUR_Percent }
#define DirectionStats_Column_Json_Combo_First_VotedPercentages_Mean DirectionStats_Column_Mean , { DirectionStatsTag_First_VotedPercentages, DirectionStatsTag_VeotedPercentages_Mean }
#define DirectionStats_Column_Json_Combo_First_VotedPercentages_PureMean_UP DirectionStats_Column_PureMean_Up , { DirectionStatsTag_First_VotedPercentages, DirectionStatsTag_VeotedPercentages_PureMean_UP }
#define DirectionStats_Column_Json_Combo_First_VotedPercentages_PureMean_DOWN DirectionStats_Column_PureMean_Down , { DirectionStatsTag_First_VotedPercentages, DirectionStatsTag_VeotedPercentages_PureMean_DOWN }
#define DirectionStats_Column_Json_Combo_First_VotedPercentages_StdDev DirectionStats_Column_StdDev , { DirectionStatsTag_First_VotedPercentages, DirectionStatsTag_VeotedPercentages_StdDev }
#define DirectionStats_Column_Json_Combo_First_VotedPercentages_MeanIndexes DirectionStats_Column_MeanIndexes , { DirectionStatsTag_First_VotedPercentages, DirectionStatsTag_VeotedPercentages_MeanIndexes }

#define DirectionStats_Column_Json_Combo_Highest_VotedPercentages_TotalVotes DirectionStats_Column_TotalVotes , { DirectionStatsTag_Highest_VotedPercentages, DirectionStatsTag_VeotedPercentages_TotalVotes }
#define DirectionStats_Column_Json_Combo_Highest_VotedPercentages_UP_Percent DirectionStats_Column_UpPercentage , { DirectionStatsTag_Highest_VotedPercentages, DirectionStatsTag_VeotedPercentages_UP_Percent }
#define DirectionStats_Column_Json_Combo_Highest_VotedPercentages_DOWN_Percent DirectionStats_Column_DownPercentage , { DirectionStatsTag_Highest_VotedPercentages, DirectionStatsTag_VeotedPercentages_DOWN_Percent }
#define DirectionStats_Column_Json_Combo_Highest_VotedPercentages_BLUR_Percent DirectionStats_Column_BlurPercentage , { DirectionStatsTag_Highest_VotedPercentages, DirectionStatsTag_VeotedPercentages_BLUR_Percent }
#define DirectionStats_Column_Json_Combo_Highest_VotedPercentages_Mean DirectionStats_Column_Mean , { DirectionStatsTag_Highest_VotedPercentages, DirectionStatsTag_VeotedPercentages_Mean }
#define DirectionStats_Column_Json_Combo_Highest_VotedPercentages_PureMean_UP DirectionStats_Column_PureMean_Up , { DirectionStatsTag_Highest_VotedPercentages, DirectionStatsTag_VeotedPercentages_PureMean_UP }
#define DirectionStats_Column_Json_Combo_Highest_VotedPercentages_PureMean_DOWN DirectionStats_Column_PureMean_Down , { DirectionStatsTag_Highest_VotedPercentages, DirectionStatsTag_VeotedPercentages_PureMean_DOWN }
#define DirectionStats_Column_Json_Combo_Highest_VotedPercentages_StdDev DirectionStats_Column_StdDev , { DirectionStatsTag_Highest_VotedPercentages, DirectionStatsTag_VeotedPercentages_StdDev }
#define DirectionStats_Column_Json_Combo_Highest_VotedPercentages_MeanIndexes DirectionStats_Column_MeanIndexes , { DirectionStatsTag_Highest_VotedPercentages, DirectionStatsTag_VeotedPercentages_MeanIndexes }

#define DirectionStats_Column_Json_Combo_TradeResult DirectionStats_Column_TradeResult , { DirectionStatsTag_TradeResult }