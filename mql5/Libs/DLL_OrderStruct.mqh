//+------------------------------------------------------------------+
//|                                              DLL_OrderStruct.mqh |
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
enum CoinsPair
{
	CP_XAUUSD = 0,
	CP_EURUSD = 1,
	CP_NONE = -1

};

enum OrderStatus
{
	OS_OPEN = 0,
	OS_CLOSED = 1,
	OS_NONE = -1
};

enum OrderType
{
	OT_LONG = 0,
	OT_SHORT = 1,
	OT_NONE = -1
};

struct DLL_OrderStruct
{
	DLL_OrderStruct()
	{
		type = OrderType::OT_NONE;
		status = OrderStatus::OS_NONE;
		pair = CoinsPair::CP_NONE;
		seqNum = -1;
		stopLoss = 0;
		takeProfit = 0;
	}

	OrderType type;
	OrderStatus status;
	CoinsPair pair;
	int seqNum;
	int stopLoss;
	int takeProfit;
};