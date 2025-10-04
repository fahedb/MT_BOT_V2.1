//+------------------------------------------------------------------+
//|                                                HandlesStruct.mqh |
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

struct TimeFrameIndicatorHandlres
{
int handle_M1;
int handle_M5;
int handle_M15;
int handle_M30;
int handle_H1;
int handle_H4;
};


struct IndicatorHandlres
{
TimeFrameIndicatorHandlres handle_L10;
TimeFrameIndicatorHandlres handle_L15;
TimeFrameIndicatorHandlres handle_L20;
TimeFrameIndicatorHandlres handle_L25;
TimeFrameIndicatorHandlres handle_L30;
};
