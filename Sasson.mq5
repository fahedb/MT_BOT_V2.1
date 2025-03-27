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

#include "../Include/Trade/Trade.mqh"
struct CandleData {
	char openPrice[8];
	char closePrice[8];
	char highPrice[8];
	char lowPrice[8];

	/// EMA values and slopes
	char ema_M1[8];
	char ema_slope_M1[8];
	char ema_M5[8];
	char ema_slope_M5[8];
	char ema_M15[8];
	char ema_slope_M15[8];
	
	char ema_M30[8];
	char ema_slope_M30[8];
	
	char ema_H1[8];
	char ema_slope_H1[8];
	
	char ema_H4[8];
	char ema_slope_H4[8];

	/// ZLSMA values and slopes
	char zlsma_M1[8];
	char zlsma_slope_M1[8];
	char zlsma_M5[8];
	char zlsma_slope_M5[8];
	char zlsma_M15[8];
	char zlsma_slope_M15[8];
	char zlsma_M30[8];
	char zlsma_slope_M30[8];
	char zlsma_H1[8];
	char zlsma_slope_H1[8];
	char zlsma_H4[8];
	char zlsma_slope_H4[8];


	/// RSI values and slopes
	char rsi_M1[8];
	char rsi_slope_M1[8];
	char rsi_M5[8];
	char rsi_slope_M5[8];
	char rsi_M15[8];
	char rsi_slope_M15[8];
	char rsi_M30[8];
	char rsi_slope_M30[8];
	char rsi_H1[8];
	char rsi_slope_H1[8];
	
	char rsi_H4[8];
	char rsi_slope_H4[8];

	/// Gravity values
	int totalTicks;
	int upperTicks;
	int lowerTicks;
};

struct tests{
int i;
int t;
char str[8];
};

#import "C:\\Windows\\SysWOW64\\MTDll.dll"
// Dll Functions def
bool testing(int);
string ReadLastOrderFromFile();
bool UpdateLineFromFile(char&[],int);
bool CopyNewOrder(char&[],char&[]);
bool SetCancledOrder(char&[]);
string GetCanceledOrder();
string GetOrderByLine(int);
int TotalOpenOrdersCount();
bool SendNewBarDetailsToFileExtended(char&[],char&[],CandleData&);
bool TruncCurrTableForFreshStart();
bool InsertNewPriceRecord(char&[],char&[],CandleData&);
int CheckForNewOrders();
bool testingstruct(CandleData&);
#import

const int m_Ind_Period = 30;
double min_ma =50000;
double max_ma = 0;

bool m_minSwitch;
int last_min=0;
double m_NewBarOpenPrice=0;
int m_totalTicks=0;
int m_upperTicks=0;
int m_lowerTicks=0;

CPositionInfo m_position;
CTrade m_trade;

int OnInit()
  {
//---
CTrade trade;

       
   //TruncCurrTableForFreshStart();

  //   Init_Last_Page_Of_Bars();

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
   Update_TP_SL();
   CloseAllExpiredOrders();
   SupportOpenedOrders();
   CountGravity();
   //OpenOrder();
   MqlDateTime dt= {};
   datetime dateT = TimeCurrent(dt);
   int current_min = dt.min;
   if(current_min > last_min || (current_min ==0 && m_minSwitch))
     {
   int newOrder = CheckForNewOrders();

   int total_Orders = PositionsTotal();
   bool ress;
      if(newOrder != -1 && total_Orders <= 9)
      {
         CTrade trade;
         double Ask = NormalizeDouble(SymbolInfoDouble("XAUUSD",SYMBOL_ASK),_Digits);
         if(newOrder == 0)
            ress=trade.Buy(0.2,"XAUUSD",0,Ask - 550 * _Point,Ask+ 130 * _Point);
         else
            ress=trade.Sell(0.2,"XAUUSD",0,Ask + 550 * _Point,Ask - 130 * _Point);
         
      }
     
     BuildAndSendNewBarDetails(current_min);
     
     // testing(1);
      if(current_min >0)
         m_minSwitch= true;
      else
         m_minSwitch= false;
      last_min=current_min;
     }

/*
   OpenOrder();
   CountGravity();
   MqlDateTime dt= {};
   datetime dateT = TimeCurrent(dt);
   int current_min = dt.min;

   if(current_min > last_min || (current_min ==0 && m_minSwitch))
     {
      BuildAndSendNewBarDetails(current_min);
         if(current_min >0)
      m_minSwitch= true;
   else
      m_minSwitch= false;
   last_min=current_min;
     }
*/
  }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void OpenOrder()
  {
   int total_Orders = PositionsTotal();
   if(total_Orders < 2)
     {

      CTrade trade;
      double rsi[2];
      int RSiHandle = iRSI("XAUUSD",PERIOD_M1,14,PRICE_CLOSE);
      CopyBuffer(RSiHandle,0,1,2,rsi);
      double rsiValue = rsi[0];

      double Ask = NormalizeDouble(SymbolInfoDouble("XAUUSD",SYMBOL_ASK),_Digits);
      if(rsiValue == 0)
         return ;
      if(rsiValue < 28)
         trade.Buy(0.01,"XAUUSD",0,Ask - 100 * _Point,Ask + 50 * _Point);
      if(rsiValue > 72)
         trade.Sell(0.01,"XAUUSD",0,Ask + 100 * _Point,Ask - 50 * _Point);

     }
  }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
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

