#include "KernelEngine_ScoreCalcs_MA.h"
#include "KernelEngine_ScoreCalcs_General.h"

__device__ __forceinline__ float CalcSingleMAScore(
	const float histValue,
	const float histSlope,
	const float currValue,
	const float currSlope,
	const float histLowest,
	const float currLowest,
	const float invHistRange,
	const float invCurrRange)
{
	// Normalize values to [0, 1] range based on the lowest and highest values
	const float normHistValue = (histValue - histLowest) * invHistRange;
	const float normCurrValue = (currValue - currLowest) * invCurrRange;

	float score = pair_score(normHistValue, normCurrValue, histSlope, currSlope);
	return score;
}

__device__ __forceinline__ void FullPatternScanCalc_Device_MA(
	const float* __restrict__ histMAs_values_shared /*array for MAs hist values */
	, const float* __restrict__ histMAs_slopes_shared /*array for MAs hist slopes */
	, const float* __restrict__ currMAs_values_shared /* array for MAs curr values in shared memory*/
	, const float* __restrict__ currMAs_slopes_shared /* array for MAs curr slopes in shared memory*/

	, const float* __restrict__ histLow_HightValues /* array for hist low/high values in shared memory*/
	, const float* __restrict__ currLow_HightValues_shared /* array for curr low/high values in shared memory*/

	, float* d_scores /*output scores, where Score(idx) is the score of currMAs(t)&&histMAs(idx + t), t is the index of the current candle */
	, const int currPatternLen, const float weight, const int idx)
{
	const float currRange = currLow_HightValues_shared[1] - currLow_HightValues_shared[0];
	const float histRange = histLow_HightValues[1] - histLow_HightValues[0];

	const float eps = 1e-6f;
	const float invCurrRange = 1.0f / fmaxf(currRange, eps);
	const float invHistRange = 1.0f / fmaxf(histRange, eps);

	if (histRange < eps || currRange < eps) // avoid division by zero
	{
		return;
	}

	const float currLowest = currLow_HightValues_shared[0]; // Get from constant memory
	const float histLowest = histLow_HightValues[0]; // Get from constant memory

	float score = 0.0f;
	int histBaseIdx = threadIdx.x;
#pragma unroll 64 // assuming max pattern length is 30-64, adjust as needed
	for (int i = 0; i < currPatternLen; i++)
	{
		const float histValue = histMAs_values_shared[histBaseIdx + i];
		const float histSlope = histMAs_slopes_shared[histBaseIdx + i];
		const float currValue = currMAs_values_shared[i]; // from shared memory
		const float currSlope = currMAs_slopes_shared[i]; // from shared memory

		score += CalcSingleMAScore(
			histValue,
			histSlope,
			currValue,
			currSlope,
			histLowest,
			currLowest,
			invHistRange,
			invCurrRange);
	}
	d_scores[idx] += score * weight; // final score for this position
}


