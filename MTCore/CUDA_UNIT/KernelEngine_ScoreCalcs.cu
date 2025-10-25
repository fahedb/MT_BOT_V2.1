#include "KernelEngine_ScoreCalcs.h"
#include <chrono>
// fabsf,... etc include
#include <math.h>

#include "CudaMemoryController.h"

#include <iostream>
#define THREADS_NUM 256
#define TILE_SIZE 512
#include <string>


// CPP:
//{
//	if (hist == curr)
//		return static_cast<int>(weight);
//
//	const double absHist = std::abs(hist);
//	const double absCurr = std::abs(curr);
//	const double diff = std::abs(hist - curr);
//	const double maxV = absHist > absCurr ? absHist : absCurr;
//
//	const double matching = std::fmax(0.0, 1.0 - (diff / maxV));
//	return static_cast<int>(matching * weight);
//}
__host__ __device__ __forceinline__ float RelativeScoreCalc(float hist, float curr) // CUDA device function
{
	
	if (hist == curr)
		return 1;
	const float absHist = fabsf(hist);
	const float absCurr = fabsf(curr);
	const float diff = fabsf(hist - curr);
	const float maxV = absHist > absCurr ? absHist : absCurr;
	const float matching = fmaxf(0.0f, 1.0f - (diff / maxV));
	return matching;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
__device__ __forceinline__
float pair_score(float hVal, float cVal, float hSlp, float cSlp) {
	float s = RelativeScoreCalc(hVal, cVal);
	s += RelativeScoreCalc(hSlp, cSlp);
	return s;
}


__global__ void RsiCalc_TileKernel(
	const float* __restrict__ d_histVal,
	const float* __restrict__ d_histSlope,
	const float* __restrict__ d_currVal,
	const float* __restrict__ d_currSlope,
	float* __restrict__ d_out, // S[t]
	int starts, int K, float weight, int TILE)
{
	extern __shared__ float sharedMem[];
	float* cVal = sharedMem;           // K
	float* cSlope = cVal + K;       // K
	float* hVal = cSlope + K;     // need = tileLen + K - 1
	
	// calc hSlope pointer:
	// calc base index
	int base = blockIdx.x * TILE;
	if (base >= starts) return;
	int tileLen = TILE > starts - base ? starts - base : TILE;
	int need = tileLen + K - 1;
	float* hSlope = hVal + need;

	// Load curr values+slopes to shared
	for (int i = threadIdx.x; i < K; i += blockDim.x) {
		cVal[i] = d_currVal[i];
		cSlope[i] = d_currSlope[i];
	}

	// Load hist values+slopes to shared
	for (int i = threadIdx.x; i < need; i += blockDim.x) {
		hVal[i] = d_histVal[base + i];
		hSlope[i] = d_histSlope[base + i];
	}
	__syncthreads();

	// Calc S[t] for each t in [base, base + tileLen - 1]
	for (int tLocal = threadIdx.x; tLocal < tileLen; tLocal += blockDim.x) {
		float sum = 0.f;
#pragma unroll
		// Calc S[tLocal]
		for (int k = 0; k < K; ++k) {
			sum += pair_score(hVal[tLocal + k], cVal[k],
				hSlope[tLocal + k], cSlope[k]);
		}
		d_out[base + tLocal] += sum * weight; // או '=' אם זה באפר RSI בלבד
	}
}

// RSI score calcs
__global__ void RSIScoreCalcKernel(	  const float* __restrict__ d_histRSI_values /*array for rsi hist values */
									, const float* __restrict__ d_histRSI_slopes /*array for rsi hist slopes */
									, const float* __restrict__ d_currRSI_values /* array for rsi curr values*/
									, const float* __restrict__ d_currRSI_slopes /* array for rsi curr slopes*/
									, float* d_scores /*output scores, where Score(idx) is the score of currRSI(t)&&histRSI(idx + t), t is the index of the current candle */
									, const int currPatternLen, const int dataSize , const float weight)
{
	extern __shared__ float sharedMem[]; // dynamically allocated shared memory
	float* currRSI_values_shared = sharedMem; // size: currPatternLen
	float* currRSI_slopes_shared = &sharedMem[currPatternLen]; // size: currPatternLen
	// Load currRSI_values and currRSI_slopes into shared memory
	for (int i = threadIdx.x; i < currPatternLen; i += blockDim.x) {
		currRSI_values_shared[i] = d_currRSI_values[i];
		currRSI_slopes_shared[i] = d_currRSI_slopes[i];
	}
	__syncthreads(); // Ensure all threads have loaded the data -- NOTE: you'll get intelisense error (undefined), but it compiles and works fine

	int idx = blockIdx.x * blockDim.x + threadIdx.x;
	
	int lastStart = dataSize - currPatternLen;
	if (idx > lastStart) return; // out of bounds check

		float score = 0.0f;
#pragma unroll 64 // assuming max pattern length is 30-64, adjust as needed
	for (int i = 0; i < currPatternLen; i++)
	{
		const float histValue = d_histRSI_values[idx + i];
		const float histSlope = d_histRSI_slopes[idx + i];
		const float currValue = currRSI_values_shared[i]; // from shared memory
		const float currSlope = currRSI_slopes_shared[i]; // from shared memory
		score += pair_score(histValue, currValue, histSlope, currSlope);

	}
	d_scores[idx] += score * weight; // final score for this position


}


// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

// MAs score calcs
__device__ void Find_MaxMinValues(const float* __restrict__ d_values, int d_size, float* results)
{
	// find min max and init in results[0] and results[1]
	float min = 1000000.0f;
	float max = 0.0f;
	for (int i = 0; i < d_size; i++)
	{
		float val = d_values[i];
		if (val < min) min = val;
		if (val > max) max = val;
	}
	results[0] = min;
	results[1] = max;
}
// -----------------------------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
///// MA Kernerls: /////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Inline helper for the scoring between two scalars (you already have this name)
__device__ __forceinline__
float RelativeScoreCalc_fast(float a, float b) {
	// If your original RelativeScoreCalc is simple, inline it here.
	// If it's more complex, keep calling the original but mark it __forceinline__ there as well.
	return RelativeScoreCalc(a, b);
}

// Tiled MA score kernel
// - Loads curr (values+slopes) to shared once per block
// - Loads a history tile (values+slopes) of length need = tileLen + K - 1 to shared
// - For each output inside the tile, scans K items from shared to:
//     1) compute hist min/max (first pass)
//     2) accumulate score using normalized values and slopes (second pass)
__global__ void MAScoreCalcKernel_Tiled(const float* __restrict__ d_histVal,
	const float* __restrict__ d_histSlope,
	const float* __restrict__ d_currVal,
	const float* __restrict__ d_currSlope,
	float* __restrict__ d_scores,
	const int K, const int dataSize,
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
	const int starts = dataSize - K + 1;
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
			// Slopes contribution (unchanged)
			sum += RelativeScoreCalc_fast(histSlope_sh[tLocal + k], currSlope_sh[k]);

			// Values contribution (normalized to [0,1] using each window's min/max and curr's min/max)
			if (doNorm) {
				float normHist = (histVal_sh[tLocal + k] - histMin) * invHistRange;
				float normCurr = (currVal_sh[k] - currLowest) * invCurrRange;
				sum += RelativeScoreCalc_fast(normHist, normCurr);
			}
		}

		// Write result: if d_scores accumulates multiple indicators use +=, else use '='.
		d_scores[base + tLocal] += sum * weight;
	}
}