string GetLastSlope(int indicator,ENUM_TIMEFRAMES timeFrame,int shift = 0)
{
      string first;
      string second;
      if(indicator==0)
      {
         first = GetRsi(timeFrame,shift);
         second = GetRsi(timeFrame,shift + 1);
      }
      else if(indicator == 1)
      {
         first = GetEma(timeFrame,shift);
         second = GetEma(timeFrame,shift + 1);
      }
      else if(indicator == 2)
      {
      first = GetZLSMA(timeFrame,shift);
      second = GetZLSMA(timeFrame,shift + 1);
      }
      
      double slope = CalcSlope(NormalizeDouble(StringToDouble(first),_Digits),NormalizeDouble(StringToDouble(second),_Digits));
      return DoubleToString(NormalizeDouble(slope,_Digits));
}

double CalcSlope(double first, double second)
{
    double slope = first - second;
    return slope;
}

void BuildAndSendNewBarDetails(int current_min)
  {
  bool test = testing(1);
  bool test2= testing(0);
  CandleData data;
   m_NewBarOpenPrice = NormalizeDouble(SymbolInfoDouble("XAUUSD",SYMBOL_ASK),_Digits);

   string openPriceStr =  GetPriceInStr(0);
   StringToCharArray(openPriceStr,data.openPrice,0,7);
   
   string closePriceStr = GetPriceInStr(1);
   StringToCharArray(closePriceStr,data.closePrice,0,7);
   
   
   string highPriceStr = GetPriceInStr(2);
   StringToCharArray(highPriceStr,data.highPrice,0,7);

   
   string lowPriceStr = GetPriceInStr(3);
   StringToCharArray(lowPriceStr,data.lowPrice,0,7);

   char currTimeArr[];
   StringToCharArray(GetCurrFormattedTime(),currTimeArr);
   char symbolArr[];
   StringToCharArray("XAUUSD",symbolArr);

   string rsiM1str = GetRsi(PERIOD_M1,0);
   StringToCharArray(rsiM1str,data.rsi_M1,0,7);
   string rsiM1Slope = GetLastSlope(0,PERIOD_M1);
   StringToCharArray(rsiM1Slope,data.rsi_slope_M1,0,7);
   
   
   string rsiM5str = GetRsi(PERIOD_M5,0);
   StringToCharArray(rsiM5str,data.rsi_M5,0,7);
   string rsiM5Slope = GetLastSlope(0,PERIOD_M5);
   StringToCharArray(rsiM5Slope,data.rsi_slope_M5,0,7);
  
   
   string rsiM15str = GetRsi(PERIOD_M15,0);
   StringToCharArray(rsiM15str,data.rsi_M15,0,7);
   string rsiM15Slope = GetLastSlope(0,PERIOD_M15);
   StringToCharArray(rsiM15Slope,data.rsi_slope_M15,0,7);

   string rsiM30str = GetRsi(PERIOD_M30,0);
   StringToCharArray(rsiM30str,data.rsi_M30,0,7);
   string rsiM30Slope = GetLastSlope(0,PERIOD_M30);
   StringToCharArray(rsiM30Slope,data.rsi_slope_M30,0,7);
   
   string rsiH1str = GetRsi(PERIOD_H1,0);
   StringToCharArray(rsiH1str,data.rsi_H1,0,7);
   string rsiH1Slope = GetLastSlope(0,PERIOD_H1);
   StringToCharArray(rsiH1Slope,data.rsi_slope_H1,0,7);
   

   string rsiH4str = GetRsi(PERIOD_H4,0);
   StringToCharArray(rsiH4str,data.rsi_H4,0,7);
   string rsiH4Slope = GetLastSlope(0,PERIOD_H4);
   StringToCharArray(rsiH4Slope,data.rsi_slope_H4,0,7);
   


   string emaM1str = GetEma(PERIOD_M1,0);
   StringToCharArray(emaM1str,data.ema_M1,0,7);
   string emaM1Slope = GetLastSlope(1,PERIOD_M1);
   StringToCharArray(emaM1Slope,data.ema_slope_M1,0,7);
   

   string emaM5str = GetEma(PERIOD_M5,0);
   StringToCharArray(emaM5str,data.ema_M5,0,7);
   string emaM5Slope = GetLastSlope(1,PERIOD_M5);
   StringToCharArray(emaM5Slope,data.ema_slope_M5,0,7);
   if(min_ma > StringToDouble(emaM5str))
      min_ma=StringToDouble(emaM5str);
   if(max_ma < StringToDouble(emaM5str))
      max_ma=StringToDouble(emaM5str);

   string emaM15str = GetEma(PERIOD_M15,0);
   StringToCharArray(emaM15str,data.ema_M15,0,7);
   string emaM15Slope = GetLastSlope(1,PERIOD_M15);
   StringToCharArray(emaM15Slope,data.ema_slope_M15,0,7);
   if(min_ma > StringToDouble(emaM15str))
      min_ma=StringToDouble(emaM15str);
   if(max_ma < StringToDouble(emaM15str))
      max_ma=StringToDouble(emaM15str);
   
   string emaM30str = GetEma(PERIOD_M30,0);
   StringToCharArray(emaM30str,data.ema_M30,0,7);
   string emaM30Slope = GetLastSlope(1,PERIOD_M30);
   StringToCharArray(emaM30Slope,data.ema_slope_M30,0,7);
   if(min_ma > StringToDouble(emaM30str))
      min_ma=StringToDouble(emaM30str);
   if(max_ma < StringToDouble(emaM30str))
      max_ma=StringToDouble(emaM30str);

   string emaH1str = GetEma(PERIOD_H1,0);
   StringToCharArray(emaH1str,data.ema_H1,0,7);
   string emaH1Slope = GetLastSlope(1,PERIOD_H1);
   StringToCharArray(emaH1Slope,data.ema_slope_H1,0,7);
   if(min_ma > StringToDouble(emaH1str))
      min_ma=StringToDouble(emaH1str);
   if(max_ma < StringToDouble(emaH1str))
      max_ma=StringToDouble(emaH1str);


   string emaH4str = GetEma(PERIOD_H4,0);
   StringToCharArray(emaH4str,data.ema_H4,0,7);
   string emaH4Slope = GetLastSlope(1,PERIOD_H4);
   StringToCharArray(emaH4Slope,data.ema_slope_H4,0,7);
   if(min_ma > StringToDouble(emaH4str))
      min_ma=StringToDouble(emaH4str);
   if(max_ma < StringToDouble(emaH4str))
      max_ma=StringToDouble(emaH4str);


   string zlsmaM1Str = GetZLSMA(PERIOD_M1,0);
   StringToCharArray(zlsmaM1Str,data.zlsma_M1,0,7);
   string zlsmaM1Slope = GetLastSlope(2,PERIOD_M1);
   StringToCharArray(zlsmaM1Slope,data.zlsma_slope_M1,0,7);
   
   
   string zlsmaM5Str = GetZLSMA(PERIOD_M5,0);
   StringToCharArray(zlsmaM5Str,data.zlsma_M5,0,7);
   string zlsmaM5Slope = GetLastSlope(2,PERIOD_M5);
   StringToCharArray(zlsmaM5Slope,data.zlsma_slope_M5,0,7);
   if(min_ma > StringToDouble(emaM5str))
      min_ma=StringToDouble(emaM5str);
   if(max_ma < StringToDouble(emaM5str))
      max_ma=StringToDouble(emaM5str);
      
   string zlsmaM15Str = GetZLSMA(PERIOD_M15,0);
   StringToCharArray(zlsmaM15Str,data.zlsma_M15,0,7);
   string zlsmaM15Slope = GetLastSlope(2,PERIOD_M15);
   StringToCharArray(zlsmaM15Slope,data.zlsma_slope_M15,0,7);
      if(min_ma > StringToDouble(emaM15str))
         min_ma=StringToDouble(emaM15str);
      if(max_ma < StringToDouble(emaM15str))
         max_ma=StringToDouble(emaM15str);

   string zlsmaM30Str = GetZLSMA(PERIOD_M30,0);
   StringToCharArray(zlsmaM30Str,data.zlsma_M30,0,7);
   string zlsmaM30Slope = GetLastSlope(2,PERIOD_M30);
   StringToCharArray(zlsmaM30Slope,data.zlsma_slope_M30,0,7);
   if(min_ma > StringToDouble(emaM30str))
      min_ma=StringToDouble(emaM30str);
   if(max_ma < StringToDouble(emaM30str))
      max_ma=StringToDouble(emaM30str);

   string zlsmaH1Str = GetZLSMA(PERIOD_H1,0);
   StringToCharArray(zlsmaH1Str,data.zlsma_H1,0,7);
   string zlsmaH1Slope = GetLastSlope(2,PERIOD_H1);
   StringToCharArray(zlsmaH1Slope,data.zlsma_slope_H1,0,7);
   if(min_ma > StringToDouble(emaH1str))
      min_ma=StringToDouble(emaH1str);
   if(max_ma < StringToDouble(emaH1str))
      max_ma=StringToDouble(emaH1str);

   string zlsmaH4Str = GetZLSMA(PERIOD_H4,0);
   StringToCharArray(zlsmaH4Str,data.zlsma_H4,0,7);
   string zlsmaH4Slope = GetLastSlope(2,PERIOD_H4);
   StringToCharArray(zlsmaH4Slope,data.zlsma_slope_H4,0,7);
   if(min_ma > StringToDouble(emaH4str))
      min_ma=StringToDouble(emaH4str);
   if(max_ma < StringToDouble(emaH4str))
      max_ma=StringToDouble(emaH4str);


   data.totalTicks = m_totalTicks;
   data.upperTicks = m_upperTicks;
   data.lowerTicks = m_lowerTicks;
   bool resss = SendNewBarDetailsToFileExtended(currTimeArr,symbolArr,data); 
   //ddOpenOrderByMA(StringToDouble(closePriceStr));
   
   max_ma =0;
   min_ma =50000;
   m_totalTicks=0;
   m_upperTicks=0;
   m_lowerTicks=0;
  }

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void OpenOrderByMA(double closePr)
{

   int total_Orders = PositionsTotal();
   if(total_Orders >= 2)
      return;
CTrade trade;
bool res;
if(closePr > max_ma)
{
   double Ask = NormalizeDouble(SymbolInfoDouble("XAUUSD",SYMBOL_ASK),_Digits);
    res = trade.Buy(0.1,Symbol(),0,Ask - 30 * _Point,Ask + 100 * _Point);
}
if(closePr < min_ma)
{
   double Ask = NormalizeDouble(SymbolInfoDouble("XAUUSD",SYMBOL_ASK),_Digits);      
   res = trade.Sell(0.1,Symbol(),0,Ask + 30 * _Point,Ask - 100 * _Point);
}

}

