#include "ChangeData.h"



////////////////////////////////////////////////////
/////////// ChangeNode class Functions//////////
/////////// Author: Fahed.b				  //////////
////////////////////////////////////////////////////

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

 Gravity_Ind* ChangeNode::GetGravityInd()
 {
	 if (m_gravity)
		 return m_gravity;
	 return nullptr;
 }
 ////////////////////////////////////////////////////
/////////// END OF ChangeNode class Functions////
////////////////////////////////////////////////////