__global__ void MAScoreCalcKernel(const float* __restrict__ d_histMAs_values /*array for MAs hist values */
	, const float* __restrict__ d_histMAs_slopes /*array for MAs hist slopes */
	, const float* __restrict__ d_currMAs_values /* array for MAs curr values*/
	, const float* __restrict__ d_currMAs_slopes /* array for MAs curr slopes*/
	, float* __restrict__ d_scores /*output scores, where Score(idx) is the score of currMAs(t)&&histMAs(idx + t), t is the index of the current candle */
	, const int currPatternLen, const int dataSize, const float weight)
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
	
	int lastStart = dataSize - currPatternLen;
	if (idx > lastStart) return; // out of bounds check
	
	float histLow_HightValues[2];
	Find_MaxMinValues(d_histMAs_values + idx, currPatternLen, histLow_HightValues);

	const float currRange = currLow_HightValues_shared[1] - currLow_HightValues_shared[0];
	const float histRange = histLow_HightValues[1] - histLow_HightValues[0];

	const float eps = 1e-8f;
	const float invCurrRange = 1.0f / fmaxf(currRange, eps);
	const float invHistRange = 1.0f / fmaxf(histRange, eps);

	if(histRange==0 || currRange==0) // avoid division by zero
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
		score += RelativeScoreCalc(histSlope, currSlope); // compare slopes

		// Normalize values to [0, 1] range based on the lowest and highest values
		const float normHistValue = (histValue - histLowest) * invHistRange;
		const float normCurrValue = (currValue - currLowest) * invCurrRange;
		score += RelativeScoreCalc(normHistValue, normCurrValue); // compare normalized values
	}
	d_scores[idx] += score * weight; // final score for this position
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

