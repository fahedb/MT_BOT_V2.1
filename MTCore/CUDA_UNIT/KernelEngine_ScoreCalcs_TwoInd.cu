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
	float normHistValue_LenX = (histXv - histX_low) * invHistX;
	float normCurrValue_LenX = (currXv - currX_low) * invCurrX;

	float normHistValue_LenY = (histYv - histY_low) * invHistY;
	float normCurrValue_LenY = (currYv - currY_low) * invCurrY;

	const float currValDiff = normCurrValue_LenX - normCurrValue_LenY;
	const float histValDiff = normHistValue_LenX - normHistValue_LenY;

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

// Full Scan two indicators kernel
// Ind score calcs (for 2 Inds different lengths, e.g., 50 and 200)
// Theory -- we compare the difference between the two Inds for hist and curr, both for value and slope
// e.g., if for curr, Ind1 - Ind2 = 5, and for hist, Ind1 - Ind2 = 5.5, then they are ~similar
__global__ void FullScan_Kernel_TwoInd(const float* __restrict__ d_histLenX_values /*array for  Ind1 hist values */
	, const float* __restrict__ d_histLenX_slopes /*array for  Ind1 hist slopes */
	, const float* __restrict__ d_currLenX_values /* array for Ind1 curr values*/
	, const float* __restrict__ d_currLenX_slopes /* array for Ind1 curr slopes*/
	, const float* __restrict__ d_histLenY_values /*array for  Ind2 hist values */
	, const float* __restrict__ d_histLenY_slopes /*array for  Ind2 hist slopes */
	, const float* __restrict__ d_currLenY_values /* array for Ind2 curr values*/
	, const float* __restrict__ d_currLenY_slopes /* array for Ind2 curr slopes*/
	, float* __restrict__ d_scores /*output scores, where Score(idx) is the score of currMAs(t)&&histMAs(idx + t), t is the index of the current candle */
	, float* __restrict__ d_prevIdx0Score /*previous score for idx 0, used in sliding window*/
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
		if (i == 0)
			d_prevIdx0Score[idx] = score * weight;
	}

	d_scores[idx] += score * weight; // final score for this position

}

// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------


__global__ void TiledScan_Kernel_TwoInd(
	const float* __restrict__ d_histX_val,
	const float* __restrict__ d_histX_slp,
	const float* __restrict__ d_currX_val,
	const float* __restrict__ d_currX_slp,
	const float* __restrict__ d_histY_val,
	const float* __restrict__ d_histY_slp,
	const float* __restrict__ d_currY_val,
	const float* __restrict__ d_currY_slp,
	float* __restrict__ d_scores,
	float* __restrict__ d_prevIdx0Score, /*previous score for idx 0, used in sliding window*/

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
			float currXv = cX_val[k];
			float currYv = cY_val[k];
			float histXv = hX_val[tLocal + k];
			float histYv = hY_val[tLocal + k];

			sum +=
				SingleCalc_Device_TwoInd(
				histXv,
				hX_slp[tLocal + k],
				currXv,
				cX_slp[k],
				histYv,
				hY_slp[tLocal + k],
				currYv,
				cY_slp[k],
				currX_low,
				currY_low,
				invCurrX,
				invCurrY,
				hx_min,
				hy_min,
				invHX,
				invHY
			);
			if(k ==0)
				d_prevIdx0Score[base + tLocal] = sum * weight;
		}

		d_scores[base + tLocal] += sum * weight;  // use '=' if this buffer is only for TwoInd
	}
}


// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------

