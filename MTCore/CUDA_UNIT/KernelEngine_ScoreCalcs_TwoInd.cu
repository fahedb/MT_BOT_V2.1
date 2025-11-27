#include "KernelEngine_ScoreCalcs_TwoInd.h"

__device__ __forceinline__ float SingleCalc_Device_TwoInd(
	const float histXv,
	const float histXs,
	const float currXv,
	const float currXs,

	const float histYv,
	const float histYs,
	const float currYv,
	const float currYs,

	const float currX_low,
	const float currY_low,
	const float invCurrX,
	const float invCurrY,

	const float histX_low,
	const float histY_low,
	const float invHistX,
	const float invHistY)
{

	/////////////////////////////////////////////////////////////////////////////////
	// Calc normalized diff between  LenX and  LenY for hist vs curr ///////////
	// //////////////////////////////////////////////////////////////////////////////
	// Normalize LenX values
	
	const float currValDiff = currXv * invCurrX - currYv * invCurrY - (currX_low * invCurrX - currY_low * invCurrY);

	const float histValDiff = histXv * invHistX - histYv * invHistY - (histX_low * invHistX - histY_low * invHistY);

	float score = 0;

	score += RelativeScoreCalc(histValDiff, currValDiff); // compare Ind value differences
	///////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////
	// Calc slope diff between LenX and LenY for hist and curr ///////////
	////////////////////////////////////////////////////////////////////////////////
	const float currSlopeDiff = currXs - currYs;
	const float histSlopeDiff = histXs - histYs;
	score += RelativeScoreCalc(histSlopeDiff, currSlopeDiff); // compare Ind slope differences
	//////////////////////////////////////////////////////////////////////////////
	return score;
}


