#pragma once
#include "KernelEngine_ScoreCalcs_General.h"

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

//__global__ void TiledScan_Kernel_MA(const float* __restrict__ d_histVal,
//	const float* __restrict__ d_histSlope,
//	const float* __restrict__ d_currVal,
//	const float* __restrict__ d_currSlope,
//	float* __restrict__ d_scores, float* __restrict__ d_prevIdx0Score,
//	const int K, const int outLen,
//	const float weight, const int TILE);
//
//__global__ void SlideScan_Kernel_MA(
//	const float* __restrict__ d_histVal,
//	const float* __restrict__ d_histSlope,
//	const float* __restrict__ d_currVal,
//	const float* __restrict__ d_currSlope,
//	float* __restrict__ d_outResNew, // S[t]
//	const float* __restrict__ d_outResOld, // S[t] for prev t (Res1)
//	float* __restrict__ d_newPrevIdx0Score, // pair_score(H[t], C[0]) for current t (Res2)
//	const float* __restrict__ d_oldPrevIdx0Score, // pair_score(H[t], C[0]) for prev t (Res1)
//	int K, int outLen, float weight);