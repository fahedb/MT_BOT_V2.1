//+------------------------------------------------------------------+
//|                                                    MT_Policy.mqh |
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

#import "C:\\Users\\Administrator\\Documents\\GitHub\\MT_BOT_V2.1\\MTDll\\x64\Debug\\MTDll.dll"
// Dll Functions def
double Get_MainTrade_Size();
double Get_SupportTrade_Size();
int GetFutureVision();
int Get_TP_Start();
int Get_SL_Start();

int Get_TP_Diff();
int Get_SL_Diff();
#import

int m_futureVision;
double m_mainOrderSize;
double m_supportOrderSize;

int m_sl_diff;
int m_tp_diff;

void RefreshPolicy()
{
   m_mainOrderSize = Get_MainTrade_Size();
   m_supportOrderSize = Get_SupportTrade_Size();
   m_futureVision = GetFutureVision();
   m_sl_diff = Get_SL_Diff();
   m_tp_diff = Get_TP_Diff();
}


double Get_MainOrderSize_Policy()
{
   return m_mainOrderSize;
}


double Get_SupportOrderSize_Policy()
{
   return m_supportOrderSize;
}

int Get_FutureVisionLen_Policy()
{
   return m_futureVision;
}

int Get_MaxOrdersAllowed_Policy()
{
   return 4; // Need to implement a config for this
}

int Get_TP_Diff_Policy()
{
   return m_sl_diff;
}

int Get_SL_Diff_Policy()
{
   return m_tp_diff;
}