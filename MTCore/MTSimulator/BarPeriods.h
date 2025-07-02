#include "INDICATORSTRUCTRES.H"

#pragma once

class BarPeriod_Utils
{
public:
	static double Extract_Value(BaseIndicator* ind, BarPeriod barP)
	{
		switch (barP)
		{
		case BarPeriod::M1:
			return ind->Get_IndicatorValues_Pointer()->M1;
		case BarPeriod::M5:
			return ind->Get_IndicatorValues_Pointer()->M5;
		case BarPeriod::M15:
			return ind->Get_IndicatorValues_Pointer()->M15;
		case BarPeriod::M30:
			return ind->Get_IndicatorValues_Pointer()->M30;
		case BarPeriod::H1:
			return ind->Get_IndicatorValues_Pointer()->H1;
		case BarPeriod::H4:
			return ind->Get_IndicatorValues_Pointer()->H4;
		case BarPeriod::NONE:
			return 0;
		default:
			return 0;
		}
	}
};
