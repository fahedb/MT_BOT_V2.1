//+------------------------------------------------------------------+
//|                                                  ZLSMA_Logic.mqh |
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
#include "Mini_Calcs_Logic.mqh"
#include "HandlesStruct.mqh"

IndicatorHandlres zlsmaHandles;
bool zlsmaHandlesInited = false;



void InitHandles_ZLSMA()
{

zlsmaHandles.handle_L10.handle_M1 = iMA("XAUUSD", PERIOD_M1, 10 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L10.handle_M5 = iMA("XAUUSD", PERIOD_M5, 10 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L10.handle_M15 = iMA("XAUUSD", PERIOD_M15, 10 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L10.handle_M30 = iMA("XAUUSD", PERIOD_M30, 10 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L10.handle_H1 = iMA("XAUUSD", PERIOD_H1, 10 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L10.handle_H4 = iMA("XAUUSD", PERIOD_H4, 10 ,0, MODE_SMA, PRICE_CLOSE);


zlsmaHandles.handle_L15.handle_M1 = iMA("XAUUSD", PERIOD_M1, 15 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L15.handle_M5 = iMA("XAUUSD", PERIOD_M5, 15 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L15.handle_M15 = iMA("XAUUSD", PERIOD_M15, 15 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L15.handle_M30 = iMA("XAUUSD", PERIOD_M30, 15 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L15.handle_H1 = iMA("XAUUSD", PERIOD_H1, 15 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L15.handle_H4 = iMA("XAUUSD", PERIOD_H4, 15 ,0, MODE_SMA, PRICE_CLOSE);

zlsmaHandles.handle_L20.handle_M1 = iMA("XAUUSD", PERIOD_M1, 20 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L20.handle_M5 = iMA("XAUUSD", PERIOD_M5, 20 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L20.handle_M15 = iMA("XAUUSD", PERIOD_M15, 20 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L20.handle_M30 = iMA("XAUUSD", PERIOD_M30, 20 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L20.handle_H1 = iMA("XAUUSD", PERIOD_H1, 20 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L20.handle_H4 = iMA("XAUUSD", PERIOD_H4, 20 ,0, MODE_SMA, PRICE_CLOSE);


zlsmaHandles.handle_L25.handle_M1 = iMA("XAUUSD", PERIOD_M1, 25 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L25.handle_M5 = iMA("XAUUSD", PERIOD_M5, 25 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L25.handle_M15 = iMA("XAUUSD", PERIOD_M15, 25 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L25.handle_M30 = iMA("XAUUSD", PERIOD_M30, 25 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L25.handle_H1 = iMA("XAUUSD", PERIOD_H1, 25 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L25.handle_H4 = iMA("XAUUSD", PERIOD_H4, 25 ,0, MODE_SMA, PRICE_CLOSE);

zlsmaHandles.handle_L30.handle_M1 = iMA("XAUUSD", PERIOD_M1, 30 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L30.handle_M5 = iMA("XAUUSD", PERIOD_M5, 30 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L30.handle_M15 = iMA("XAUUSD", PERIOD_M15, 30 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L30.handle_M30 = iMA("XAUUSD", PERIOD_M30, 30 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L30.handle_H1 = iMA("XAUUSD", PERIOD_H1, 30 ,0, MODE_SMA, PRICE_CLOSE);
zlsmaHandles.handle_L30.handle_H4 = iMA("XAUUSD", PERIOD_H4, 30 ,0, MODE_SMA, PRICE_CLOSE);

}


void Reset_zlsma_Init()
{
zlsmaHandlesInited =false;
}

TimeFrameIndicatorHandlres GetAllTimeFramesHandle_ZLSMA(int len)
{
if(len == 10) return zlsmaHandles.handle_L10;
if(len == 15) return zlsmaHandles.handle_L15;
if(len == 20) return zlsmaHandles.handle_L20;
if(len == 25) return zlsmaHandles.handle_L25;
if(len == 30) return zlsmaHandles.handle_L30;
return zlsmaHandles.handle_L15;
}

int Get_ZLSMAHandle(ENUM_TIMEFRAMES timeFrame,int len)
{
TimeFrameIndicatorHandlres timeFrameHandle = GetAllTimeFramesHandle_ZLSMA(len);
if(timeFrame == PERIOD_M1)  return timeFrameHandle.handle_M1;
if(timeFrame == PERIOD_M5)  return timeFrameHandle.handle_M5;
if(timeFrame == PERIOD_M15) return timeFrameHandle.handle_M15;
if(timeFrame == PERIOD_M30) return timeFrameHandle.handle_M30;
if(timeFrame == PERIOD_H1)  return timeFrameHandle.handle_H1;
if(timeFrame == PERIOD_H4)  return timeFrameHandle.handle_H4;
return INVALID_HANDLE;
}


double Get_Zlsma(ENUM_TIMEFRAMES timeFrame,int indL,int shift=0)
  {
  
// Calculate the Simple Moving Average (SMA)
   
   double smaCurrentArr[2];
   int smaHandle = Get_ZLSMAHandle(timeFrame,indL);
   int copied = CopyBuffer(smaHandle,0,shift,2,smaCurrentArr);
   
   if(copied <= 0)
   {
      InitHandles_ZLSMA();
      copied = CopyBuffer(smaHandle,0,shift,2,smaCurrentArr);
      if(copied <= 0)
         {
            Reset_zlsma_Init();
            return -1000;
         }
   }
   
   double smaCurrent = smaCurrentArr[0];
   
// Calculate the previous Simple Moving Average (SMA)
   double smaPreviousArr[2];
   copied = CopyBuffer(smaHandle,0,shift + 1,2,smaPreviousArr);
   if(copied <= 0)
   {
      InitHandles_ZLSMA();
      copied = CopyBuffer(smaHandle,0,shift + 1,2,smaPreviousArr);
      if(copied <= 0)
         {
            Reset_zlsma_Init();
            return -1000;
         }
   }
   
   double smaPrevious = smaPreviousArr[0];
   
// Calculate the Zero-Lag Smooth Moving Average
   double zlsma = (smaCurrent * 2) - smaPrevious;
   return NormalizeDouble(zlsma,_Digits);
  }

double Get_Zlsma_Slope(ENUM_TIMEFRAMES timeFrame,int indL,int shift=0)
{
    double first = Get_Zlsma(timeFrame,indL,shift);
    double second = Get_Zlsma(timeFrame,indL,shift + 1);
    
    double slope = CalcSlope(first,second);
    return slope;
}

