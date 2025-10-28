#pragma once
#include "KernelEngine_ScoreCalcs_General.h"

// Full scan kernel for RSI calculation
__global__ void FullScan_Kernel_RSI(const float* __restrict__ d_histRSI_values /*array for rsi hist values */
	, const float* __restrict__ d_histRSI_slopes /*array for rsi hist slopes */
	, const float* __restrict__ d_currRSI_values /* array for rsi curr values*/
	, const float* __restrict__ d_currRSI_slopes /* array for rsi curr slopes*/
	, float* d_scores /*output scores, where Score(idx) is the score of currRSI(t)&&histRSI(idx + t), t is the index of the current candle */
	, float* __restrict__ d_prevIdx0Score
	, const int currPatternLen, const int dataSize, const float weight);

// Full tiled scan kernel for RSI calculation
__global__ void TiledScan_Kernel_RSI(
	const float* __restrict__ d_histVal, // H[t + i]
	const float* __restrict__ d_histSlope, // H'[t + i]
	const float* __restrict__ d_currVal, // C[i]
	const float* __restrict__ d_currSlope, // C'[i]
	float* __restrict__ d_out, // S[t]
	float* __restrict__ d_prevIdx0Score, // pair_score(H[t], C[0])
	int starts, int K, float weight, int TILE);

// Sliding scan kernel for RSI calculation
// Need to use only when ScanNum > 0
__global__ void SlideScan_Kernel_RSI(
	const float* __restrict__ d_histVal,
	const float* __restrict__ d_histSlope,
	const float* __restrict__ d_currVal,
	const float* __restrict__ d_currSlope,
	float* __restrict__ d_outResNew, // S[t]
	float* __restrict__ d_outResOld, // S[t] for prev t (Res1)
	float* __restrict__ d_prevIdx0Score, // pair_score(H[t], C[0]) for prev t (Res1)
	int outLen, int K, float weight);