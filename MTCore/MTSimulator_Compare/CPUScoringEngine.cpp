#include "CPUScoringEngine.h"
#include "CPUEngine_ScoreCalcs.h"

void CPUScoringEngine::Scan_SingleIndicator(float* d_results, const float* d_histValues, const float* d_histSlopes, const float* d_hist_MinValues, const float* d_hist_MaxValues, const float* d_hist_MinSlopes, const float* d_hist_MaxSlopes, const float* d_currValues, const float* d_currSlopes, const float* d_curr_MinValues, const float* d_curr_MaxValues, const float* d_curr_MinSlopes, const float* d_curr_MaxSlopes, int currTimeLineLen, int numOfResults, IndicatorType indT, int weight, const int* inputIndices, int inputIndicesCount)
{
	if(!inputIndices || inputIndicesCount <= 0)
	{
		FullScan_SingleIndicator(d_results, d_histValues, d_histSlopes, d_hist_MinValues, d_hist_MaxValues, d_hist_MinSlopes, d_hist_MaxSlopes, d_currValues, d_currSlopes, d_curr_MinValues, d_curr_MaxValues, d_curr_MinSlopes, d_curr_MaxSlopes, currTimeLineLen, numOfResults, indT, weight);
	}
	else
	{
		CandidateScan_SingleIndicator(d_results, d_histValues, d_histSlopes, d_hist_MinValues, d_hist_MaxValues, d_hist_MinSlopes, d_hist_MaxSlopes, d_currValues, d_currSlopes, d_curr_MinValues, d_curr_MaxValues, d_curr_MinSlopes, d_curr_MaxSlopes, currTimeLineLen, numOfResults, indT, weight, inputIndices, inputIndicesCount);
	}

}

void CPUScoringEngine::Scan_DualIndicatorsRatio(float* d_results, const float* d_histIndX_Values, const float* d_histIndX_Slopes, const float* d_histIndX_MinValues, const float* d_histIndX_MaxValues, const float* d_histIndX_MinSlopes, const float* d_histIndX_MaxSlopes, const float* d_currIndX_Values, const float* d_currIndX_Slopes, const float* d_currIndX_MinValues, const float* d_currIndX_MaxValues, const float* d_currIndX_MinSlopes, const float* d_currIndX_MaxSlopes, const float* d_histIndY_Values, const float* d_histIndY_Slopes, const float* d_histIndY_MinValues, const float* d_histIndY_MaxValues, const float* d_histIndY_MinSlopes, const float* d_histIndY_MaxSlopes, const float* d_currIndY_Values, const float* d_currIndY_Slopes, const float* d_currIndY_MinValues, const float* d_currIndY_MaxValues, const float* d_currIndY_MinSlopes, const float* d_currIndY_MaxSlopes, int currTimeLineLen, int numOfResults, int weight, const int* inputIndices, const int inputIndicesCount)
{
	if(!inputIndices || inputIndicesCount <= 0)
	{
		TwoIndCalc_CPU(
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
			d_results, currTimeLineLen, numOfResults, weight);
	}
	else
	{
		CandidateTwoIndCalc_CPU(
			inputIndices, inputIndicesCount,
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
			d_results, currTimeLineLen, numOfResults, weight);
	}
}

void CPUScoringEngine::CandidateScan_SingleIndicator(float* results, const float* histValues, const float* histSlopes, const float* hist_MinValues, const float* hist_MaxValues, const float* hist_MinSlopes, const float* hist_MaxSlopes, const float* currValues, const float* currSlopes, const float* curr_MinValues, const float* curr_MaxValues, const float* curr_MinSlopes, const float* curr_MaxSlopes, int currTimeLineLen, int numOfResults, IndicatorType indT, int weight, const int* candidateIndices, int candidateCount)
{
	if(!results || numOfResults <= 0 || !candidateIndices || candidateCount <= 0)
	{
		std::cout << "Invalid input to CPUScoringEngine::CandidateScan_SingleIndicator" << std::endl;
		return;
	}
	if(indT == IndicatorType::EMA || indT == IndicatorType::ZLSMA)
	{
		CandidateCalcMA_CPU(candidateIndices, candidateCount,
			histValues, histSlopes,
			hist_MinValues, hist_MaxValues,
			hist_MinSlopes, hist_MaxSlopes,
			currValues, currSlopes,
			curr_MinValues, curr_MaxValues,
			curr_MinSlopes, curr_MaxSlopes,
			results, currTimeLineLen, numOfResults, weight);
	}
	else if(indT == IndicatorType::RSI)
	{
		CandidateCalcRSI_CPU(candidateIndices, candidateCount,
			histValues, histSlopes,
			currValues, currSlopes,
			results, currTimeLineLen, numOfResults, weight);
		
	}
}

void CPUScoringEngine::FullScan_SingleIndicator(float* results, const float* histValues, const float* histSlopes, const float* hist_MinValues, const float* hist_MaxValues, const float* hist_MinSlopes, const float* hist_MaxSlopes, const float* currValues, const float* currSlopes, const float* curr_MinValues, const float* curr_MaxValues, const float* curr_MinSlopes, const float* curr_MaxSlopes, int currTimeLineLen, int numOfResults, IndicatorType indT, int weight)
{
	if(!results || numOfResults <= 0)
	{
		std::cout << "Invalid input to CPUScoringEngine::FullScan_SingleIndicator" << std::endl;
		return;
	}
	if(indT == IndicatorType::EMA || indT == IndicatorType::ZLSMA)
	{
		calcMA_CPU(histValues, histSlopes, hist_MinValues, hist_MaxValues, hist_MinSlopes, hist_MaxSlopes
				, currValues, currSlopes, curr_MinValues, curr_MaxValues, curr_MinSlopes, curr_MaxSlopes
				, results, currTimeLineLen, numOfResults + currTimeLineLen, weight);
	}
	else if(indT == IndicatorType::RSI)
	{
		calcRSI_CPU(histValues, histSlopes, currValues, currSlopes, results, currTimeLineLen, numOfResults + currTimeLineLen, weight);
	}
}