__device__ __forceinline__ void CandidatePatternScanCalc_Device_MA(
	const float* __restrict__ histMAs_values /*array for MAs hist values */
	, const float* __restrict__ histMAs_slopes /*array for MAs hist slopes */
	, const float* __restrict__ currMAs_values_shared /* array for MAs curr values in shared memory*/
	, const float* __restrict__ currMAs_slopes_shared /* array for MAs curr slopes in shared memory*/

	, const float* __restrict__ histLow_HightValues /* array for hist low/high values*/
	, const float* __restrict__ currLow_HightValues_shared /* array for curr low/high values in shared memory*/
	, const int histBaseIdx /*base index in hist arrays*/

	, float* d_scores /*output scores, where Score(idx) is the score of currMAs(t)&&histMAs(idx + t), t is the index of the current candle */
	, const int currPatternLen, const float weight, const int idx)
{
	const float currRange = currLow_HightValues_shared[1] - currLow_HightValues_shared[0];
	const float histRange = histLow_HightValues[1] - histLow_HightValues[0];

	const float eps = 1e-8f;
	const float invCurrRange = 1.0f / fmaxf(currRange, eps);
	const float invHistRange = 1.0f / fmaxf(histRange, eps);

	if (histRange == 0 || currRange == 0) // avoid division by zero
	{
		return;
	}

	const float currLowest = currLow_HightValues_shared[0]; // Get from constant memory
	const float histLowest = histLow_HightValues[0]; // Get from constant memory

	float score = 0.0f;
#pragma unroll 64 // assuming max pattern length is 30-64, adjust as needed
	for (int i = 0; i < currPatternLen; i++)
	{
		const float histValue = histMAs_values[histBaseIdx + i];
		const float histSlope = histMAs_slopes[histBaseIdx + i];
		const float currValue = currMAs_values_shared[i]; // from shared memory
		const float currSlope = currMAs_slopes_shared[i]; // from shared memory

		score += CalcSingleMAScore(
			histValue,
			histSlope,
			currValue,
			currSlope,
			histLowest,
			currLowest,
			invHistRange,
			invCurrRange);
	}
	d_scores[idx] += score * weight; // final score for this position
}

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
	, const int currPatternLen, const int outLen, const float weight)
{
	extern __shared__ float sharedMem[]; // dynamically allocated shared memory
	float* currMAs_values_shared = sharedMem; // size: currPatternLen
	float* currMAs_slopes_shared = &sharedMem[currPatternLen]; // size: currPatternLen
	float* currLow_HightValues_shared = &sharedMem[2 * currPatternLen]; // size: 2

	if (threadIdx.x == 0)
	{
		currLow_HightValues_shared[0] = d_currMAs_MinValues[0];
		currLow_HightValues_shared[1] = d_currMAs_MaxValues[0];
	}
	// Load currMAs_values and currMAs_slopes into shared memory
	for (int i = threadIdx.x; i < currPatternLen; i += blockDim.x) {
		currMAs_values_shared[i] = d_currMAs_values[i];
		currMAs_slopes_shared[i] = d_currMAs_slopes[i];
	}

	__syncthreads(); // Ensure all threads have loaded the data -- NOTE: you'll get intelisense error (undefined), but it compiles and works fine


	int cid = blockIdx.x * blockDim.x + threadIdx.x;
	if (cid >= candidateCount) return; // out of bounds check
	int workIdx = d_candidateIndices[cid];
	if (workIdx >= outLen) return; // out of bounds check

	float histLow_HightValues[2];
	histLow_HightValues[0] = d_histMAs_MinValues[workIdx];
	histLow_HightValues[1] = d_histMAs_MaxValues[workIdx];

	int histBaseIdx = workIdx; // base index in hist arrays for this candidate

	CandidatePatternScanCalc_Device_MA(
		d_histMAs_values,
		d_histMAs_slopes,
		currMAs_values_shared,
		currMAs_slopes_shared,
		histLow_HightValues,
		currLow_HightValues_shared,
		histBaseIdx,
		d_scores,
		currPatternLen, weight, workIdx);


}

// MA score calcs
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
	, const int currPatternLen, const int outLen, const float weight)
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;
	if (idx >= outLen) return; // out of bounds check

	extern __shared__ float sharedMem[]; // dynamically allocated shared memory
	float* currMAs_values_shared = sharedMem; // size: currPatternLen
	float* currMAs_slopes_shared = &sharedMem[currPatternLen]; // size: currPatternLen
	float* currLow_HightValues_shared = &sharedMem[2 * currPatternLen]; // size: 2

	int histTileLen = blockDim.x + currPatternLen - 1;
	float* histMAs_values_shared = &sharedMem[2 * currPatternLen + 2]; // size: histTileLen
	float* histMAs_slopes_shared = &sharedMem[2 * currPatternLen + 2 + histTileLen]; // size: histTileLen

	// Load currMAs_values and currMAs_slopes into shared memory
	for (int i = threadIdx.x; i < currPatternLen; i += blockDim.x) {
		currMAs_values_shared[i] = d_currMAs_values[i];
		currMAs_slopes_shared[i] = d_currMAs_slopes[i];
	}

	for(int i = threadIdx.x; i < histTileLen; i += blockDim.x) {
		int histIdx = blockIdx.x * blockDim.x + i;
		histMAs_values_shared[i] = d_histMAs_values[histIdx];
		histMAs_slopes_shared[i] = d_histMAs_slopes[histIdx];
	}

	__syncthreads(); // Ensure all threads have loaded the data -- NOTE: you'll get intelisense error (undefined), but it compiles and works fine

	if (threadIdx.x == 0)
	{
		currLow_HightValues_shared[0] = d_currMAs_MinValues[0];
		currLow_HightValues_shared[1] = d_currMAs_MaxValues[0];
	}

	__syncthreads(); // Ensure all threads have loaded the data -- NOTE: you'll get intelisense error (undefined), but it compiles and works fine

	
	float histLow_HightValues[2];
	histLow_HightValues[0] = d_histMAs_MinValues[idx];
	histLow_HightValues[1] = d_histMAs_MaxValues[idx];

	FullPatternScanCalc_Device_MA(
		histMAs_values_shared,
		histMAs_slopes_shared,
		currMAs_values_shared,
		currMAs_slopes_shared,
		histLow_HightValues,
		currLow_HightValues_shared,
		d_scores,
		currPatternLen, weight, idx);
}