// Ind score calcs (for 2 Inds different lengths, e.g., 50 and 200)
// Theory -- we compare the difference between the two Inds for hist and curr, both for value and slope
// e.g., if for curr, Ind1 - Ind2 = 5, and for hist, Ind1 - Ind2 = 5.5, then they are ~similar
__global__ void TwoIndScoreCalcKernel(const float* __restrict__ d_histLenX_values /*array for  Ind1 hist values */
									, const float* __restrict__ d_histLenX_slopes /*array for  Ind1 hist slopes */
									, const float* __restrict__ d_currLenX_values /* array for Ind1 curr values*/
									, const float* __restrict__ d_currLenX_slopes /* array for Ind1 curr slopes*/
									, const float* __restrict__ d_histLenY_values /*array for  Ind2 hist values */
									, const float* __restrict__ d_histLenY_slopes /*array for  Ind2 hist slopes */
									, const float* __restrict__ d_currLenY_values /* array for Ind2 curr values*/
									, const float* __restrict__ d_currLenY_slopes /* array for Ind2 curr slopes*/
									, float* __restrict__ d_scores /*output scores, where Score(idx) is the score of currMAs(t)&&histMAs(idx + t), t is the index of the current candle */
									, const int currPatternLen, const int histDataSize, const float weight)
{
	extern __shared__ float sharedMem[]; // dynamically allocated shared memory
	float* currLenX_values_shared = sharedMem; // size: currPatternLen
	float* currLenX_slopes_shared = &sharedMem[currPatternLen]; // size: currPatternLen
	float* currLenY_values_shared = &sharedMem[2 * currPatternLen]; // size: currPatternLen
	float* currLenY_slopes_shared = &sharedMem[3 * currPatternLen]; // size: currPatternLen
	float* currLow_HighValues_LenX_shared = &sharedMem[4 * currPatternLen];
	float* currLow_HighValues_LenY_shared = &sharedMem[(4 * currPatternLen) + 2];
	
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
		Find_MaxMinValues(currLenY_values_shared, currPatternLen, currLow_HighValues_LenY_shared);
		Find_MaxMinValues(currLenX_values_shared, currPatternLen, currLow_HighValues_LenX_shared);
	}
	__syncthreads(); // Ensure all threads have loaded the data -- NOTE: you'll get intelisense error (undefined), but it compiles and works fine

	int idx = blockIdx.x * blockDim.x + threadIdx.x;
	
	int lastStart = histDataSize - currPatternLen;
	if (idx > lastStart) return; // out of bounds check
	
	float histLow_HighValues_LenX[2];
	Find_MaxMinValues(d_histLenX_values + idx, currPatternLen, histLow_HighValues_LenX);

	float histLow_HighValues_LenY[2];
	Find_MaxMinValues(d_histLenY_values + idx, currPatternLen, histLow_HighValues_LenY);
	float score = 0.0f;
	

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
	
	
#pragma unroll 64 // assuming max pattern length is 30-64, adjust as needed
	for (int i = 0; i < currPatternLen; i++)
	{
		// Len X inits
		const float histValue_LenX = d_histLenX_values[idx + i];
		const float histSlope_LenX = d_histLenX_slopes[idx + i];
		const float currValue_LenX = currLenX_values_shared[i]; // from shared memory
		const float currSlope_LenX = currLenX_slopes_shared[i]; // from shared memory

		/////////////////////////////////////////////////////////////////////////////////
		// len Y inits
		const float histValue_LenY = d_histLenY_values[idx + i];
		const float histSlope_LenY = d_histLenY_slopes[idx + i];
		const float currValue_LenY = currLenY_values_shared[i]; // from shared memory
		const float currSlope_LenY = currLenY_slopes_shared[i]; // from shared memory

		/////////////////////////////////////////////////////////////////////////////////


		/////////////////////////////////////////////////////////////////////////////////
		// Calc normalized diff between  LenX and  LenY for hist vs curr ///////////
		// //////////////////////////////////////////////////////////////////////////////
		// Normalize LenX values
		float normHistValue_LenX = (histValue_LenX - histLowest_LenX) * invHistRange_LenX;
		float normCurrValue_LenX = (currValue_LenX - currLowest_LenX) * invCurrRange_LenX;

		float normHistValue_LenY = (histValue_LenY - histLowest_LenY) * invHistRange_LenY;
		float normCurrValue_LenY = (currValue_LenY - currLowest_LenY) * invCurrRange_LenY;
	
		const float currValDiff = normCurrValue_LenX - normCurrValue_LenY;
		const float histValDiff = normHistValue_LenX - normHistValue_LenY;
		score += RelativeScoreCalc(histValDiff, currValDiff); // compare Ind value differences
		///////////////////////////////////////////////////////////////////////////////
		

		/////////////////////////////////////////////////////////////////////////////
		// Calc slope diff between LenX and LenY for hist and curr ///////////
		////////////////////////////////////////////////////////////////////////////////
		const float currSlopeDiff = currSlope_LenX - currSlope_LenY;
		const float histSlopeDiff = histSlope_LenX - histSlope_LenY;
		score += RelativeScoreCalc(histSlopeDiff, currSlopeDiff); // compare Ind slope differences
		//////////////////////////////////////////////////////////////////////////////
	}
	
	d_scores[idx] += score * weight; // final score for this position
	
}
__device__ __forceinline__
float rel_score(float a, float b) {
	// Make sure your original RelativeScoreCalc is __forceinline__ too
	return RelativeScoreCalc(a, b);
}

