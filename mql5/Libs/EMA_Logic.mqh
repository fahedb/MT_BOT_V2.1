//+------------------------------------------------------------------+
//|                                                    EMA_Logic.mqh |
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

IndicatorHandlres emaHandles;
bool EmaHandlesInited = false;



void InitHandles_EMA()
{

emaHandles.handle_L10.handle_M1 = iMA("XAUUSD", PERIOD_M1, 10 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L10.handle_M5 = iMA("XAUUSD", PERIOD_M5, 10 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L10.handle_M15 = iMA("XAUUSD", PERIOD_M15, 10 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L10.handle_M30 = iMA("XAUUSD", PERIOD_M30, 10 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L10.handle_H1 = iMA("XAUUSD", PERIOD_H1, 10 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L10.handle_H4 = iMA("XAUUSD", PERIOD_H4, 10 ,0, MODE_EMA, PRICE_CLOSE);


emaHandles.handle_L15.handle_M1 = iMA("XAUUSD", PERIOD_M1, 15 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L15.handle_M5 = iMA("XAUUSD", PERIOD_M5, 15 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L15.handle_M15 = iMA("XAUUSD", PERIOD_M15, 15 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L15.handle_M30 = iMA("XAUUSD", PERIOD_M30, 15 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L15.handle_H1 = iMA("XAUUSD", PERIOD_H1, 15 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L15.handle_H4 = iMA("XAUUSD", PERIOD_H4, 15 ,0, MODE_EMA, PRICE_CLOSE);

emaHandles.handle_L20.handle_M1 = iMA("XAUUSD", PERIOD_M1, 20 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L20.handle_M5 = iMA("XAUUSD", PERIOD_M5, 20 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L20.handle_M15 = iMA("XAUUSD", PERIOD_M15, 20 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L20.handle_M30 = iMA("XAUUSD", PERIOD_M30, 20 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L20.handle_H1 = iMA("XAUUSD", PERIOD_H1, 20 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L20.handle_H4 = iMA("XAUUSD", PERIOD_H4, 20 ,0, MODE_EMA, PRICE_CLOSE);


emaHandles.handle_L25.handle_M1 = iMA("XAUUSD", PERIOD_M1, 25 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L25.handle_M5 = iMA("XAUUSD", PERIOD_M5, 25 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L25.handle_M15 = iMA("XAUUSD", PERIOD_M15, 25 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L25.handle_M30 = iMA("XAUUSD", PERIOD_M30, 25 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L25.handle_H1 = iMA("XAUUSD", PERIOD_H1, 25 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L25.handle_H4 = iMA("XAUUSD", PERIOD_H4, 25 ,0, MODE_EMA, PRICE_CLOSE);

emaHandles.handle_L30.handle_M1 = iMA("XAUUSD", PERIOD_M1, 30 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L30.handle_M5 = iMA("XAUUSD", PERIOD_M5, 30 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L30.handle_M15 = iMA("XAUUSD", PERIOD_M15, 30 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L30.handle_M30 = iMA("XAUUSD", PERIOD_M30, 30 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L30.handle_H1 = iMA("XAUUSD", PERIOD_H1, 30 ,0, MODE_EMA, PRICE_CLOSE);
emaHandles.handle_L30.handle_H4 = iMA("XAUUSD", PERIOD_H4, 30 ,0, MODE_EMA, PRICE_CLOSE);

}


void Reset_Ema_Init()
{
EmaHandlesInited =false;
}

TimeFrameIndicatorHandlres GetAllTimeFramesHandle_EMA(int len)
{
if(len == 10) return emaHandles.handle_L10;
if(len == 15) return emaHandles.handle_L15;
if(len == 20) return emaHandles.handle_L20;
if(len == 25) return emaHandles.handle_L25;
if(len == 30) return emaHandles.handle_L30;
return emaHandles.handle_L15;
}

int Get_EMAHandle(ENUM_TIMEFRAMES timeFrame,int len)
{
TimeFrameIndicatorHandlres timeFrameHandle = GetAllTimeFramesHandle_EMA(len);
if(timeFrame == PERIOD_M1)  return timeFrameHandle.handle_M1;
if(timeFrame == PERIOD_M5)  return timeFrameHandle.handle_M5;
if(timeFrame == PERIOD_M15) return timeFrameHandle.handle_M15;
if(timeFrame == PERIOD_M30) return timeFrameHandle.handle_M30;
if(timeFrame == PERIOD_H1)  return timeFrameHandle.handle_H1;
if(timeFrame == PERIOD_H4)  return timeFrameHandle.handle_H4;
return INVALID_HANDLE;
}




double Get_Ema(ENUM_TIMEFRAMES timeFrame,int indL,int shift=0)
  {
  if(!EmaHandlesInited)
  {
   InitHandles_EMA();
   EmaHandlesInited= true;  
  }

   double ema[2];
   int emaHandle = Get_EMAHandle(timeFrame,indL);
   
   int calcedBars = BarsCalculated(emaHandle);
   if(calcedBars < shift )
      {
         Print("Critical: Calced Bars are: " + calcedBars);
         return -1000;
      }
      
   int copied = CopyBuffer(emaHandle,0,shift,2,ema);
   if(copied <= 0)
   {
      InitHandles_EMA();
      emaHandle = Get_EMAHandle(timeFrame,indL);
      copied = CopyBuffer(emaHandle,0,shift,2,ema);
      if(copied <= 0)
         {
            Reset_Ema_Init();
            
            Print("Critical: CopyBuffer failed even after reinit. Returning -1");
            return -1000.0;
         }
   
   }
   return NormalizeDouble(ema[0],_Digits);
  }


double Get_Ema_Slope(ENUM_TIMEFRAMES timeFrame,int indL,int shift=0)
{
    double first = Get_Ema(timeFrame,indL,shift);
    double second = Get_Ema(timeFrame,indL,shift + 1);
    
      double slope = CalcSlope(first,second);
      return NormalizeDouble(slope,2);
}
