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
#import "C:\\Users\\Administrator\\Documents\\GitHub\\MT_BOT_V2.1\\MTDll\\x64\\Debug\\MTDll.dll"
// Dll Functions def
int IsOrderClosed(int seqNum);
#import

#include "Mini_Calcs_Logic.mqh"
#include "../Include/Trade/Trade.mqh"

void Update_TP_SL(int sl,int tp)
{
        // Get the current time
    datetime currentTime = TimeCurrent();
    CPositionInfo position;
    CTrade trade;
    // Loop through all open positions
    for (int i = 0; i < PositionsTotal() ; i++)
    {
        // Select the position by index
        if (position.SelectByIndex(i))
        {
            // Check if the position is a buy or sell
            if (position.PositionType() == POSITION_TYPE_BUY || position.PositionType() == POSITION_TYPE_SELL)
            {
                // Calculate the duration the position has been open
                datetime positionOpenTime = position.Time(); // Get the position open time
                double SL = position.StopLoss();
                double TP = position.TakeProfit();

                if(position.Profit() > 0 && position.PriceCurrent() > position.PriceOpen())
                {
                     
                     double BID = position.PriceCurrent();
                     if(TP - BID < 50 * _Point)
                     {   
                      CTrade trade;
                      trade.PositionModify(position.Ticket(),BID - sl * _Point,BID+ tp*_Point);
                     }    
               }
               
               if(position.Profit() > 0 && position.PriceCurrent() < position.PriceOpen())
               {
                 double ASK = position.PriceCurrent();
                 if(ASK - TP < 50 * _Point)
                 {
                     CTrade trade;
                     trade.PositionModify(position.Ticket(),ASK + sl * _Point,ASK- tp *_Point);
                 }
               
               }
                      
            }

         }
    }
 }

void RefreshOrders()
{
   CTrade trade;
   CPositionInfo position;
   for (int i = 0; i < PositionsTotal() ; i++)
   {
      // Select the position by index
      if (position.SelectByIndex(i))
      {
         string seqNum = position.Comment();
         int isClosedOrder = IsOrderClosed(StringToInteger(seqNum));
         if(isClosedOrder)
            trade.PositionClose(position.Ticket());
         
      }
   
   }


}