// Slide window Two Ind score kernel
__global__ void SlideCalc_Kernel_TwoInd(
	const float* __restrict__ d_histLenX_values /*array for  Ind1 hist values */
	, const float* __restrict__ d_histLenX_slopes /*array for  Ind1 hist slopes */
	, const float* __restrict__ d_currLenX_values /* array for Ind1 curr values*/
	, const float* __restrict__ d_currLenX_slopes /* array for Ind1 curr slopes*/
	, const float* __restrict__ d_histLenY_values /*array for  Ind2 hist values */
	, const float* __restrict__ d_histLenY_slopes /*array for  Ind2 hist slopes */
	, const float* __restrict__ d_currLenY_values /* array for Ind2 curr values*/
	, const float* __restrict__ d_currLenY_slopes /* array for Ind2 curr slopes*/
	, float* __restrict__ d_outResNew // Res2
	, float* __restrict__ d_outResOld // Res1
	, float* __restrict__ d_prevIdx0Score // pair_score(H[t], C[0]) for prev t (Res1)
	, int outLen, int K, float weight)
{
	extern __shared__ float sharedMem[]; // dynamically allocated shared memory
	float* currLenX_values_shared = sharedMem; // size: currPatternLen
	float* currLenX_slopes_shared = &sharedMem[K]; // size: currPatternLen
	float* currLenY_values_shared = &sharedMem[2 * K]; // size: currPatternLen
	float* currLenY_slopes_shared = &sharedMem[3 * K]; // size: currPatternLen
	float* currLow_HighValues_LenX_shared = &sharedMem[4 *  K];
	float* currLow_HighValues_LenY_shared = &sharedMem[(4 * K) + 2];

	// Load currMAs_values and currMAs_slopes into shared memory
	for (int i = threadIdx.x; i < K; i += blockDim.x) {
		currLenX_values_shared[i] = d_currLenX_values[i];
		currLenX_slopes_shared[i] = d_currLenX_slopes[i];
		currLenY_values_shared[i] = d_currLenY_values[i];
		currLenY_slopes_shared[i] = d_currLenY_slopes[i];
	}

	__syncthreads(); // Ensure all threads have loaded the data -- NOTE: you'll get intelisense error (undefined), but it compiles and works fine

	if (threadIdx.x == 0)
	{
		Find_MaxMinValues(currLenY_values_shared, K, currLow_HighValues_LenY_shared);
		Find_MaxMinValues(currLenX_values_shared, K, currLow_HighValues_LenX_shared);
	}
	__syncthreads(); // Ensure all threads have loaded the data -- NOTE: you'll get intelisense error (undefined), but it compiles and works fine

	int idx = blockIdx.x * blockDim.x + threadIdx.x;

	if (idx > outLen) return; // out of bounds check


	float histXv_K = d_histLenX_values[idx + K - 1];
	float histXs_K = d_histLenX_slopes[idx + K - 1];
	float histYv_K = d_histLenY_values[idx + K - 1];
	float histYs_K = d_histLenY_slopes[idx + K - 1];

	float currXv_K = currLenX_values_shared[K - 1];
	float currXs_K = currLenX_slopes_shared[K - 1];
	float currYv_K = currLenY_values_shared[K - 1];
	float currYs_K = currLenY_slopes_shared[K - 1];

	float histLow_HighValues_LenX[2];
	Find_MaxMinValues(d_histLenX_values + idx, K, histLow_HighValues_LenX);

	float histLow_HighValues_LenY[2];
	Find_MaxMinValues(d_histLenY_values + idx, K, histLow_HighValues_LenY);

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
	

	float newIdxKScore = weight *
		SingleCalc_Device_TwoInd(
		histXv_K,
		histXs_K,
		currXv_K,
		currXs_K,
		histYv_K,
		histYs_K,
		currYv_K,
		currYs_K,
		currLowest_LenX,
		currLowest_LenY,
		invCurrRange_LenX,
		invCurrRange_LenY,
		histLow_HighValues_LenX[0],
		histLow_HighValues_LenY[0],
		invHistRange_LenX,
		invHistRange_LenY
		);

	float prevIdx0Score = d_prevIdx0Score[idx - 1]; // pair_score(H[t], C[0]) for prev t
	float resOld_t = d_outResOld[idx - 1];
	d_outResNew[idx] = resOld_t - prevIdx0Score + newIdxKScore; // S[t] for new t

	d_prevIdx0Score[idx] =
		SingleCalc_Device_TwoInd(
		d_histLenX_values[idx],
		d_histLenX_slopes[idx],
		currLenX_values_shared[0],
		currLenX_slopes_shared[0],
		d_histLenY_values[idx],
		d_histLenY_slopes[idx],
		currLenY_values_shared[0],
		currLenY_slopes_shared[0],
		currLowest_LenX,
		currLowest_LenY,
		invCurrRange_LenX,
		invCurrRange_LenY,
		histLow_HighValues_LenX[0],
		histLow_HighValues_LenY[0],
		invHistRange_LenX,
		invHistRange_LenY
		); // store for t+1 in next iteration
}