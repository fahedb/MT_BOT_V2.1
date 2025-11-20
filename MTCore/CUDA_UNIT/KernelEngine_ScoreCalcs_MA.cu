#include "KernelEngine_ScoreCalcs_MA.h"

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

// Slide kernel helper 
// The math behind this function is:
// Let's assume the Res1 is the Result Array for scan 1, and Res2 is for scan 2
// So if Res1[t] = Sum(i=0 to K-1) pair_score(H[t+i], C[i]), 
// Then we insert a new data to C, So we can calc Res2[t+1] = Sum(i=1 to K) pair_score(H[t+i], C[i])
// As Res2[t+1] = Res1[t] - pair_score(H[t], C[0]) + pair_score(H[t+K], C[K])
// Run time: O(1) per t, or O(N) for all t in [0, outLen-1]
__device__ __forceinline__ void SlideCalc_Device_MA(
	const float* __restrict__ d_histVal,
	const float* __restrict__ d_histSlope,
	const float* __restrict__ d_currVal,
	const float* __restrict__ d_currSlope,
	const float* __restrict__ currLow_HightValues_shared, // from shared memory
	float* __restrict__ d_outResNew, // S[t]
	const float* __restrict__ d_outResOld, // S[t] for prev t (Res1)
	float* __restrict__ d_newPrevIdx0Score, // pair_score(H[t], C[0]) for current t (to be used in next iteration)
	const float* __restrict__ d_oldPrevIdx0Score, // pair_score(H[t], C[0]) for prev t (Res1) (Need to be const to avoid accidental modification -- Pointer swaped in host)
	int K, float weight, int t)
{

	float histLow_HightValues[2];
	Find_MaxMinValues(d_histVal + t, K, histLow_HightValues);

	const float currRange = currLow_HightValues_shared[1] - currLow_HightValues_shared[0];
	const float histRange = histLow_HightValues[1] - histLow_HightValues[0];

	const float eps = 1e-8f;
	const float invCurrRange = 1.0f / fmaxf(currRange, eps);
	const float invHistRange = 1.0f / fmaxf(histRange, eps);

	if (histRange == 0 || currRange == 0) // avoid division by zero
	{
		return;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////

	const float currLowest = currLow_HightValues_shared[0]; // Get from constant memory
	const float histLowest = histLow_HightValues[0]; // Get from constant memory



	/////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////
	// Calc pair for idx K-1
	float histVal_K = d_histVal[t + K - 1];
	float histSlope_K = d_histSlope[t + K - 1];
	float currVal_K = d_currVal[K - 1];
	float currSlope_K = d_currSlope[K - 1];

	float score_K =
	CalcSingleMAScore(
		histVal_K,
		histSlope_K,
		currVal_K,
		currSlope_K,
		histLowest,
		currLowest,
		invHistRange,
		invCurrRange);

	//////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////

	// Calc ResNew[t] based on ResOld[t-1]
	float resOld_t = d_outResOld[t - 1]; // ResOld[t-1]
	float prevIdx0Score = d_oldPrevIdx0Score[t - 1]; // pair_score(H[t], C[0]) for prev t-1 (Res1)
	d_outResNew[t] = resOld_t - prevIdx0Score + score_K; // calc ResNew[t]

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	// Now calc pair for idx 0 to store for t+1 in next iteration
	float histVal_0 = d_histVal[t];
	float histSlope_0 = d_histSlope[t];
	float currVal_0 = d_currVal[0];
	float currSlope_0 = d_currSlope[0];

	float score_0 = 
		CalcSingleMAScore(
		histVal_0,
		histSlope_0,
		currVal_0,
		currSlope_0,
		histLowest,
		currLowest,
		invHistRange,
		invCurrRange);

	d_newPrevIdx0Score[t] += score_0 * weight; // store for t+1 in next iteration
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
}

__device__ __forceinline__ void FullPatternScanCalc_Device_MA(
	const float* __restrict__ d_histMAs_values /*array for MAs hist values */
	, const float* __restrict__ d_histMAs_slopes /*array for MAs hist slopes */
	, const float* __restrict__ currMAs_values_shared /* array for MAs curr values in shared memory*/
	, const float* __restrict__ currMAs_slopes_shared /* array for MAs curr slopes in shared memory*/

	, const float* __restrict__ histLow_HightValues /* array for hist low/high values in shared memory*/
	, const float* __restrict__ currLow_HightValues_shared /* array for curr low/high values in shared memory*/
	
	, float* d_scores /*output scores, where Score(idx) is the score of currMAs(t)&&histMAs(idx + t), t is the index of the current candle */
	, float* __restrict__ d_prevIdx0Score /*previous score for idx 0, used in sliding window*/
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
		const float histValue = d_histMAs_values[idx + i];
		const float histSlope = d_histMAs_slopes[idx + i];
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

		if(i==0)
			d_prevIdx0Score[idx] += score * weight; // store for sliding window
	}
	d_scores[idx] += score * weight; // final score for this position
}

// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------

// Tiled MA score kernel
// - Loads curr (values+slopes) to shared once per block
// - Loads a history tile (values+slopes) of length need = tileLen + K - 1 to shared
// - For each output inside the tile, scans K items from shared to:
//     1) compute hist min/max (first pass)
//     2) accumulate score using normalized values and slopes (second pass)
__global__ void TiledScan_Kernel_MA(const float* __restrict__ d_histVal,
	const float* __restrict__ d_histSlope,
	const float* __restrict__ d_currVal,
	const float* __restrict__ d_currSlope,
	float* __restrict__ d_scores,
	float* __restrict__ d_prevIdx0Score, /*previous score for idx 0, used in sliding window*/
	const int K, const int outLen,
	const float weight, const int TILE)
{
	extern __shared__ float smem[];
	// Layout in shared:
	// [0 .. K-1]            -> curr values
	// [K .. 2K-1]           -> curr slopes
	// [2K .. 2K+1]          -> curr low/high (2 floats)
	// [2K+2 .. 2K+2 + need-1]       -> hist values tile
	// [2K+2+need .. 2K+2+2*need-1]  -> hist slopes tile

	float* currVal_sh = smem;                 // K
	float* currSlope_sh = currVal_sh + K;       // K
	float* currLowHigh = currSlope_sh + K;     // 2

	// Compute tile geometry
	const int starts = outLen;
	const int base = blockIdx.x * TILE;       // first output index covered by this block
	if (base >= starts) return;

	const int tileLen = (starts - base < TILE ? (starts - base) : TILE);
	const int need = tileLen + K - 1;

	float* histVal_sh = currLowHigh + 2;      // need
	float* histSlope_sh = histVal_sh + need;    // need

	// Load curr to shared (values & slopes)
	for (int i = threadIdx.x; i < K; i += blockDim.x) {
		currVal_sh[i] = d_currVal[i];
		currSlope_sh[i] = d_currSlope[i];
	}
	__syncthreads();

	// Compute curr min/max (thread 0 does it; you can parallelize if you want)
	if (threadIdx.x == 0) {
		float mn = currVal_sh[0], mx = currVal_sh[0];
		for (int i = 1; i < K; ++i) {
			float v = currVal_sh[i];
			mn = fminf(mn, v);
			mx = fmaxf(mx, v);
		}
		currLowHigh[0] = mn;
		currLowHigh[1] = mx;
	}
	__syncthreads();

	// Load history tile (values & slopes) to shared
	for (int i = threadIdx.x; i < need; i += blockDim.x) {
		histVal_sh[i] = d_histVal[base + i];
		histSlope_sh[i] = d_histSlope[base + i];
	}
	__syncthreads();

	// Precompute for normalization
	const float currRange = currLowHigh[1] - currLowHigh[0];
	const float eps = 1e-8f;
	const float invCurrRange = 1.0f / fmaxf(currRange, eps);
	const float currLowest = currLowHigh[0];

	// Each thread computes multiple outputs inside the tile (register blocking optional)
	for (int tLocal = threadIdx.x; tLocal < tileLen; tLocal += blockDim.x) {

		// Pass 1: compute hist low/high over the window [tLocal .. tLocal+K-1]
		float histMin = histVal_sh[tLocal];
		float histMax = histVal_sh[tLocal];
#pragma unroll
		for (int k = 1; k < K; ++k) {
			float v = histVal_sh[tLocal + k];
			histMin = fminf(histMin, v);
			histMax = fmaxf(histMax, v);
		}
		const float histRange = histMax - histMin;
		const float invHistRange = 1.0f / fmaxf(histRange, eps);

		// Optional: if either range is zero, you can early-return or just skip normalization diff
		// Here we simply skip value normalization when histRange==0 or currRange==0
		bool doNorm = (histRange > 0.0f && currRange > 0.0f);

		// Pass 2: accumulate score over K
		float sum = 0.0f;
#pragma unroll
		for (int k = 0; k < K; ++k) {
			
			sum += CalcSingleMAScore(
				histVal_sh[tLocal + k],
				histSlope_sh[tLocal + k],
				currVal_sh[k],
				currSlope_sh[k],
				histMin,
				currLowest,
				invHistRange,
				invCurrRange);
			if (k == 0)
				d_prevIdx0Score[base + tLocal] += sum * weight; // store for sliding window
		}

		// Write result: if d_scores accumulates multiple indicators use +=, else use '='.
		d_scores[base + tLocal] += sum * weight;
	}
}


// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------