void Init_Last_Page_Of_Bars()
{

for(int i =1;i< 121;i++)
{
   Bar_By_Shift(i);
}

}

void Bar_By_Shift(int shift)
{
   CandleData data;
   
   string timePointStr = IntegerToString(1800 - shift);
   char timePointArr[];
   StringToCharArray(timePointStr,timePointArr);
   
   string openPriceStr =  GetPriceInStr(0,shift);
   StringToCharArray(openPriceStr,data.openPrice,0,7);
   
   string closePriceStr = GetPriceInStr(1,shift);
   StringToCharArray(closePriceStr,data.closePrice,0,7);
   
   
   string highPriceStr = GetPriceInStr(2,shift);
   StringToCharArray(highPriceStr,data.highPrice,0,7);

   
   string lowPriceStr = GetPriceInStr(3,shift);
   StringToCharArray(lowPriceStr,data.lowPrice,0,7);

   char currTimeArr[];
   StringToCharArray(GetCurrFormattedTime(),currTimeArr);
   char symbolArr[];
   StringToCharArray("XAUUSD",symbolArr);

   string rsiM1str = GetRsi(PERIOD_M1,shiftCalc(PERIOD_M1,shift));
   StringToCharArray(rsiM1str,data.rsi_M1,0,7);
   string rsiM1Slope = GetLastSlope(0,PERIOD_M1,shiftCalc(PERIOD_M1,shift));
   StringToCharArray(rsiM1Slope,data.rsi_slope_M1,0,7);
   
   string rsiM5str = GetRsi(PERIOD_M5,shiftCalc(PERIOD_M5,shift));
   StringToCharArray(rsiM5str,data.rsi_M5,0,7);
   string rsiM5Slope = GetLastSlope(0,PERIOD_M5,shiftCalc(PERIOD_M5,shift));
   StringToCharArray(rsiM5Slope,data.rsi_slope_M5,0,7);
   
   string rsiM15str = GetRsi(PERIOD_M15,shiftCalc(PERIOD_M15,shift));
   StringToCharArray(rsiM15str,data.rsi_M15,0,7);
   string rsiM15Slope = GetLastSlope(0,PERIOD_M15,shiftCalc(PERIOD_M15,shift));
   StringToCharArray(rsiM15Slope,data.rsi_slope_M15,0,7);
   

   string rsiM30str = GetRsi(PERIOD_M30,shiftCalc(PERIOD_M30,shift));
   StringToCharArray(rsiM30str,data.rsi_M30,0,7);
   string rsiM30Slope = GetLastSlope(0,PERIOD_M30,shiftCalc(PERIOD_M30,shift));
   StringToCharArray(rsiM30Slope,data.rsi_slope_M30,0,7);
   

   string rsiH1str = GetRsi(PERIOD_H1,shiftCalc(PERIOD_H1,shift));
   StringToCharArray(rsiH1str,data.rsi_H1,0,7);
   string rsiH1Slope = GetLastSlope(0,PERIOD_H1,shiftCalc(PERIOD_H1,shift));
   StringToCharArray(rsiH1Slope,data.rsi_slope_H1,0,7);
   

   string rsiH4str = GetRsi(PERIOD_H4,shiftCalc(PERIOD_H4,shift));
   StringToCharArray(rsiH4str,data.rsi_H4,0,7);
   string rsiH4Slope = GetLastSlope(0,PERIOD_H4,shiftCalc(PERIOD_H4,shift));
   StringToCharArray(rsiH4Slope,data.rsi_slope_H4,0,7);
   


   string emaM1str = GetEma(PERIOD_M1,shiftCalc(PERIOD_M1,shift));
   StringToCharArray(emaM1str,data.ema_M1,0,7);
   string emaM1Slope = GetLastSlope(1,PERIOD_M1,shiftCalc(PERIOD_M1,shift));
   StringToCharArray(emaM1Slope,data.ema_slope_M1,0,7);
   

   string emaM5str = GetEma(PERIOD_M5,shiftCalc(PERIOD_M5,shift));
   StringToCharArray(emaM5str,data.ema_M5,0,7);
   string emaM5Slope = GetLastSlope(1,PERIOD_M5,shiftCalc(PERIOD_M5,shift));
   StringToCharArray(emaM5Slope,data.ema_slope_M5,0,7);
   

   string emaM15str = GetEma(PERIOD_M15,shiftCalc(PERIOD_M15,shift));
   StringToCharArray(emaM15str,data.ema_M15,0,7);
   string emaM15Slope = GetLastSlope(1,PERIOD_M15,shiftCalc(PERIOD_M15,shift));
   StringToCharArray(emaM15Slope,data.ema_slope_M15,0,7);
   
   string emaM30str = GetEma(PERIOD_M30,shiftCalc(PERIOD_M30,shift));
   StringToCharArray(emaM30str,data.ema_M30,0,7);
   string emaM30Slope = GetLastSlope(1,PERIOD_M30,shiftCalc(PERIOD_M30,shift));
   StringToCharArray(emaM30Slope,data.ema_slope_M30,0,7);
   
   string emaH1str = GetEma(PERIOD_H1,shiftCalc(PERIOD_H1,shift));
   StringToCharArray(emaH1str,data.ema_H1,0,7);
   string emaH1Slope = GetLastSlope(1,PERIOD_H1,shiftCalc(PERIOD_H1,shift));
   StringToCharArray(emaH1Slope,data.ema_slope_H1,0,7);
   

   string emaH4str = GetEma(PERIOD_H4,shiftCalc(PERIOD_H4,shift));
   StringToCharArray(emaH4str,data.ema_H4,0,7);
   string emaH4Slope = GetLastSlope(1,PERIOD_H4,shiftCalc(PERIOD_H4,shift));
   StringToCharArray(emaH4Slope,data.ema_slope_H4,0,7);
   

   string zlsmaM1Str = GetZLSMA(PERIOD_M1,shiftCalc(PERIOD_M1,shift));
   StringToCharArray(zlsmaM1Str,data.zlsma_M1,0,7);
   string zlsmaM1Slope = GetLastSlope(2,PERIOD_M1,shiftCalc(PERIOD_M1,shift));
   StringToCharArray(zlsmaM1Slope,data.zlsma_slope_M1,0,7);
   
   
   string zlsmaM5Str = GetZLSMA(PERIOD_M5,shiftCalc(PERIOD_M5,shift));
   StringToCharArray(zlsmaM5Str,data.zlsma_M5,0,7);
   string zlsmaM5Slope = GetLastSlope(2,PERIOD_M5,shiftCalc(PERIOD_M5,shift));
   StringToCharArray(zlsmaM5Slope,data.zlsma_slope_M5,0,7);
      
   string zlsmaM15Str = GetZLSMA(PERIOD_M15,shiftCalc(PERIOD_M15,shift));
   StringToCharArray(zlsmaM15Str,data.zlsma_M15,0,7);
   string zlsmaM15Slope = GetLastSlope(2,PERIOD_M15,shiftCalc(PERIOD_M15,shift));
   StringToCharArray(zlsmaM15Slope,data.zlsma_slope_M15,0,7);
   
   string zlsmaM30Str = GetZLSMA(PERIOD_M30,shiftCalc(PERIOD_M30,shift));
   StringToCharArray(zlsmaM30Str,data.zlsma_M30,0,7);
   string zlsmaM30Slope = GetLastSlope(2,PERIOD_M30,shiftCalc(PERIOD_M30,shift));
   StringToCharArray(zlsmaM30Slope,data.zlsma_slope_M30,0,7);
   
   string zlsmaH1Str = GetZLSMA(PERIOD_H1,shiftCalc(PERIOD_H1,shift));
   StringToCharArray(zlsmaH1Str,data.zlsma_H1,0,7);
   string zlsmaH1Slope = GetLastSlope(2,PERIOD_H1,shiftCalc(PERIOD_H1,shift));
   StringToCharArray(zlsmaH1Slope,data.zlsma_slope_H1,0,7);
   
   string zlsmaH4Str = GetZLSMA(PERIOD_H4,shiftCalc(PERIOD_H4,shift));
   StringToCharArray(zlsmaH4Str,data.zlsma_H4,0,7);
   string zlsmaH4Slope = GetLastSlope(2,PERIOD_H4,shiftCalc(PERIOD_H4,shift));
   StringToCharArray(zlsmaH4Slope,data.zlsma_slope_H4,0,7);
   
   data.totalTicks = 0;
   data.upperTicks = 0;
   data.lowerTicks = 0;

   StringToCharArray("XAUUSD",symbolArr);
   
   bool resss = InsertNewPriceRecord(timePointArr,symbolArr,data);
   
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

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
string GetRsi(ENUM_TIMEFRAMES timeFrame,int shift =1)
  {
   double rsi[2];
   int RSiHandle = iRSI("XAUUSD",timeFrame,m_Ind_Period,PRICE_CLOSE);
   CopyBuffer(RSiHandle,0,shift,2,rsi);
   double rsiValue = rsi[0];
   string rsiStr = DoubleToString(NormalizeDouble(rsiValue,_Digits));
   return rsiStr;
  }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
string GetEma(ENUM_TIMEFRAMES timeFrame,int shift=0)
  {
   double ema[2];
   int emaHandle = iMA("XAUUSD", timeFrame, m_Ind_Period,shift, MODE_EMA, PRICE_CLOSE);
   CopyBuffer(emaHandle,0,0,2,ema);
   string emaStr = DoubleToString(NormalizeDouble(ema[0],_Digits));
   return emaStr;
  }

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
string GetZLSMA(ENUM_TIMEFRAMES timeFrame,int shift = 0)
  {

// Calculate the Simple Moving Average (SMA)
   
   double smaCurrentArr[2];
   int smaCurrentHandle = iMA("XAUUSD", timeFrame, m_Ind_Period, shift, MODE_SMA, PRICE_CLOSE);
   CopyBuffer(smaCurrentHandle,0,1,2,smaCurrentArr);
   double smaCurrent = smaCurrentArr[0];
   
// Calculate the previous Simple Moving Average (SMA)
   double smaPreviousArr[2];
   int smaPreviouHandle = iMA("XAUUSD", timeFrame, m_Ind_Period,shift + 1, MODE_SMA, PRICE_CLOSE);
   CopyBuffer(smaPreviouHandle,0,1,2,smaPreviousArr);
   double smaPrevious = smaPreviousArr[0];
   
// Calculate the Zero-Lag Smooth Moving Average
   double zlsma = (smaCurrent * 2) - smaPrevious;
   string zlsmaStr = DoubleToString(NormalizeDouble(zlsma,_Digits));
   return zlsmaStr;
  }
  
  string GetGravity()
  {
  string totalTicks = IntegerToString(m_totalTicks);
  string upperTicks = IntegerToString(m_upperTicks);
  string lowerTicks = IntegerToString(m_lowerTicks);
  
  string GravityStr = totalTicks + ";" + upperTicks +";" + lowerTicks;
  return GravityStr;
  }
//+------------------------------------------------------------------+


int shiftCalc(ENUM_TIMEFRAMES timeFrame, int minute)
{
if(timeFrame == PERIOD_M1)
return minute;

if(timeFrame == PERIOD_M5)
return MathFloor((minute) / 5.0);

if(timeFrame == PERIOD_M15)
return MathFloor(minute / 15);


if(timeFrame == PERIOD_M30)
return MathFloor(minute / 30);


if(timeFrame == PERIOD_H1)
return MathFloor(minute / 60);


if(timeFrame == PERIOD_H4)
return MathFloor(minute / 240);

return minute;
}

void Update_TP_SL()
{
        // Get the current time
    datetime currentTime = TimeCurrent();

    // Loop through all open positions
    for (int i = 0; i < PositionsTotal() ; i++)
    {
        // Select the position by index
        if (m_position.SelectByIndex(i))
        {
            // Check if the position is a buy or sell
            if (m_position.PositionType() == POSITION_TYPE_BUY || m_position.PositionType() == POSITION_TYPE_SELL)
            {
                // Calculate the duration the position has been open
                datetime positionOpenTime = m_position.Time(); // Get the position open time
                double SL = m_position.StopLoss();
                double TP = m_position.TakeProfit();

                if(m_position.Profit() > 0 && m_position.PriceCurrent() > m_position.PriceOpen())
                {
                     
                     double BID = m_position.PriceCurrent();
                     if(TP - BID < 50 * _Point)
                     {
                      
                      CTrade trade;
                      trade.PositionModify(m_position.Ticket(),BID - 70 * _Point,BID+70*_Point);
                     }    
               }
               
               if(m_position.Profit() > 0 && m_position.PriceCurrent() < m_position.PriceOpen())
               {
                     double ASK = m_position.PriceCurrent();
                     if(ASK - TP < 50 * _Point)
                     {
                        
                        CTrade trade;
                        trade.PositionModify(m_position.Ticket(),ASK + 70 * _Point,ASK-70*_Point);
                     }
               
               }
                      
                }

                }
            }
 }


void CloseAllExpiredOrders()
{
        // Get the current time
    datetime currentTime = TimeCurrent();

    // Loop through all open positions
    for (int i = 0; i < PositionsTotal() ; i++)
    {
        // Select the position by index
        if (m_position.SelectByIndex(i))
        {
            // Check if the position is a buy or sell
            if (m_position.PositionType() == POSITION_TYPE_BUY || m_position.PositionType() == POSITION_TYPE_SELL)
            {
                // Calculate the duration the position has been open
                datetime positionOpenTime = m_position.Time(); // Get the position open time
                double durationMinutes = (currentTime - positionOpenTime) / 60;
                double profit = m_position.Profit();
                // Close the position if it's been open for more than 10 minutes
              if (durationMinutes > 15 && profit > 2)
              {
                
                if(m_position.StopLoss() < m_position.PriceOpen())
                {
                  if(m_position.PriceOpen() < m_position.PriceCurrent() + 50 * _Point )
                     m_trade.PositionModify(m_position.Ticket(),m_position.PriceOpen(),m_position.PriceCurrent() + 50 * _Point);
                }

                }
            }
            
           }
        }
    }



void SupportOpenedOrders()
{
   int totalPositions =PositionsTotal();
    for (int i = 0; i <  totalPositions ; i++)
    {
        // Select the position by index
        if (m_position.SelectByIndex(i))
        {
            double opentest = m_position.PriceOpen();
            ENUM_POSITION_TYPE type = m_position.PositionType() ;
            if(type == POSITION_TYPE_BUY && m_position.PriceCurrent() < m_position.PriceOpen() - 200 * _Point && IsOrderSpportable(i))
            {
                int orderLevel = SupportOrderLevel(i);
               double orderLot = 0.02 + (orderLevel * 0.01);
               double pipsDiff = (200.0 +(50.0 * orderLevel) ) * _Point;
               m_position.SelectByIndex(i);
               double currPrice = m_position.PriceCurrent();
               double openPrice = m_position.PriceOpen() ;
               if(m_position.PriceCurrent() < m_position.PriceOpen() - pipsDiff)
                    m_trade.Buy(orderLot,"XAUUSD",0,m_position.PriceCurrent() - 350.0 * _Point,m_position.PriceCurrent() + 150.0 * _Point,IntegerToString(m_position.Ticket()));
               
            }
            else if(m_position.PositionType() == POSITION_TYPE_SELL && m_position.PriceCurrent() > m_position.PriceOpen() + 200.0 * _Point && IsOrderSpportable(i))
            {
               int orderLevel = SupportOrderLevel(i);
               double orderLot = NormalizeDouble(0.6 + (orderLevel * 0.1),2);
               double pipsDiff = (200.0 +(50.0 * orderLevel) ) * _Point;
               m_position.SelectByIndex(i);
               double currPrice = m_position.PriceCurrent();
               double openPrice = m_position.PriceOpen() ;
               bool ress;
               if( currPrice> openPrice + pipsDiff )
                    ress=m_trade.Sell(orderLot,"XAUUSD",0,m_position.PriceCurrent() + 350.0 * _Point,m_position.PriceCurrent() - 150.0 * _Point,IntegerToString(m_position.Ticket()));
            int tt=0;
            }
            
        }
    }
}
int SupportOrderLevel(int orderIndex)
{
    CPositionInfo position;
    position.SelectByIndex(orderIndex);
    int tckt = position.Ticket();
    int maxSupportOrders = 4;
    int ordersCounter=0;
    for (int i = 0; i < PositionsTotal() ; i++)
    {
            if (position.SelectByIndex(i))
        {
            string comment = position.Comment();
            
            if(comment == IntegerToString(tckt))
               {
                 ordersCounter++;
                 if(ordersCounter > maxSupportOrders)
                     return -1;
               
               }  
        }

    }
    return ordersCounter;
}
bool IsOrderSpportable(int orderIndex)
{

    CPositionInfo position;
    position.SelectByIndex(orderIndex);
    int tckt = position.Ticket();
    if(position.Comment().Length() > 0)
      return false;
    int maxSupportOrders = 4;
    int ordersCounter=0;
    for (int i = 0; i < PositionsTotal() ; i++)
    {
        // Select the position by index
        if (position.SelectByIndex(i))
        {
            string comment = position.Comment();
            
            if(comment == IntegerToString(tckt))
               {
               ordersCounter++;
               if(ordersCounter >= maxSupportOrders)
                  return false;
               
               }  
        }
    }
    return true;

}
