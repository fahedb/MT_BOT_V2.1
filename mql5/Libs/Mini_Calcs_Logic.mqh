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

double CalcSlope(double first, double second)
{
    double slope = first - second;
    return slope;
}


string GetPriceInStr(int priceType, int shift = 1) // open = 0, close = 1, high = 2, low = 3
  {
   double priceD=0;
   string priceStr;

   if(priceType ==0)
     {
      priceD = iOpen("XAUUSD",PERIOD_M1,shift);
     }
   else
      if(priceType == 1)
        {
         priceD = iClose("XAUUSD",PERIOD_M1,shift);
        }
      else
         if(priceType == 2)
           {

            priceD = iHigh("XAUUSD",PERIOD_M1,shift);
           }
         else
            if(priceType == 3)
              {
               priceD = iLow("XAUUSD",PERIOD_M1,shift);
              }
   priceStr = DoubleToString(NormalizeDouble(priceD,_Digits));
   return priceStr;
  }
  
  string GetCurrFormattedTime()
  {

// Get the current time
   MqlDateTime dt = {};
// Get the current time
   datetime currentTime = TimeCurrent();
   datetime previousMinuteTime = currentTime - 60;

   TimeToStruct(previousMinuteTime, dt); // Fill the MqlDateTime structure
// Extract the individual components for the previous minute
   int year = dt.year;
   int month = dt.mon;
   int day = dt.day;
   int hour = dt.hour;

   int minute = dt.min;
   int second = 0;
   int millisecond = 0; // Get milliseconds

// Format the string
   string result = StringFormat("%04d%02d%02d%02d%02d%02d%03d",
                                year, month, day, hour, minute, second, millisecond);

   return result;
  }