__global__ void TwoIndScoreCalcKernel_Tiled(
	const float* __restrict__ d_histX_val,
	const float* __restrict__ d_histX_slp,
	const float* __restrict__ d_currX_val,
	const float* __restrict__ d_currX_slp,
	const float* __restrict__ d_histY_val,
	const float* __restrict__ d_histY_slp,
	const float* __restrict__ d_currY_val,
	const float* __restrict__ d_currY_slp,
	float* __restrict__ d_scores,
	const int K, const int dataSize, const float weight, const int TILE)
{
	extern __shared__ float smem[];

	// Shared layout:
	// [0 .. K-1]            -> currX_val
	// [K .. 2K-1]           -> currX_slp
	// [2K .. 3K-1]          -> currY_val
	// [3K .. 4K-1]          -> currY_slp
	// [4K .. 4K+1]          -> currX low/high
	// [4K+2 .. 4K+3]        -> currY low/high
	// [4K+4 .. 4K+4+need-1]                 -> histX_val tile
	// [4K+4+need .. 4K+4+2*need-1]          -> histX_slp tile
	// [4K+4+2*need .. 4K+4+3*need-1]        -> histY_val tile
	// [4K+4+3*need .. 4K+4+4*need-1]        -> histY_slp tile

	float* cX_val = smem;             // K
	float* cX_slp = cX_val + K;       // K
	float* cY_val = cX_slp + K;       // K
	float* cY_slp = cY_val + K;       // K
	float* currX_LH = cY_slp + K;       // 2 (min,max)
	float* currY_LH = currX_LH + 2;     // 2 (min,max)

	const int starts = dataSize - K + 1;
	const int base = blockIdx.x * TILE;
	if (base >= starts) return;

	const int tileLen = (starts - base < TILE ? (starts - base) : TILE);
	const int need = tileLen + K - 1;

	float* hX_val = currY_LH + 2;           // need
	float* hX_slp = hX_val + need;          // need
	float* hY_val = hX_slp + need;          // need
	float* hY_slp = hY_val + need;          // need

	// Load curr (X & Y) to shared
	for (int i = threadIdx.x; i < K; i += blockDim.x) {
		cX_val[i] = d_currX_val[i];
		cX_slp[i] = d_currX_slp[i];
		cY_val[i] = d_currY_val[i];
		cY_slp[i] = d_currY_slp[i];
	}
	__syncthreads();

	// Compute curr low/high for X and Y (thread 0)
	if (threadIdx.x == 0) {
		float mnx = cX_val[0], mxx = cX_val[0];
		float mny = cY_val[0], mxy = cY_val[0];
		for (int i = 1; i < K; ++i) {
			float vx = cX_val[i]; mnx = fminf(mnx, vx); mxx = fmaxf(mxx, vx);
			float vy = cY_val[i]; mny = fminf(mny, vy); mxy = fmaxf(mxy, vy);
		}
		currX_LH[0] = mnx; currX_LH[1] = mxx;
		currY_LH[0] = mny; currY_LH[1] = mxy;
	}
	__syncthreads();

	const float eps = 1e-8f;
	const float currX_range = currX_LH[1] - currX_LH[0];
	const float currY_range = currY_LH[1] - currY_LH[0];
	const float invCurrX = 1.0f / fmaxf(currX_range, eps);
	const float invCurrY = 1.0f / fmaxf(currY_range, eps);
	const float currX_low = currX_LH[0];
	const float currY_low = currY_LH[0];
	const bool  doNormX = (currX_range > 0.0f);
	const bool  doNormY = (currY_range > 0.0f);

	// Load history tiles (X & Y, values & slopes)
	for (int i = threadIdx.x; i < need; i += blockDim.x) {
		hX_val[i] = d_histX_val[base + i];
		hX_slp[i] = d_histX_slp[base + i];
		hY_val[i] = d_histY_val[base + i];
		hY_slp[i] = d_histY_slp[base + i];
	}
	__syncthreads();

	// Compute scores for all outputs within the tile
	for (int tLocal = threadIdx.x; tLocal < tileLen; tLocal += blockDim.x) {
		// Per-output hist low/high for X and Y on shared window [tLocal .. tLocal+K-1]
		float hx_min = hX_val[tLocal], hx_max = hX_val[tLocal];
		float hy_min = hY_val[tLocal], hy_max = hY_val[tLocal];
#pragma unroll
		for (int k = 1; k < K; ++k) {
			float vx = hX_val[tLocal + k]; hx_min = fminf(hx_min, vx); hx_max = fmaxf(hx_max, vx);
			float vy = hY_val[tLocal + k]; hy_min = fminf(hy_min, vy); hy_max = fmaxf(hy_max, vy);
		}
		const float hx_rng = hx_max - hx_min;
		const float hy_rng = hy_max - hy_min;
		const float invHX = 1.0f / fmaxf(hx_rng, eps);
		const float invHY = 1.0f / fmaxf(hy_rng, eps);
		const bool  doNormHX = (hx_rng > 0.0f);
		const bool  doNormHY = (hy_rng > 0.0f);

		float sum = 0.0f;
#pragma unroll
		for (int k = 0; k < K; ++k) {
			// Values normalization and diff: (X - Y) for curr and hist
			float currXv = cX_val[k], currYv = cY_val[k];
			float histXv = hX_val[tLocal + k], histYv = hY_val[tLocal + k];

			if (doNormX && doNormHX) {
				currXv = (currXv - currX_low) * invCurrX;
				histXv = (histXv - hx_min) * invHX;
			}
			if (doNormY && doNormHY) {
				currYv = (currYv - currY_low) * invCurrY;
				histYv = (histYv - hy_min) * invHY;
			}

			const float currValDiff = currXv - currYv;
			const float histValDiff = histXv - histYv;
			sum += rel_score(histValDiff, currValDiff);

			// Slopes diff
			const float currSlopeDiff = cX_slp[k] - cY_slp[k];
			const float histSlopeDiff = hX_slp[tLocal + k] - hY_slp[tLocal + k];
			sum += rel_score(histSlopeDiff, currSlopeDiff);
		}

		d_scores[base + tLocal] += sum * weight;  // use '=' if this buffer is only for TwoInd
	}
}




// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////
///////// Kernels callers ////////////////////////////////////
//////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Two Ind lanunchers: /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void TwoIndCalc_tiled(const float* d_hist_LenX_Val, const float* d_hist_LenX_Slope,
	const float* d_curr_LenX_Val, const float* d_curr_LenX_Slope,
	const float* d_hist_LenY_Val, const float* d_hist_LenY_Slope,
	const float* d_curr_LenY_Val, const float* d_curr_LenY_Slope,
	float* d_results, int K, int histSize, float weight)
{
	const int starts = histSize - K + 1;
	if (starts <= 0) return;

	const int TILE = TILE_SIZE;                 // 512/1024 usually good
	const int threads = THREADS_NUM;          // e.g., 256
	const int blocks = (starts + TILE - 1) / TILE;

	const int need_max = (starts < TILE ? starts : TILE) + K - 1;

	// Shared layout: curr (4*K) + 2*(min,max) for LenX/LenY + hist tiles: 4*need_max
	size_t shBytes = sizeof(float) * (4 * K     /* curr LenX/LenY values+slopes */
		+ 4       /* curr min/max for X and Y (two pairs) */
		+ 4 * need_max /* hist tiles: X.val, X.slp, Y.val, Y.slp */);

	TwoIndScoreCalcKernel_Tiled << <blocks, threads, shBytes >> > (
		d_hist_LenX_Val, d_hist_LenX_Slope,
		d_curr_LenX_Val, d_curr_LenX_Slope,
		d_hist_LenY_Val, d_hist_LenY_Slope,
		d_curr_LenY_Val, d_curr_LenY_Slope,
		d_results, K, histSize, weight, TILE);

#ifndef NDEBUG
	cudaError_t err = cudaGetLastError();
	if (err != cudaSuccess) {
		printf("TwoIndScoreCalcKernel_Tiled launch error: %s\n", cudaGetErrorString(err));
	}
	cudaDeviceSynchronize();
#endif
}


