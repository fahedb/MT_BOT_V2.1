#include "ChangeData.h"



////////////////////////////////////////////////////
/////////// ChangeNode class Functions//////////
/////////// Author: Fahed.b				  //////////
////////////////////////////////////////////////////
ChangeNode::ChangeNode()
{
	point_DateTime = "";
	Increase_Decrease = false;
	m_PointPlace = _Points::_Points_DEFUALT;
	Increase_Decrease = false;
	activePoint = false;
	m_openPrice = INVALID_DATA;
	m_closePrice = INVALID_DATA;
	m_highPrice = INVALID_DATA;
	m_lowPrice = INVALID_DATA;
}

 ChangeNode::~ChangeNode()
 {
 }

 /*RSI_Ind* ChangeNode::Get_RSI()
 {
	 if (m_rsi)
		 return m_rsi;
	 return nullptr;

 }

 EMA_Ind* ChangeNode::Get_EMA()
 {
	 if (m_ema)
		 return m_ema;
	 return nullptr;
 }
 ZLSMA_Ind* ChangeNode::Get_ZLSMA()
 {
	 if (m_zlsma)
		 return m_zlsma;
	 return nullptr;

 }*/


 ////////////////////////////////////////////////////
/////////// END OF ChangeNode class Functions////
////////////////////////////////////////////////////