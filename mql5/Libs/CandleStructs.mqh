//+------------------------------------------------------------------+
//|                                                CandleStructs.mqh |
//|                                  Copyright 2025, MetaQuotes Ltd. |
//|                                             https://www.mql5.com |
//+------------------------------------------------------------------+
#property copyright "Copyright 2025, MetaQuotes Ltd."
#property link      "https://www.mql5.com"
//+------------------------------------------------------------------+
//| defines                                                          |
//+------------------------------------------------------------------+
// #define MacrosHello   "Hello, world!"
// #define MacrosYear    2010
//+------------------------------------------------------------------+
//| DLL imports                                                      |
//+------------------------------------------------------------------+
// #import "user32.dll"
//   int      SendMessageA(int hWnd,int Msg,int wParam,int lParam);
// #import "my_expert.dll"
//   int      ExpertRecalculate(int wParam,int lParam);
// #import
//+------------------------------------------------------------------+
//| EX5 imports                                                      |
//+------------------------------------------------------------------+
// #import "stdlib.ex5"
//   string ErrorDescription(int error_code);
// #import
//+------------------------------------------------------------------+


struct IndicatorValues{

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
};


struct IndicatorNode
{
   int len;
   IndicatorValues vals;
};


struct CandleData {
	char timePoint[20];
	char openPrice[8];
	char closePrice[8];
	char highPrice[8];
	char lowPrice[8];

	/// EMA values and slopes
	IndicatorNode Ema_nodes[5];
	/// ZLSMA values and slopes
	IndicatorNode Zlsma_nodes[5];
	/// RSI values and slopes
	IndicatorNode Rsi_nodes[5];
	/// Gravity values
	int totalTicks;
	int upperTicks;
	int lowerTicks;
};
