#include "GPUScoringEngine.h"
#include "CPP_To_Cuda_Tunnel.h"

void GPUScoringEngine::Scan_SingleIndicator(float* d_results, const float* d_histValues, const float* d_histSlopes, const float* d_hist_MinValues, const float* d_hist_MaxValues, const float* d_hist_MinSlopes, const float* d_hist_MaxSlopes, const float* d_currValues, const float* d_currSlopes, const float* d_curr_MinValues, const float* d_curr_MaxValues, const float* d_curr_MinSlopes, const float* d_curr_MaxSlopes, int currTimeLineLen, int numOfResults, IndicatorType indT, int weight, const int* inputIndices, const int inputIndicesCount)
{
	if (!inputIndices || inputIndicesCount <= 0)
	{
		CPP_To_Cuda_Tunnel::Calc_Indicator_ScoreCalcs(
			d_results,
			d_histValues, d_histSlopes,
			d_hist_MinValues, d_hist_MaxValues,
			d_hist_MinSlopes, d_hist_MaxSlopes,

			d_currValues, d_currSlopes,
			d_curr_MinValues, d_curr_MaxValues,
			d_curr_MinSlopes, d_curr_MaxSlopes,
			currTimeLineLen, numOfResults, indT, weight);
	}
	else
	{
		CPP_To_Cuda_Tunnel::Calc_CandidateIndicator_ScoreCalcs(
			inputIndices, inputIndicesCount,
			d_results,
			d_histValues, d_histSlopes,
			d_hist_MinValues, d_hist_MaxValues,
			d_hist_MinSlopes, d_hist_MaxSlopes,
			d_currValues, d_currSlopes,
			d_curr_MinValues, d_curr_MaxValues,
			d_curr_MinSlopes, d_curr_MaxSlopes,
			currTimeLineLen, numOfResults, indT, weight);
	}
}

void GPUScoringEngine::Scan_DualIndicatorsRatio(float* d_results, const float* d_histIndX_Values, const float* d_histIndX_Slopes, const float* d_histIndX_MinValues, const float* d_histIndX_MaxValues, const float* d_histIndX_MinSlopes, const float* d_histIndX_MaxSlopes, const float* d_currIndX_Values, const float* d_currIndX_Slopes, const float* d_currIndX_MinValues, const float* d_currIndX_MaxValues, const float* d_currIndX_MinSlopes, const float* d_currIndX_MaxSlopes, const float* d_histIndY_Values, const float* d_histIndY_Slopes, const float* d_histIndY_MinValues, const float* d_histIndY_MaxValues, const float* d_histIndY_MinSlopes, const float* d_histIndY_MaxSlopes, const float* d_currIndY_Values, const float* d_currIndY_Slopes, const float* d_currIndY_MinValues, const float* d_currIndY_MaxValues, const float* d_currIndY_MinSlopes, const float* d_currIndY_MaxSlopes, int currTimeLineLen, int numOfResults, int weight, const int* inputIndices, const int inputIndicesCount)
{
	if(!inputIndices || inputIndicesCount <= 0)
	{
		CPP_To_Cuda_Tunnel::Calc_TwoIndicatorsRatio_ScoreCalcs(
			d_results,
			d_histIndX_Values, d_histIndX_Slopes,
			d_histIndX_MinValues, d_histIndX_MaxValues,
			d_histIndX_MinSlopes, d_histIndX_MaxSlopes,
			d_currIndX_Values, d_currIndX_Slopes,
			d_currIndX_MinValues, d_currIndX_MaxValues,
			d_currIndX_MinSlopes, d_currIndX_MaxSlopes,
			d_histIndY_Values, d_histIndY_Slopes,
			d_histIndY_MinValues, d_histIndY_MaxValues,
			d_histIndY_MinSlopes, d_histIndY_MaxSlopes,
			d_currIndY_Values, d_currIndY_Slopes,
			d_currIndY_MinValues, d_currIndY_MaxValues,
			d_currIndY_MinSlopes, d_currIndY_MaxSlopes,
			currTimeLineLen, numOfResults, weight);
	}
	else {

		CPP_To_Cuda_Tunnel::Calc_CandidateTwoIndicatorRatio_ScoreCalcs(
			inputIndices, inputIndicesCount,
			d_results,
			d_histIndX_Values, d_histIndX_Slopes,
			d_histIndX_MinValues, d_histIndX_MaxValues,
			d_histIndX_MinSlopes, d_histIndX_MaxSlopes,
			d_currIndX_Values, d_currIndX_Slopes,
			d_currIndX_MinValues, d_currIndX_MaxValues,
			d_currIndX_MinSlopes, d_currIndX_MaxSlopes,
			d_histIndY_Values, d_histIndY_Slopes,
			d_histIndY_MinValues, d_histIndY_MaxValues,
			d_histIndY_MinSlopes, d_histIndY_MaxSlopes,
			d_currIndY_Values, d_currIndY_Slopes,
			d_currIndY_MinValues, d_currIndY_MaxValues,
			d_currIndY_MinSlopes, d_currIndY_MaxSlopes,
			currTimeLineLen, numOfResults, weight);
	}
}
