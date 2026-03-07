#pragma once
#include "CudaMemoryController.h"

__global__ void FullScan_Kernel_MA(
	////////////////////////////////// Hist MAs arrays ////////////////////////////////////////////////////
	const float* __restrict__ d_histMAs_values /*array for MAs hist values */							///
	, const float* __restrict__ d_histMAs_slopes /*array for MAs hist slopes */							///
																										///
	, const float* __restrict__ d_histMAs_MinValues, const float* __restrict__ d_histMAs_MaxValues		///
	, const float* __restrict__ d_histMAs_MinSlopes, const float* __restrict__ d_histMAs_MaxSlopes		///
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////// Current MAs arrays /////////////////////////////////////////////////
	, const float* __restrict__ d_currMAs_values /* array for MAs curr values*/							///
	, const float* __restrict__ d_currMAs_slopes /* array for MAs curr slopes*/							///			
																										///
	, const float* __restrict__ d_currMAs_MinValues, const float* __restrict__ d_currMAs_MaxValues		///
	, const float* __restrict__ d_currMAs_MinSlopes, const float* __restrict__ d_currMAs_MaxSlopes		///
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	, float* __restrict__ d_scores /*output scores, where Score(idx) is the score of currMAs(t)&&histMAs(idx + t), t is the index of the current candle */
	, const int currPatternLen, const int outLen, const float weight);

__global__ void CandidateScan_Kernel_MA(
	const int* __restrict__ d_candidateIndices /*array of candidate indices to process */
	, int candidateCount
	////////////////////////////////// Hist MAs arrays ////////////////////////////////////////////////////
	, const float* __restrict__ d_histMAs_values /*array for MAs hist values */							///
	, const float* __restrict__ d_histMAs_slopes /*array for MAs hist slopes */							///
	///
	, const float* __restrict__ d_histMAs_MinValues, const float* __restrict__ d_histMAs_MaxValues		///
	, const float* __restrict__ d_histMAs_MinSlopes, const float* __restrict__ d_histMAs_MaxSlopes		///
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////// Current MAs arrays /////////////////////////////////////////////////
	, const float* __restrict__ d_currMAs_values /* array for MAs curr values*/							///
	, const float* __restrict__ d_currMAs_slopes /* array for MAs curr slopes*/							///			
	///
	, const float* __restrict__ d_currMAs_MinValues, const float* __restrict__ d_currMAs_MaxValues		///
	, const float* __restrict__ d_currMAs_MinSlopes, const float* __restrict__ d_currMAs_MaxSlopes		///
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	, float* __restrict__ d_scores /*output scores, where Score(idx) is the score of currMAs(t)&&histMAs(idx + t), t is the index of the current candle */
	, const int currPatternLen, const int outLen, const float weight);