__device__ void FullPatternScanCalc_Device_TwoInd(const float* __restrict__ d_histLenX_values_shared /*array for  Ind1 hist values */
	, const float* __restrict__ d_histLenX_slopes_shared /*array for  Ind1 hist slopes */
	, const float* __restrict__ currLenX_values_shared /*from shared memory*/
	, const float* __restrict__ currLenX_slopes_shared /*from shared memory*/
	, const float* __restrict__ d_histLenY_values_shared /*array for  Ind2 hist values */
	, const float* __restrict__ d_histLenY_slopes_shared /*array for  Ind2 hist slopes */
	, const float* __restrict__ currLenY_values_shared /*from shared memory*/
	, const float* __restrict__ currLenY_slopes_shared /*from shared memory*/
	, const int histBaseIdx /*base index in hist arrays*/
	
	, const float* __restrict__ currLow_HighValues_LenX_shared /*from shared memory*/
	, const float* __restrict__ currLow_HighValues_LenY_shared /*from shared memory*/

	, const float* __restrict__ histLow_HighValues_LenX /*from constant memory*/
	, const float* __restrict__ histLow_HighValues_LenY /*from constant memory*/
	
	, float* __restrict__ d_scores /*output scores, where Score(idx) is the score of currMAs(t)&&histMAs(idx + t), t is the index of the current candle */
	, const int currPatternLen, const int outLen, int idx , const float weight)
{
	// Normalize values to [0, 1] range based on the lowest and highest values for MA LenX
	const float currRange_LenX = currLow_HighValues_LenX_shared[1] - currLow_HighValues_LenX_shared[0];
	const float histRange_LenX = histLow_HighValues_LenX[1] - histLow_HighValues_LenX[0];

	const float currLowest_LenX = currLow_HighValues_LenX_shared[0]; // Get from constant memory
	const float histLowest_LenX = histLow_HighValues_LenX[0]; // Get from constant memory

	// Normalize values to [0, 1] range based on the lowest and highest values for MA LenY
	const float currRange_LenY = currLow_HighValues_LenY_shared[1] - currLow_HighValues_LenY_shared[0];
	const float histRange_LenY = histLow_HighValues_LenY[1] - histLow_HighValues_LenY[0];

	const float currLowest_LenY = currLow_HighValues_LenY_shared[0]; // Get from constant memory
	const float histLowest_LenY = histLow_HighValues_LenY[0]; // Get from constant memory

	const float eps = 1e-8f;
	const float invCurrRange_LenX = 1.0f / fmaxf(currRange_LenX, eps);
	const float invHistRange_LenX = 1.0f / fmaxf(histRange_LenX, eps);

	const float invCurrRange_LenY = 1.0f / fmaxf(currRange_LenY, eps);
	const float invHistRange_LenY = 1.0f / fmaxf(histRange_LenY, eps);

	float score = 0.0f;

#pragma unroll 64 // assuming max pattern length is 30-64, adjust as needed
	for (int i = 0; i < currPatternLen; i++)
	{
		// Len X inits
		const float histValue_LenX = d_histLenX_values_shared[histBaseIdx + i];
		const float histSlope_LenX = d_histLenX_slopes_shared[histBaseIdx + i];
		const float currValue_LenX = currLenX_values_shared[i]; // from shared memory
		const float currSlope_LenX = currLenX_slopes_shared[i]; // from shared memory

		/////////////////////////////////////////////////////////////////////////////////
		// len Y inits
		const float histValue_LenY = d_histLenY_values_shared[histBaseIdx + i];
		const float histSlope_LenY = d_histLenY_slopes_shared[histBaseIdx + i];
		const float currValue_LenY = currLenY_values_shared[i]; // from shared memory
		const float currSlope_LenY = currLenY_slopes_shared[i]; // from shared memory

		score +=
			SingleCalc_Device_TwoInd(
				histValue_LenX,
				histSlope_LenX,
				currValue_LenX,
				currSlope_LenX,
				histValue_LenY,
				histSlope_LenY,
				currValue_LenY,
				currSlope_LenY,
				currLowest_LenX,
				currLowest_LenY,
				invCurrRange_LenX,
				invCurrRange_LenY,
				histLowest_LenX,
				histLowest_LenY,
				invHistRange_LenX,
				invHistRange_LenY
			);
	}

	d_scores[idx] += score * weight; // final score for this position
}
// Full Scan two indicators kernel
// Ind score calcs (for 2 Inds different lengths, e.g., 50 and 200)
// Theory -- we compare the difference between the two Inds for hist and curr, both for value and slope
// e.g., if for curr, Ind1 - Ind2 = 5, and for hist, Ind1 - Ind2 = 5.5, then they are ~similar
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
	, const int currPatternLen, const int outLen, const float weight)
{
	extern __shared__ float sharedMem[]; // dynamically allocated shared memory
	float* currLenX_values_shared = sharedMem; // size: currPatternLen
	float* currLenX_slopes_shared = &sharedMem[currPatternLen]; // size: currPatternLen
	float* currLenY_values_shared = &sharedMem[2 * currPatternLen]; // size: currPatternLen
	float* currLenY_slopes_shared = &sharedMem[3 * currPatternLen]; // size: currPatternLen
	float* currLow_HighValues_LenX_shared = &sharedMem[4 * currPatternLen]; // size: 2
	float* currLow_HighValues_LenY_shared = &sharedMem[(4 * currPatternLen) + 2]; // size: 2
	
	const int baseHistShared = 4 * currPatternLen + 4;
	int histTileLen = blockDim.x + currPatternLen - 1;

	float* histLenX_values_shared = &sharedMem[baseHistShared]; // size: blockDim.x + currPatternLen - 1
	float* histLenX_slopes_shared = &sharedMem[baseHistShared + histTileLen]; // size: blockDim.x + currPatternLen - 1
	float* histLenY_values_shared = &sharedMem[baseHistShared + 2 * histTileLen]; // size: blockDim.x + currPatternLen - 1
	float* histLenY_slopes_shared = &sharedMem[baseHistShared + 3 * histTileLen]; // size: blockDim.x + currPatternLen - 1

	for(int i = threadIdx.x; i < histTileLen; i += blockDim.x)
	{
		int histIdx = blockIdx.x * blockDim.x + i;
		histLenX_values_shared[i] = d_histLenX_values[histIdx];
		histLenX_slopes_shared[i] = d_histLenX_slopes[histIdx];
		histLenY_values_shared[i] = d_histLenY_values[histIdx];
		histLenY_slopes_shared[i] = d_histLenY_slopes[histIdx];
	}

	// Load currMAs_values and currMAs_slopes into shared memory
	for (int i = threadIdx.x; i < currPatternLen; i += blockDim.x) {
		currLenX_values_shared[i] = d_currLenX_values[i];
		currLenX_slopes_shared[i] = d_currLenX_slopes[i];
		currLenY_values_shared[i] = d_currLenY_values[i];
		currLenY_slopes_shared[i] = d_currLenY_slopes[i];
	}

	__syncthreads(); // Ensure all threads have loaded the data -- NOTE: you'll get intelisense error (undefined), but it compiles and works fine


	if (threadIdx.x == 0)
	{
		currLow_HighValues_LenX_shared[0] = d_currLenX_MinValues[0];
		currLow_HighValues_LenX_shared[1] = d_currLenX_MaxValues[0];

		currLow_HighValues_LenY_shared[0] = d_currLenY_MinValues[0];
		currLow_HighValues_LenY_shared[1] = d_currLenY_MaxValues[0];
	}
	__syncthreads(); // Ensure all threads have loaded the data -- NOTE: you'll get intelisense error (undefined), but it compiles and works fine


	int idx = blockIdx.x * blockDim.x + threadIdx.x;

	if (idx >= outLen) return; // out of bounds check

	float histLow_HighValues_LenX[2];
	histLow_HighValues_LenX[0] = d_histLenX_MinValues[idx];
	histLow_HighValues_LenX[1] = d_histLenX_MaxValues[idx];

	float histLow_HighValues_LenY[2];
	histLow_HighValues_LenY[0] = d_histLenY_MinValues[idx];
	histLow_HighValues_LenY[1] = d_histLenY_MaxValues[idx];

	int histBaseIdx = threadIdx.x; // each thread processes its own starting index in the shared hist arrays
	FullPatternScanCalc_Device_TwoInd(
		histLenX_values_shared,
		histLenX_slopes_shared,
		currLenX_values_shared,
		currLenX_slopes_shared,

		histLenY_values_shared,
		histLenY_slopes_shared,
		currLenY_values_shared,
		currLenY_slopes_shared,
		histBaseIdx,
		currLow_HighValues_LenX_shared,
		currLow_HighValues_LenY_shared,
		histLow_HighValues_LenX,
		histLow_HighValues_LenY,

		d_scores,
		currPatternLen,
		outLen,
		idx,
		weight);

}


