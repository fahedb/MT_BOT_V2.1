//+------------------------------------------------------------------+
//|                                                 PricesFeeder.mq5 |
//|                                  Copyright 2024, MetaQuotes Ltd. |
//|                                             https://www.mql5.com |
//+------------------------------------------------------------------+
#property copyright "Copyright 2024, MetaQuotes Ltd."
#property link      "https://www.mql5.com"
#property version   "1.00"
//+------------------------------------------------------------------+
//| Expert initialization function                                   |
//+------------------------------------------------------------------+

#include "../Include/CandleStructs_Logic.mqh"
#include "../Include/Modify_Orders_Logic.mqh"
#include "../Include/DLL_OrderStruct.mqh"
#include "../Include/MT_Policy.mqh"

#import "C:\\MTSOURCE\\MT_BOT_V2.1\\MTDll\\x64\\Debug\\MTDll.dll"
// Dll Functions def
bool InsertNewBar_HTTP_M1(char&[],char&[],CandleData&);
bool InsertNewBar_HTTP_M1_JSON_DB(char&[],char&[],CandleData&);
bool InsertNewBar_M1_JSON_DB(char&[],char&[],CandleData&);
bool InsertNewBar_AndWaitForResult_HTTP_M1(char&[],char&[],CandleData&,DLL_OrderStruct& );
int CheckForNewOrders_HTTP(DLL_OrderStruct&);
void WaitForScanFinish();
#import

bool m_minSwitch;
int last_min=0;
double m_NewBarOpenPrice=0;
int m_totalTicks=0;
int m_upperTicks=0;
int m_lowerTicks=0;
int m_backTestBarCounter = 0;

CPositionInfo m_position;
CTrade m_trade;
int ticksCounter;
bool m_syncScanWithServer= true; // Change this to true when you're in a history mode

int OnInit()
  {
//---
   //TruncCurrTableForFreshStart();
    RefreshPolicy();
    if(!MQLInfoInteger(MQL_TESTER))
      Init_Last_Page_Of_Bars();
     ticksCounter =0;
     //ExecuteNewOrder();
//   bool resss = InsertNewPriceRecord(test,test,data);
//---*/6
   return(INIT_SUCCEEDED); 
  }
//+------------------------------------------------------------------+
//| Expert deinitialization function                                 |
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
  {
//---

  }
//+------------------------------------------------------------------+
//| Expert tick function                                             |
//+------------------------------------------------------------------+
void OnTick()
  {
//---
   if(Symbol() != "XAUUSD")
      return;
      
   Update_TP_SL(Get_SL_Diff_Policy(),Get_TP_Diff_Policy());
   //SupportOpenedOrders();
   CountGravity();
   
   ticksCounter++;
   if(ticksCounter >= 4)
   {
      ticksCounter = 0;
      //ExecuteNewOrder();
      //RefreshOrders();
   }
   
   MqlDateTime dt= {};
   datetime dateT = TimeCurrent(dt);
   int current_min = dt.min;
   if(current_min > last_min || (current_min ==0 && m_minSwitch))
     {
      if(MQLInfoInteger(MQL_TESTER))
      {
         if(m_backTestBarCounter < 90)
            {
               m_backTestBarCounter++;
               if(current_min >0)
                  m_minSwitch= true;
               else
                  m_minSwitch= false;
               last_min=current_min;
               return;
            }
          if(m_backTestBarCounter == 90)
            {
               Init_Last_Page_Of_Bars();
               m_backTestBarCounter++;
            }
          if(m_backTestBarCounter > 90)
          {
            //BuildAndSendNewBarDetails(current_min);
            SendNewBarAndWaitForScan(current_min);
            RefreshPolicy();
          }
      }
      else{
         SendNewBarAndWaitForScan(current_min);
         RefreshPolicy();
      }
      //testing(1);
      if(current_min >0)
         m_minSwitch= true;
      else
         m_minSwitch= false;
      last_min=current_min;
      //if(MQLInfoInteger(MQL_TESTER))
      //   WaitForScanFinish(); // Wait till the scan end (Relevant only for history check mode)
     }
  }

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void CountGravity()
  {
   double currPrice = NormalizeDouble(SymbolInfoDouble("XAUUSD",SYMBOL_ASK),_Digits);
   if(currPrice < m_NewBarOpenPrice)
      m_lowerTicks++;
   if(currPrice > m_NewBarOpenPrice)
      m_upperTicks++;
   m_totalTicks++;
  }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

