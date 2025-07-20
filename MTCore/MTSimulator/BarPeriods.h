#include "INDICATORSTRUCTRES.H"

#pragma once

class BarPeriod_Utils
{
public:
	static double Extract_Value(BaseIndicator* ind, BarPeriod barP,IndicatorValueType indVT = IndicatorValueType::Value)
	{
		if (!ind || !(ind->Get_IndicatorValues_Pointer()))
			return 0;

		switch (barP)
		{
		case BarPeriod::M1:
			return indVT == IndicatorValueType::Value ? ind->Get_IndicatorValues_Pointer()->M1 : ind->Get_IndicatorValues_Pointer()->m_Slope_M1;
		case BarPeriod::M5:
			return indVT == IndicatorValueType::Value ? ind->Get_IndicatorValues_Pointer()->M5 : ind->Get_IndicatorValues_Pointer()->m_Slope_M5;
		case BarPeriod::M15:
			return indVT == IndicatorValueType::Value ? ind->Get_IndicatorValues_Pointer()->M15 : ind->Get_IndicatorValues_Pointer()->m_Slope_M15;
		case BarPeriod::M30:
			return indVT == IndicatorValueType::Value ? ind->Get_IndicatorValues_Pointer()->M30 : ind->Get_IndicatorValues_Pointer()->m_Slope_M30;
		case BarPeriod::H1:
			return indVT == IndicatorValueType::Value ? ind->Get_IndicatorValues_Pointer()->H1 : ind->Get_IndicatorValues_Pointer()->m_Slope_H1;
		case BarPeriod::H4:
			return indVT == IndicatorValueType::Value ? ind->Get_IndicatorValues_Pointer()->H4 : ind->Get_IndicatorValues_Pointer()->m_Slope_H4;
		case BarPeriod::NONE:
			return 0;
		default:
			return 0;
		}
	}
};