// MA score calcs
__global__ void FullScan_Kernel_MA(const float* __restrict__ d_histMAs_values /*array for MAs hist values */
	, const float* __restrict__ d_histMAs_slopes /*array for MAs hist slopes */
	, const float* __restrict__ d_currMAs_values /* array for MAs curr values*/
	, const float* __restrict__ d_currMAs_slopes /* array for MAs curr slopes*/
	, float* __restrict__ d_scores /*output scores, where Score(idx) is the score of currMAs(t)&&histMAs(idx + t), t is the index of the current candle */
	, float* __restrict__ d_prevIdx0Score /*previous score for idx 0, used in sliding window*/
	, const int currPatternLen, const int outLen, const float weight)
{
	extern __shared__ float sharedMem[]; // dynamically allocated shared memory
	float* currMAs_values_shared = sharedMem; // size: currPatternLen
	float* currMAs_slopes_shared = &sharedMem[currPatternLen]; // size: currPatternLen
	float* currLow_HightValues_shared = &sharedMem[2 * currPatternLen];
	// Load currMAs_values and currMAs_slopes into shared memory
	for (int i = threadIdx.x; i < currPatternLen; i += blockDim.x) {
		currMAs_values_shared[i] = d_currMAs_values[i];
		currMAs_slopes_shared[i] = d_currMAs_slopes[i];
	}

	__syncthreads(); // Ensure all threads have loaded the data -- NOTE: you'll get intelisense error (undefined), but it compiles and works fine

	if (threadIdx.x == 0)
		Find_MaxMinValues(currMAs_values_shared, currPatternLen, currLow_HightValues_shared);
	__syncthreads(); // Ensure all threads have loaded the data -- NOTE: you'll get intelisense error (undefined), but it compiles and works fine

	int idx = blockIdx.x * blockDim.x + threadIdx.x;

	int lastStart = outLen;
	if (idx >= lastStart) return; // out of bounds check

	float histLow_HightValues[2];
	Find_MaxMinValues(d_histMAs_values + idx, currPatternLen, histLow_HightValues);

	FullPatternScanCalc_Device_MA(
		d_histMAs_values,
		d_histMAs_slopes,
		currMAs_values_shared,
		currMAs_slopes_shared,
		histLow_HightValues,
		currLow_HightValues_shared,
		d_scores, d_prevIdx0Score,
		currPatternLen, weight, idx);
}

// ---------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------

// Slide MA score kernel
__global__ void SlideScan_Kernel_MA(const float* __restrict__ d_histMAs_values /*array for MAs hist values */
	, const float* __restrict__ d_histMAs_slopes /*array for MAs hist slopes */
	, const float* __restrict__ d_currMAs_values /* array for MAs curr values*/
	, const float* __restrict__ d_currMAs_slopes /* array for MAs curr slopes*/
	, float* __restrict__ d_outResNew // S[t]
	, const float* __restrict__ d_outResOld // S[t] for prev t (Res1) (Need to be const to avoid accidental modification -- Pointer swaped in host)
	, float* __restrict__ d_newPrevIdx0Score // previous score for idx 0, used in sliding window (to be updated)
	, const float* __restrict__ d_oldPrevIdx0Score // pair_score(H[t], C[0]) for prev t (Res1)
	, const int K, const int outLen, const float weight)
{
	extern __shared__ float sharedMem[]; // dynamically allocated shared memory
	float* currMAs_values_shared = sharedMem; // size: currPatternLen
	float* currMAs_slopes_shared = &sharedMem[K]; // size: currPatternLen
	float* currLow_HightValues_shared = &sharedMem[2 * K];
	// Load currMAs_values and currMAs_slopes into shared memory
	for (int i = threadIdx.x; i < K; i += blockDim.x) {
		currMAs_values_shared[i] = d_currMAs_values[i];
		currMAs_slopes_shared[i] = d_currMAs_slopes[i];
	}

	__syncthreads(); // Ensure all threads have loaded the data -- NOTE: you'll get intelisense error (undefined), but it compiles and works fine

	if (threadIdx.x == 0)
		Find_MaxMinValues(currMAs_values_shared, K, currLow_HightValues_shared);
	__syncthreads(); // Ensure all threads have loaded the data -- NOTE: you'll get intelisen

	int t = blockIdx.x * blockDim.x + threadIdx.x;
	int lastStart = outLen;
	if (t >= lastStart) return; // out of bounds check

	float histLow_HightValues[2];
	Find_MaxMinValues(d_histMAs_values + t, K, histLow_HightValues);

	if (t == 0)
	{
		FullPatternScanCalc_Device_MA(
			d_histMAs_values,
			d_histMAs_slopes,
			currMAs_values_shared,
			currMAs_slopes_shared,
			histLow_HightValues,
			currLow_HightValues_shared,
			d_outResNew, d_newPrevIdx0Score,
			K, weight, t); 
	}
	else
	{
		SlideCalc_Device_MA(
			d_histMAs_values,
			d_histMAs_slopes,
			currMAs_values_shared,
			currMAs_slopes_shared,
			currLow_HightValues_shared,
			d_outResNew,
			d_outResOld,
			d_newPrevIdx0Score, d_oldPrevIdx0Score,
			K, weight,t);
	}
}
