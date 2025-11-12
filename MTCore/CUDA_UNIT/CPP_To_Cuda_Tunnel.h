#pragma once
#include "MTStreamsStructs.h"

class CPP_To_Cuda_Tunnel
{
public:
	////////////////////////////////////////////////////////////////////////////////////////
	static void Calc_TwoIndicatorsRatio_ScoreCalcs(float* h_outScores, int numOfResults, IndicatorType indT, IndicatorLen indL1, BarPeriod barP1, IndicatorLen indL2, BarPeriod barP2, int weight, bool fullScan = false);
	static void Calc_Indicator_ScoreCalcs(float* h_outScores, int numOfResults ,IndicatorType indT, IndicatorLen indL, BarPeriod barP, int weight, bool fullScan = false);
	//////////////////// Setters ///////////////////////////////////////////////////////////
	static void Set_HistDataTimeFreameStream(float* h_data, size_t h_dataSize, IndicatorType indT, IndicatorLen indL, BarPeriod barP, IndicatorValueType valT, bool forceOverWrite = false);
	static void Set_CurrDataTimeFreameStream(float* h_data, size_t h_dataSize, IndicatorType indT, IndicatorLen indL, BarPeriod barP, IndicatorValueType valT, bool forceOverWrite = false);
	
	static void Set_TotalScore_D_Memory(size_t size);
	static void Reset_TotalScores_D_To0();
	/////////////////////////////////////////////////////////////////////////////////////////
	static void Get_ResultsFromDevice(float* h_outScores, int numOfResults);
	static bool IsCudaAvailable();
	static void SyncCUDADevice();
};

