#pragma once
#include "CudaMemoryController.h"

// Full scan kernel for RSI calculation
__global__ void FullScan_Kernel_RSI(const float* __restrict__ d_histRSI_values /*array for rsi hist values */
	, const float* __restrict__ d_histRSI_slopes /*array for rsi hist slopes */
	, const float* __restrict__ d_currRSI_values /* array for rsi curr values*/
	, const float* __restrict__ d_currRSI_slopes /* array for rsi curr slopes*/
	, float* d_scores /*output scores, where Score(idx) is the score of currRSI(t)&&histRSI(idx + t), t is the index of the current candle */
	, const int currPatternLen, const int outLen, const float weight);

// Candidate scan kernel for RSI calculation, where only the candidate indices will be processed to calculate their scores
__global__ void CandidateScan_Kernel_RSI(
	const int* __restrict__ d_candidateIndices /*array of candidate indices to process */
	, int candidateCount
	, const float* __restrict__ d_histRSI_values /*array for rsi hist values */
	, const float* __restrict__ d_histRSI_slopes /*array for rsi hist slopes */
	, const float* __restrict__ d_currRSI_values /* array for rsi curr values*/
	, const float* __restrict__ d_currRSI_slopes /* array for rsi curr slopes*/
	, float* d_scores /*output scores, where Score(idx) is the score of currRSI(t)&&histRSI(idx + t), t is the index of the current candle */
	, const int currPatternLen, const int outLen, const float weight);