//#include "KernelEngine_ScoreCalcs_RSI.h"
#include "KernelEngine_ScoreCalcs_General.h"

// Full Pattern Scan Kernel for RSI
// Run time: O(K) per t, or O(N*K) for all t in [0, outLen-1]
__device__ __forceinline__
void FullPatternScanCalc_Device_RSI(
	const float* __restrict__ d_histRSI_values /*array for rsi hist values */
	, const float* __restrict__ d_histRSI_slopes /*array for rsi hist slopes */
	, const float* __restrict__ d_currRSI_values /* array for rsi curr values*/
	, const float* __restrict__ d_currRSI_slopes /* array for rsi curr slopes*/
	, float* d_scores /*output scores, where Score(idx) is the score of currRSI(t)&&histRSI(idx + t), t is the index of the current candle */
	, const int currPatternLen, const int dataSize, int t, const float weight)
{
	float score = 0.0f;
#pragma unroll 64 // assuming max pattern length is 30-64, adjust as needed
	for (int i = 0; i < currPatternLen; i++)
	{
		const float histValue = d_histRSI_values[t + i];
		const float histSlope = d_histRSI_slopes[t + i];
		const float currValue = d_currRSI_values[i]; // from shared memory
		const float currSlope = d_currRSI_slopes[i]; // from shared memory
		score += pair_score(histValue, currValue, histSlope, currSlope);
	}
	d_scores[t] += score * weight; // final score for this position
}

// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------


// RSI score calcs
// Full Scan Kernel
__global__ void FullScan_Kernel_RSI(const float* __restrict__ d_histRSI_values /*array for rsi hist values */
	, const float* __restrict__ d_histRSI_slopes /*array for rsi hist slopes */
	, const float* __restrict__ d_currRSI_values /* array for rsi curr values*/
	, const float* __restrict__ d_currRSI_slopes /* array for rsi curr slopes*/
	, float* d_scores /*output scores, where Score(idx) is the score of currRSI(t)&&histRSI(idx + t), t is the index of the current candle */
	, const int currPatternLen, const int outLen, const float weight)
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

	int lastStart = outLen;
	if (idx >= lastStart) return; // out of bounds check

	FullPatternScanCalc_Device_RSI(
		d_histRSI_values,
		d_histRSI_slopes,
		currRSI_values_shared,
		currRSI_slopes_shared,
		d_scores,
		currPatternLen, lastStart, idx, weight);
}