void TwoIndCalc(float* d_hist_LenX_Val, float* d_hist_LenX_Slope,
				float* d_curr_LenX_Val, float* d_curr_LenX_Slope,
				float* d_hist_LenY_Val, float* d_hist_LenY_Slope,
				float* d_curr_LenY_Val, float* d_curr_LenY_Slope,
				float* d_results, int patternLen, int histSize, float weight)
{
	const int starts = histSize - patternLen + 1;
	const int threads = THREADS_NUM;
	const int blocks = (starts + threads - 1) / threads;
	const size_t shBytes = sizeof(float) * ((4 * patternLen) + 4);
	
	TwoIndScoreCalcKernel << <blocks, threads, shBytes >> > (
		d_hist_LenX_Val, d_hist_LenX_Slope,
		d_curr_LenX_Val, d_curr_LenX_Slope,
		d_hist_LenY_Val, d_hist_LenY_Slope,
		d_curr_LenY_Val, d_curr_LenY_Slope,
		d_results,
		patternLen,
		histSize,
		weight
		);

}
// ---------------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////
// MA lanunchers: /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void calcMA_tiled(const float* d_histMA_values,
	const float* d_histMA_slopes,
	const float* d_currMA_values,
	const float* d_currMA_slopes,
	float* d_results,
	int patternLen, int histSize, float weight)
{
	const int K = patternLen;
	const int starts = histSize - K + 1;
	if (starts <= 0) return;

	// Choose a tile size that fits shared memory comfortably.
	// 512/1024 are good starting points; adjust per your SM shared capacity.
	const int TILE = TILE_SIZE;
	const int threads = THREADS_NUM;                 // e.g., 256
	const int blocks = (starts + TILE - 1) / TILE;

	// Worst-case shared footprint: curr(2*K) + currMinMax(2)
	// + histVal(need_max) + histSlope(need_max), where need_max = min(TILE, starts) + K - 1
	const int need_max = (starts < TILE ? starts : TILE) + K - 1;
	size_t shBytes = sizeof(float) * (2 * K              // curr values + slopes
		+ 2                // curr min/max
		+ need_max         // hist values tile
		+ need_max);      // hist slopes tile

	MAScoreCalcKernel_Tiled << <blocks, threads, shBytes >> > (
		d_histMA_values, d_histMA_slopes,
		d_currMA_values, d_currMA_slopes,
		d_results,
		K, histSize, weight, TILE);

#ifndef NDEBUG
	cudaError_t err = cudaGetLastError();
	if (err != cudaSuccess) {
		printf("MAScoreCalcKernel_Tiled launch error: %s\n", cudaGetErrorString(err));
	}
	cudaDeviceSynchronize();
#endif
}


