#pragma once
#include "MTStreamsStructs.h"

class CPP_To_Cuda_Tunnel
{
public:
	////////////////////////////////////////////////////////////////////////////////////////
	static void Calc_TwoIndicatorsRatio_ScoreCalcs(float* d_results

		///////////////////// Hist len X arrays ////////////////////////////////////////////
		, const float* d_histIndX_Values, const float* d_histIndX_Slopes				////
		, const float* d_histIndX_MinValues, const float* d_histIndX_MaxValues			////
		, const float* d_histIndX_MinSlopes, const float* d_histIndX_MaxSlopes			////
		////////////////////////////////////////////////////////////////////////////////////

		/////////////////////// Current len X arrays //////////////////////////////////////
		, const float* d_currIndX_Values, const float* d_currIndX_Slopes				///
		, const float* d_currIndX_MinValues, const float* d_currIndX_MaxValues			///
		, const float* d_currIndX_MinSlopes, const float* d_currIndX_MaxSlopes			///
		///////////////////////////////////////////////////////////////////////////////////

		////////////////////// Hist len Y arrays //////////////////////////////////////////
		, const float* d_histIndY_Values, const float* d_histIndY_Slopes				///				
		, const float* d_histIndY_MinValues, const float* d_histIndY_MaxValues			///
		, const float* d_histIndY_MinSlopes, const float* d_histIndY_MaxSlopes			///
		///////////////////////////////////////////////////////////////////////////////////

		///////////////////// Current len Y arrays ////////////////////////////////////////
		, const float* d_currIndY_Values, const float* d_currIndY_Slopes				///			
		, const float* d_currIndY_MinValues, const float* d_currIndY_MaxValues			///
		, const float* d_currIndY_MinSlopes, const float* d_currIndY_MaxSlopes			///
		///////////////////////////////////////////////////////////////////////////////////

		, int currTimeLineLen, int numOfResults , int weight);
	

	static void Calc_Indicator_ScoreCalcs(float* d_results
		//////////////////////////////////// Hist arrays //////////////////////////////////////////////////
		, const float* d_histValues, const float* d_histSlopes											///
		, const float* d_hist_MinValues, const float* d_hist_MaxValues									///
		, const float* d_hist_MinSlopes, const float* d_hist_MaxSlopes									///
		///////////////////////////////////////////////////////////////////////////////////////////////////

		//////////////////////////////////// Current arrays ///////////////////////////////////////////////
		, const float* d_currValues, const float* d_currSlopes											///
		, const float* d_curr_MinValues, const float* d_curr_MaxValues									///
		, const float* d_curr_MinSlopes, const float* d_curr_MaxSlopes									///
		///////////////////////////////////////////////////////////////////////////////////////////////////
		, int currTimeLineLen, int numOfResults, IndicatorType indT, int weight);

	//////////////////// Utility Functions /////////////////////////////////////////////////
	static bool IsCudaAvailable();
	static void SyncCUDADevice();

};

