//+------------------------------------------------------------------+
//|                                             Indicators_Logic.mqh |
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
#include "EMA_Logic.mqh"
#include "RSI_Logic.mqh"
#include "ZLSMA_Logic.mqh"

enum IndicatorType
{
RSI = 0,
EMA = 1,
ZLSMA = 2
};


double Get_IndicatorValue(IndicatorType indT,int indL, ENUM_TIMEFRAMES barP,int shift)
{
   if(indT == EMA)
      return Get_Ema(barP,indL,shift);
   if(indT == RSI)
      return Get_Rsi(barP,indL,shift);
   if(indT == ZLSMA)
      return Get_Zlsma(barP,indL,shift);
    
    return 0.0;
}

double Get_IndicatorSlope(IndicatorType indT,int indL, ENUM_TIMEFRAMES barP,int shift)
{
   if(indT == EMA)
      return Get_Ema_Slope(barP,indL,shift);
   if(indT == RSI)
      return Get_Rsi_Slope(barP,indL,shift);
   if(indT == ZLSMA)
      return Get_Zlsma_Slope(barP,indL,shift);
   return 0.0;
}


IndicatorNode Get_Ind_Node(IndicatorType indT,int indL,int shift)
{
  IndicatorNode node;
  node.len=indL;
  StringToCharArray(DoubleToString(Get_IndicatorValue(indT,indL,PERIOD_M1,shift)),node.vals.M1);
  StringToCharArray(DoubleToString(Get_IndicatorValue(indT,indL,PERIOD_M5,shift)),node.vals.M5);
  StringToCharArray(DoubleToString(Get_IndicatorValue(indT,indL,PERIOD_M15,shift)),node.vals.M15);
  StringToCharArray(DoubleToString(Get_IndicatorValue(indT,indL,PERIOD_M30,shift)),node.vals.M30);
  StringToCharArray(DoubleToString(Get_IndicatorValue(indT,indL,PERIOD_H1,shift)),node.vals.H1);
  StringToCharArray(DoubleToString(Get_IndicatorValue(indT,indL,PERIOD_H4,shift)),node.vals.H4);
  
  StringToCharArray(DoubleToString(Get_IndicatorSlope(indT,indL,PERIOD_M1,shift)),node.vals.M1_Slope);
  StringToCharArray(DoubleToString(Get_IndicatorSlope(indT,indL,PERIOD_M5,shift)),node.vals.M5_Slope);
  StringToCharArray(DoubleToString(Get_IndicatorSlope(indT,indL,PERIOD_M15,shift)),node.vals.M15_Slope);
  StringToCharArray(DoubleToString(Get_IndicatorSlope(indT,indL,PERIOD_M30,shift)),node.vals.M30_Slope);
  StringToCharArray(DoubleToString(Get_IndicatorSlope(indT,indL,PERIOD_H1,shift)),node.vals.H1_Slope);
  StringToCharArray(DoubleToString(Get_IndicatorSlope(indT,indL,PERIOD_H4,shift)),node.vals.H4_Slope);
    
  return node;
}