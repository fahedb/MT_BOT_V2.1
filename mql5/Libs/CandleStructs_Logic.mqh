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

#include "CandleStructs.mqh"
#include "Indicators_Logic.mqh"
CandleData Get_CandleData(int shift)
{
   CandleData data;
   StringToCharArray(GetPriceInStr(0),data.openPrice,0,7);
   StringToCharArray(GetPriceInStr(1),data.closePrice,0,7);
   StringToCharArray(GetPriceInStr(2),data.highPrice,0,7);
   StringToCharArray(GetPriceInStr(3),data.lowPrice,0,7);
   
   data.Ema_nodes[0] = Get_Ind_Node(EMA,10,shift);
   data.Ema_nodes[1] = Get_Ind_Node(EMA,15,shift);
   data.Ema_nodes[2] = Get_Ind_Node(EMA,20,shift);
   data.Ema_nodes[3] = Get_Ind_Node(EMA,25,shift);
   data.Ema_nodes[4] = Get_Ind_Node(EMA,30,shift);
   
   data.Rsi_nodes[0] = Get_Ind_Node(RSI,10,shift);
   data.Rsi_nodes[1] = Get_Ind_Node(RSI,15,shift);
   data.Rsi_nodes[2] = Get_Ind_Node(RSI,20,shift);
   data.Rsi_nodes[3] = Get_Ind_Node(RSI,25,shift);
   data.Rsi_nodes[4] = Get_Ind_Node(RSI,30,shift);
   
   data.Zlsma_nodes[0] = Get_Ind_Node(ZLSMA,10,shift);
   data.Zlsma_nodes[1] = Get_Ind_Node(ZLSMA,15,shift);
   data.Zlsma_nodes[2] = Get_Ind_Node(ZLSMA,20,shift);
   data.Zlsma_nodes[3] = Get_Ind_Node(ZLSMA,25,shift);
   data.Zlsma_nodes[4] = Get_Ind_Node(ZLSMA,30,shift);
   return data;
}