__global__ void CandidateScan_Kernel_TwoInd(
	const int* __restrict__ d_candidateIndices /*array of candidate indices to process */
	, int candidateCount
	/////////////////////////////////// Hist len X arrays //////////////////////////////////////////////
	, const float* __restrict__ d_histLenX_values /*array for  Ind1 hist values */					////
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
	, const int currPatternLen, const int outLen, const float weight)
{
	extern __shared__ float sharedMem[]; // dynamically allocated shared memory
	float* currLenX_values_shared = sharedMem; // size: currPatternLen
	float* currLenX_slopes_shared = &sharedMem[currPatternLen]; // size: currPatternLen
	float* currLenY_values_shared = &sharedMem[2 * currPatternLen]; // size: currPatternLen
	float* currLenY_slopes_shared = &sharedMem[3 * currPatternLen]; // size: currPatternLen
	float* currLow_HighValues_LenX_shared = &sharedMem[4 * currPatternLen]; // size: 2
	float* currLow_HighValues_LenY_shared = &sharedMem[(4 * currPatternLen) + 2]; // size: 2

	if (threadIdx.x == 0)
	{
		currLow_HighValues_LenX_shared[0] = d_currLenX_MinValues[0];
		currLow_HighValues_LenX_shared[1] = d_currLenX_MaxValues[0];

		currLow_HighValues_LenY_shared[0] = d_currLenY_MinValues[0];
		currLow_HighValues_LenY_shared[1] = d_currLenY_MaxValues[0];
	}

	// Load currMAs_values and currMAs_slopes into shared memory
	for (int i = threadIdx.x; i < currPatternLen; i += blockDim.x) {
		currLenX_values_shared[i] = d_currLenX_values[i];
		currLenX_slopes_shared[i] = d_currLenX_slopes[i];
		currLenY_values_shared[i] = d_currLenY_values[i];
		currLenY_slopes_shared[i] = d_currLenY_slopes[i];
	}

	__syncthreads(); // Ensure all threads have loaded the data -- NOTE: you'll get intelisense error (undefined), but it compiles and works fine

	int cid = blockIdx.x * blockDim.x + threadIdx.x;
	if (cid >= candidateCount) return; // out of bounds check
	int workIdx = d_candidateIndices[cid];
	if (workIdx >= outLen) return; // out of bounds check

	float histLow_HighValues_LenX[2];
	histLow_HighValues_LenX[0] = d_histLenX_MinValues[workIdx];
	histLow_HighValues_LenX[1] = d_histLenX_MaxValues[workIdx];

	float histLow_HighValues_LenY[2];
	histLow_HighValues_LenY[0] = d_histLenY_MinValues[workIdx];
	histLow_HighValues_LenY[1] = d_histLenY_MaxValues[workIdx];

	
	FullPatternScanCalc_Device_TwoInd(
		d_histLenX_values,
		d_histLenX_slopes,
		currLenX_values_shared,
		currLenX_slopes_shared,
		d_histLenY_values,
		d_histLenY_slopes,
		currLenY_values_shared,
		currLenY_slopes_shared,
		workIdx,
		currLow_HighValues_LenX_shared,
		currLow_HighValues_LenY_shared,
		histLow_HighValues_LenX,
		histLow_HighValues_LenY,
		d_scores,
		currPatternLen,
		outLen,
		workIdx,
		weight);



}