void calcMA(float* d_histMA_values, float* d_histMA_slopes,
			float* d_currMA_values, float* d_currMA_slopes, 
			float* d_results, int patternLen, int histSize, float weight)
{
	const int starts = histSize - patternLen + 1;
	const int threads = THREADS_NUM;
	const int blocks = (starts + threads - 1) / threads;
	const size_t shBytes = sizeof(float) * ((2 * patternLen) + 2);
	MAScoreCalcKernel << <blocks, threads, shBytes >> > (
		d_histMA_values,
		d_histMA_slopes,
		d_currMA_values,
		d_currMA_slopes,
		d_results,
		patternLen,
		histSize,
		weight
		);
}

// ----------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////
//////////////// RSI launchers: //////////////////////////////////
//////////////////////////////////////////////////////////////////
void calcRSI(float* d_histRSI_values, float* d_histRSI_slopes, float* d_currRSI_values, float* d_currRSI_slopes,float* d_results, int patternLen, int histSize, float weight)
{
	const int starts = histSize - patternLen + 1;   // number of starting points in hist where pattern can fit
	const int threads = THREADS_NUM;                         // 128/256, depending on GPU
	const int blocks = (starts + threads - 1) / threads;
	const size_t shBytes = sizeof(float) * (2 * patternLen); // curr values+slopes

	RSIScoreCalcKernel << <blocks, threads, shBytes >> > (
		d_histRSI_values,
		d_histRSI_slopes,
		d_currRSI_values,
		d_currRSI_slopes,
		d_results,
		patternLen,
		histSize,
		weight
		);

	// Log any errors from launch if not in release mode
#ifndef NDEBUG
	cudaError_t err = cudaGetLastError();
	if (err != cudaSuccess) {
		printf("RSIScoreCalcKernel launch error: %s\n", cudaGetErrorString(err));
	}
	cudaDeviceSynchronize();
#endif
}


void calcRSI_tiled(const float* d_histVal, const float* d_histSlope,
	const float* d_currVal, const float* d_currSlope,
	float* d_resOut, int currPatternLen, int histSize, float weight)
{
	const int starts = histSize - currPatternLen + 1;
	if (starts <= 0) return;

	constexpr int TILE = TILE_SIZE;           // tile size for processing
	const int blocks = (starts + TILE - 1) / TILE;
	const int threads = THREADS_NUM;  
	int need = TILE > starts ? starts : TILE; // get min between TILE and starts
	need += (currPatternLen - 1); // add K-1 for slope/val overlap

	size_t shBytes = sizeof(float) * (2 * currPatternLen               // curr val+slope
		+ (need)            // hist val tile
		+(need));         // hist slope tile

	RsiCalc_TileKernel << <blocks, threads, shBytes >> > (
		d_histVal, d_histSlope, d_currVal, d_currSlope,
		d_resOut, starts, currPatternLen, weight, TILE);

#ifndef NDEBUG
	cudaError_t err = cudaGetLastError();
	if (err != cudaSuccess) {
		printf("CorrTileKernel launch error: %s\n", cudaGetErrorString(err));
	}
	cudaDeviceSynchronize();
#endif
}