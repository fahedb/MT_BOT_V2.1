//#include "KernelEngine_ScoreCalcs_RSI.h"
#include "KernelEngine_ScoreCalcs_General.h"

// Slide kernel helper 
// The math behind this function is:
// Let's assume the Res1 is the Result Array for scan 1, and Res2 is for scan 2
// So if Res1[t] = Sum(i=0 to K-1) pair_score(H[t+i], C[i]), 
// Then we insert a new data to C, So we can calc Res2[t+1] = Sum(i=1 to K) pair_score(H[t+i], C[i])
// As Res2[t+1] = Res1[t] - pair_score(H[t], C[0]) + pair_score(H[t+K], C[K])
// Run time: O(1) per t, or O(N) for all t in [0, outLen-1]
__device__ __forceinline__ void SlideCalc_Device_RSI(
	const float* __restrict__ d_histVal,
	const float* __restrict__ d_histSlope,
	const float* __restrict__ d_currVal,
	const float* __restrict__ d_currSlope,
	float* __restrict__ d_outResNew, // S[t]
	float* __restrict__ d_outResOld, // S[t] for prev t (Res1)
	float* __restrict__ d_prevIdx0Score, // pair_score(H[t], C[0]) for prev t (Res1)
	int outLen, int K, int t, float weight)
{
	float prevIdx0Score = d_prevIdx0Score[t - 1];

	float histVal_K = d_histVal[t + K - 1];
	float histSlope_K = d_histSlope[t + K - 1];
	float currVal_K = d_currVal[K - 1];
	float currSlope_K = d_currSlope[K - 1];

	float histVal_0 = d_histVal[t];
	float histSlope_0 = d_histSlope[t];
	float currVal_0 = d_currVal[0];
	float currSlope_0 = d_currSlope[0];

	float resOld_t = d_outResOld[t - 1];


	float newIdxKScore = pair_score(histVal_K, currVal_K, histSlope_K, currSlope_K) * weight;

	d_outResNew[t] = resOld_t - prevIdx0Score + newIdxKScore; // S[t] for new t

	d_prevIdx0Score[t] = pair_score(histVal_0, currVal_0, histSlope_0, currSlope_0) * weight; // store for t+1 in next iteration

}

// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------

// Full Pattern Scan Kernel for RSI
// Run time: O(K) per t, or O(N*K) for all t in [0, outLen-1]
__device__ __forceinline__
void FullPatternScanCalc_Device_RSI(
	const float* __restrict__ d_histRSI_values /*array for rsi hist values */
	, const float* __restrict__ d_histRSI_slopes /*array for rsi hist slopes */
	, const float* __restrict__ d_currRSI_values /* array for rsi curr values*/
	, const float* __restrict__ d_currRSI_slopes /* array for rsi curr slopes*/
	, float* d_scores /*output scores, where Score(idx) is the score of currRSI(t)&&histRSI(idx + t), t is the index of the current candle */
	, float* __restrict__ d_prevIdx0Score /*previous score for idx 0, used in sliding window*/
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
		if(i==0)
		{
			// store the idx 0 score for sliding window use
			d_prevIdx0Score[t] = score * weight;
		}
	}
	d_scores[t] += score * weight; // final score for this position
}


// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------


// Let's assume the Res1 is the Result Array for scan 1, and Res2 is for scan 2
// So if Res1[t] = Sum(i=0 to K-1) pair_score(H[t+i], C[i]), 
// Then we insert a new data to C, So we can calc Res2[t+1] = Sum(i=1 to K) pair_score(H[t+i], C[i])
// As Res2[t+1] = Res1[t] - pair_score(H[t], C[0]) + pair_score(H[t+K], C[K])
__global__ void SlideScan_Kernel_RSI(
	const float* __restrict__ d_histVal,
	const float* __restrict__ d_histSlope,
	const float* __restrict__ d_currVal,
	const float* __restrict__ d_currSlope,
	float* __restrict__ d_outResNew, // S[t]
	float* __restrict__ d_outResOld, // S[t] for prev t (Res1)
	float* __restrict__ d_prevIdx0Score, // pair_score(H[t], C[0]) for prev t (Res1)
	int outLen, int K, float weight)
{
	int t = blockIdx.x * blockDim.x + threadIdx.x;
	if (t >= outLen) return;
	if (t == 0)
	{
		// Full scan for t=0
		FullPatternScanCalc_Device_RSI(
			d_histVal,
			d_histSlope,
			d_currVal,
			d_currSlope,
			d_outResNew, d_prevIdx0Score,
			K, outLen, t, weight);
	}
	else {
		// Slide calc for t>0
		SlideCalc_Device_RSI(
			d_histVal,
			d_histSlope,
			d_currVal,
			d_currSlope,
			d_outResNew,
			d_outResOld,
			d_prevIdx0Score,
			outLen, K, t, weight);
	}
}

// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------

// Tiled RSI score kernel
__global__ void TiledScan_Kernel_RSI(
	const float* __restrict__ d_histVal,
	const float* __restrict__ d_histSlope,
	const float* __restrict__ d_currVal,
	const float* __restrict__ d_currSlope,
	float* __restrict__ d_out, // S[t]
	float* __restrict__ d_prevIdx0Score, /*previous score for idx 0, used in sliding window*/
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
			if(k==0)
			{
				// we can store d_prevIdx0Score if needed for sliding window later
				d_prevIdx0Score[base + tLocal] = sum * weight;
			}
		}
		d_out[base + tLocal] += sum * weight;
	}
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
	, float* __restrict__ d_prevIdx0Score /*previous score for idx 0, used in sliding window*/
	, const int currPatternLen, const int dataSize, const float weight)
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

	FullPatternScanCalc_Device_RSI(
		d_histRSI_values,
		d_histRSI_slopes,
		currRSI_values_shared,
		currRSI_slopes_shared,
		d_scores, d_prevIdx0Score,
		currPatternLen, dataSize, idx, weight);
}

