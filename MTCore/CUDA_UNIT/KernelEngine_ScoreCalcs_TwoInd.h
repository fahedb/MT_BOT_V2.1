#pragma once
#include "KernelEngine_ScoreCalcs_General.h"
// Full scan kernel for Two Indicators calculation
__global__ void FullScan_Kernel_TwoInd(
	/////////////////////////////////// Hist len X arrays //////////////////////////////////////////////
	  const float* __restrict__ d_histLenX_values /*array for  Ind1 hist values */					////
	, const float* __restrict__ d_histLenX_slopes /*array for  Ind1 hist slopes */					////
																									////
	, const float* __restrict__ d_histLenX_MinValues /*array for  Ind1 hist min values */			////
	, const float* __restrict__ d_histLenX_MaxValues /*array for  Ind1 hist max values */			////
	, const float* __restrict__ d_histLenX_MinSlopes /*array for  Ind1 hist min slopes */			////
	, const float* __restrict__ d_histLenX_MaxSlopes /*array for  Ind1 hist max slopes */			////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/////////////////////////////////// Current len X arrays ///////////////////////////////////////////
	, const float* __restrict__ d_currLenX_values /*array for Ind1 curr values*/					////
	, const float* __restrict__ d_currLenX_slopes /* array for Ind1 curr slopes*/					////
																									////
																									////
	, const float* __restrict__ d_currLenX_MinValues /* array for Ind1 curr min values*/			////
	, const float* __restrict__ d_currLenX_MaxValues /* array for Ind1 curr max values*/			////
	, const float* __restrict__ d_currLenX_MinSlopes /* array for Ind1 curr min slopes*/			////
	, const float* __restrict__ d_currLenX_MaxSlopes /* array for Ind1 curr max slopes*/			////
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/////////////////////////////////// Hist len Y arrays //////////////////////////////////////////////
	, const float* __restrict__ d_histLenY_values /*array for  Ind2 hist values */					////
	, const float* __restrict__ d_histLenY_slopes /*array for  Ind2 hist slopes */					////
																									////
	, const float* __restrict__ d_histLenY_MinValues /*array for  Ind2 hist min values */			////
	, const float* __restrict__ d_histLenY_MaxValues /*array for  Ind2 hist max values */			////
	, const float* __restrict__ d_histLenY_MinSlopes /*array for  Ind2 hist min slopes */			////
	, const float* __restrict__ d_histLenY_MaxSlopes /*array for  Ind2 hist max slopes */			////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	 
	/////////////////////////////////// Current len Y arrays ///////////////////////////////////////////
	, const float* __restrict__ d_currLenY_values /* array for Ind2 curr values*/					////
	, const float* __restrict__ d_currLenY_slopes /* array for Ind2 curr slopes*/					////
																									////
	, const float* __restrict__ d_currLenY_MinValues /* array for Ind2 curr min values*/			////
	, const float* __restrict__ d_currLenY_MaxValues /* array for Ind2 curr max values*/			////
	, const float* __restrict__ d_currLenY_MinSlopes /* array for Ind2 curr min slopes*/			////
	, const float* __restrict__ d_currLenY_MaxSlopes /* array for Ind2 curr max slopes*/			////
	////////////////////////////////////////////////////////////////////////////////////////////////////

	, float* __restrict__ d_scores /*output scores, where Score(idx) is the score of currMAs(t)&&histMAs(idx + t), t is the index of the current candle */
	, const int currPatternLen, const int outLen, const float weight);


//// Tiled scan kernel for Two Indicators calculation
//__global__ void TiledScan_Kernel_TwoInd(
//	const float* __restrict__ d_histX_val, /*array for ind1 hist vals*/
//	const float* __restrict__ d_histX_slp, /*array for ind1 hist slopes*/
//	const float* __restrict__ d_currX_val, /*array for ind1 curr vals*/
//	const float* __restrict__ d_currX_slp, /*array for ind1 curr slopes*/
//	const float* __restrict__ d_histY_val, /*array for ind2 hist vals*/
//	const float* __restrict__ d_histY_slp, /*array for ind2 hist slopes*/
//	const float* __restrict__ d_currY_val, /*array for ind2 curr vals*/
//	const float* __restrict__ d_currY_slp, /*array for ind2 curr slopes*/
//	float* __restrict__ d_scores, // output scores
//	const int K, const int outLen, const float weight, const int TILE);
//

//__global__ void SlideCalc_Kernel_TwoInd(
//	const float* __restrict__ d_histLenX_values /*array for  Ind1 hist values */
//	, const float* __restrict__ d_histLenX_slopes /*array for  Ind1 hist slopes */
//	, const float* __restrict__ d_currLenX_values /* array for Ind1 curr values*/
//	, const float* __restrict__ d_currLenX_slopes /* array for Ind1 curr slopes*/
//	, const float* __restrict__ d_histLenY_values /*array for  Ind2 hist values */
//	, const float* __restrict__ d_histLenY_slopes /*array for  Ind2 hist slopes */
//	, const float* __restrict__ d_currLenY_values /* array for Ind2 curr values*/
//	, const float* __restrict__ d_currLenY_slopes /* array for Ind2 curr slopes*/
//	, float* __restrict__ d_outResNew // Res2
//	, const float* __restrict__ d_outResOld // Res1
//	, float* __restrict__ d_newPrevIdx0Score // pair_score(H[t], C[0]) for new t (Res2) (to be filled for next iteration)
//	, const float* __restrict__ d_oldPrevIdx0Score // pair_score(H[t], C[0]) for prev t (Res1)
//	, int K , int outLen, float weight);
//	