void ExecuteNewOrder()
{
   DLL_OrderStruct order;
   int newOrder = CheckForNewOrders_HTTP(order);
   if(!newOrder)
      return;
   OrderType orderT = order.type;   
   OrderStatus orderS = order.status;
   CoinsPair coinP = order.pair;
   int stopL = order.stopLoss;
   int takeP = order.takeProfit;
   int seqN = order.seqNum;
   int total_Orders = PositionsTotal();
   bool ress;
   
   if(orderS == OrderStatus::OS_OPEN && total_Orders < Get_MaxOrdersAllowed_Policy())
   {
      CTrade trade;
      double Ask = NormalizeDouble(SymbolInfoDouble("XAUUSD",SYMBOL_ASK),_Digits);
      double orderSize = Get_MainOrderSize_Policy();
      if(orderT == OrderType::OT_LONG)
      {
         ress=trade.Buy(orderSize,"XAUUSD",0,Ask - stopL * _Point,Ask+ takeP * _Point,IntegerToString(seqN));
      }
      else if(orderT == OrderType::OT_SHORT)
      {
         ress=trade.Sell(orderSize,"XAUUSD",0,Ask + stopL * _Point,Ask - takeP * _Point,IntegerToString(seqN));
      }
   
   }
}

void SendNewBarAndWaitForScan(int current_min)
{
   CandleData data = Get_CandleData(1);
   m_NewBarOpenPrice = NormalizeDouble(SymbolInfoDouble("XAUUSD",SYMBOL_ASK),_Digits);

   char currTimeArr[];
   StringToCharArray(GetCurrFormattedTime(),currTimeArr);
   StringToCharArray(GetCurrFormattedTime(),data.timePoint,0,20);
   
   char symbolArr[];
   StringToCharArray("XAUUSD",symbolArr);

   data.totalTicks = m_totalTicks;
   data.upperTicks = m_upperTicks;
   data.lowerTicks = m_lowerTicks;

   DLL_OrderStruct order;
   bool newOrder = InsertNewBar_AndWaitForResult_HTTP_M1(currTimeArr,symbolArr,data, order);
   
   if(!newOrder)
      return;

   OrderType orderT = order.type;   
   OrderStatus orderS = order.status;
   CoinsPair coinP = order.pair;
   int stopL = order.stopLoss;
   int takeP = order.takeProfit;
   int seqN = order.seqNum;
   int total_Orders = PositionsTotal();
   bool ress;
   
   if(orderS == OrderStatus::OS_OPEN && total_Orders < Get_MaxOrdersAllowed_Policy())
   {
      CTrade trade;
      double Ask = NormalizeDouble(SymbolInfoDouble("XAUUSD",SYMBOL_ASK),_Digits);
      double orderSize = Get_MainOrderSize_Policy();
      if(orderT == OrderType::OT_LONG)
      {
         ress=trade.Buy(orderSize,"XAUUSD",0,Ask - stopL * _Point,Ask+ takeP * _Point,IntegerToString(seqN));
      }
      else if(orderT == OrderType::OT_SHORT)
      {
         ress=trade.Sell(orderSize,"XAUUSD",0,Ask + stopL * _Point,Ask - takeP * _Point,IntegerToString(seqN));
      }
   
   }

}

void BuildAndSendNewBarDetails(int current_min)
  {
   CandleData data = Get_CandleData(1);
   m_NewBarOpenPrice = NormalizeDouble(SymbolInfoDouble("XAUUSD",SYMBOL_ASK),_Digits);

   char currTimeArr[];
   StringToCharArray(GetCurrFormattedTime(),currTimeArr);
   StringToCharArray(GetCurrFormattedTime(),data.timePoint,0,20);
   
   char symbolArr[];
   StringToCharArray("XAUUSD",symbolArr);

   data.totalTicks = m_totalTicks;
   data.upperTicks = m_upperTicks;
   data.lowerTicks = m_lowerTicks;
   bool resss = InsertNewBar_HTTP_M1(currTimeArr,symbolArr,data); 
   //bool res = InsertNewBar_M1_JSON_DB(currTimeArr,symbolArr,data);
   m_totalTicks=0;
   m_upperTicks=0;
   m_lowerTicks=0;
  }

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+

void Init_Last_Page_Of_Bars()
{

for(int i =90;i > 0;i--)
{
   Bar_By_Shift(i);
}

}

void Bar_By_Shift(int shift)
{
  
   
   string timePointStr = IntegerToString(1800 - shift);
   char timePointArr[];
   StringToCharArray(timePointStr,timePointArr);
   
   CandleData data = Get_CandleData(shift);
   data.totalTicks = 0;
   data.upperTicks = 0;
   data.lowerTicks = 0;
   
   char symbolArr[];
   StringToCharArray("XAUUSD",symbolArr);
   
   bool resss = InsertNewBar_HTTP_M1(timePointArr,symbolArr,data);
   
}