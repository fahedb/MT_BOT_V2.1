#pragma once

#define INDICATOR_LENS_COUNT 5

typedef struct IndicatorValues_Dll {

	char M1[8];
	char M5[8];
	char M15[8];
	char M30[8];
	char H1[8];
	char H4[8];

	char M1_Slope[8];
	char M5_Slope[8];
	char M15_Slope[8];
	char M30_Slope[8];
	char H1_Slope[8];
	char H4_Slope[8];
}IndicatorValues_Dll,* IndicatorValues_Dll_P;


typedef struct IndicatorNode
{
	int len=0;
	IndicatorValues_Dll vals;
}IndicatorNode, * IndicatorNode_P;


typedef struct CandleData  // New struct for the new method, TODO: Need to Implement a logic in Sasson for this
{
	char timePoint[20];
	char openPrice[8];
	char closePrice[8];
	char highPrice[8];
	char lowPrice[8];

	/// EMA values and slopes
	IndicatorNode Ema_nodes[INDICATOR_LENS_COUNT];
	/// ZLSMA values and slopes
	IndicatorNode Zlsma_nodes[INDICATOR_LENS_COUNT];
	/// RSI values and slopes
	IndicatorNode Rsi_nodes[INDICATOR_LENS_COUNT];
	/// Gravity values

	int totalTicks=0;
	int upperTicks=0;
	int lowerTicks=0;
}CandleData, * CandleStruct;


