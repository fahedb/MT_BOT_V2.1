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

IndicatorHandlres rsiHandles;
bool RsiHandlesInited = false;

void InitHandles_RSI()
{
rsiHandles.handle_L10.handle_M1 = iRSI("XAUUSD",PERIOD_M1,10,PRICE_CLOSE);
rsiHandles.handle_L10.handle_M5 = iRSI("XAUUSD",PERIOD_M5,10,PRICE_CLOSE);
rsiHandles.handle_L10.handle_M15 = iRSI("XAUUSD",PERIOD_M15,10,PRICE_CLOSE);
rsiHandles.handle_L10.handle_M30 = iRSI("XAUUSD",PERIOD_M30,10,PRICE_CLOSE);
rsiHandles.handle_L10.handle_H1 = iRSI("XAUUSD",PERIOD_H1,10,PRICE_CLOSE);
rsiHandles.handle_L10.handle_H4 = iRSI("XAUUSD",PERIOD_H4,10,PRICE_CLOSE);


rsiHandles.handle_L15.handle_M1 = iRSI("XAUUSD",PERIOD_M1,15,PRICE_CLOSE);
rsiHandles.handle_L15.handle_M5 = iRSI("XAUUSD",PERIOD_M5,15,PRICE_CLOSE);
rsiHandles.handle_L15.handle_M15 = iRSI("XAUUSD",PERIOD_M15,15,PRICE_CLOSE);
rsiHandles.handle_L15.handle_M30 = iRSI("XAUUSD",PERIOD_M30,15,PRICE_CLOSE);
rsiHandles.handle_L15.handle_H1 = iRSI("XAUUSD",PERIOD_H1,15,PRICE_CLOSE);
rsiHandles.handle_L15.handle_H4 = iRSI("XAUUSD",PERIOD_H4,15,PRICE_CLOSE);


rsiHandles.handle_L20.handle_M1 = iRSI("XAUUSD",PERIOD_M1,20,PRICE_CLOSE);
rsiHandles.handle_L20.handle_M5 = iRSI("XAUUSD",PERIOD_M5,20,PRICE_CLOSE);
rsiHandles.handle_L20.handle_M15 = iRSI("XAUUSD",PERIOD_M15,20,PRICE_CLOSE);
rsiHandles.handle_L20.handle_M30 = iRSI("XAUUSD",PERIOD_M30,20,PRICE_CLOSE);
rsiHandles.handle_L20.handle_H1 = iRSI("XAUUSD",PERIOD_H1,20,PRICE_CLOSE);
rsiHandles.handle_L20.handle_H4 = iRSI("XAUUSD",PERIOD_H4,20,PRICE_CLOSE);


rsiHandles.handle_L25.handle_M1 = iRSI("XAUUSD",PERIOD_M1,25,PRICE_CLOSE);
rsiHandles.handle_L25.handle_M5 = iRSI("XAUUSD",PERIOD_M5,25,PRICE_CLOSE);
rsiHandles.handle_L25.handle_M15 = iRSI("XAUUSD",PERIOD_M15,25,PRICE_CLOSE);
rsiHandles.handle_L25.handle_M30 = iRSI("XAUUSD",PERIOD_M30,25,PRICE_CLOSE);
rsiHandles.handle_L25.handle_H1 = iRSI("XAUUSD",PERIOD_H1,25,PRICE_CLOSE);
rsiHandles.handle_L25.handle_H4 = iRSI("XAUUSD",PERIOD_H4,25,PRICE_CLOSE);


rsiHandles.handle_L30.handle_M1 = iRSI("XAUUSD",PERIOD_M1,30,PRICE_CLOSE);
rsiHandles.handle_L30.handle_M5 = iRSI("XAUUSD",PERIOD_M5,30,PRICE_CLOSE);
rsiHandles.handle_L30.handle_M15 = iRSI("XAUUSD",PERIOD_M15,30,PRICE_CLOSE);
rsiHandles.handle_L30.handle_M30 = iRSI("XAUUSD",PERIOD_M30,30,PRICE_CLOSE);
rsiHandles.handle_L30.handle_H1 = iRSI("XAUUSD",PERIOD_H1,30,PRICE_CLOSE);
rsiHandles.handle_L30.handle_H4 = iRSI("XAUUSD",PERIOD_H4,30,PRICE_CLOSE);
}

void Reset_Rsi_Init()
{
RsiHandlesInited =false;
}

TimeFrameIndicatorHandlres GetAllTimeFramesHandle_RSI(int len)
{
if(len == 10) return rsiHandles.handle_L10;
if(len == 15) return rsiHandles.handle_L15;
if(len == 20) return rsiHandles.handle_L20;
if(len == 25) return rsiHandles.handle_L25;
if(len == 30) return rsiHandles.handle_L30;
return rsiHandles.handle_L15;
}

int Get_RSIHandle(ENUM_TIMEFRAMES timeFrame,int len)
{
TimeFrameIndicatorHandlres timeFrameHandle = GetAllTimeFramesHandle_RSI(len);
if(timeFrame == PERIOD_M1)  return timeFrameHandle.handle_M1;
if(timeFrame == PERIOD_M5)  return timeFrameHandle.handle_M5;
if(timeFrame == PERIOD_M15) return timeFrameHandle.handle_M15;
if(timeFrame == PERIOD_M30) return timeFrameHandle.handle_M30;
if(timeFrame == PERIOD_H1)  return timeFrameHandle.handle_H1;
if(timeFrame == PERIOD_H4)  return timeFrameHandle.handle_H4;
return INVALID_HANDLE;
}

double Get_Rsi(ENUM_TIMEFRAMES timeFrame,int indL,int shift=0)
  {
   if(!RsiHandlesInited)
   {
   RsiHandlesInited =true;
   InitHandles_RSI();
   }
   
   double rsi[2];
   int RSiHandle = Get_RSIHandle(timeFrame,indL);
   int copied = CopyBuffer(RSiHandle,0,shift,2,rsi);
   
   if(copied <= 0)
   {
      InitHandles_RSI();
      copied = CopyBuffer(RSiHandle, 0, shift, 2, rsi); // try again
      
      if(copied <=0)
      {
         Print("Critical: CopyBuffer failed even after reinit. Returning -1");
         return -1000.0;
      }
   }
   
   double rsiValue = rsi[0];
   return NormalizeDouble(rsiValue,2);
  }

double Get_Rsi_Slope(ENUM_TIMEFRAMES timeFrame,int indL,int shift=0)
{
    double first = Get_Rsi(timeFrame,indL,shift);
    double second = Get_Rsi(timeFrame,indL,shift + 1);
    
    double slope = CalcSlope(first,second);
    return NormalizeDouble(slope,2);
}
