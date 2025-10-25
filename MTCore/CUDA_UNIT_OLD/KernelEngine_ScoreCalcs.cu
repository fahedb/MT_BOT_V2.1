#include "KernelEngine_ScoreCalcs.h"
#include <chrono>
// fabsf,... etc include
#include <math.h>

#include <cuda_runtime.h>
#include "device_launch_parameters.h"
#include <cuda_runtime_api.h>
#include <iostream>
#define THREADS_NUM 256

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
		score += RelativeScoreCalc(histValue, currValue);
		score += RelativeScoreCalc(histSlope, currSlope);
	}
	d_scores[idx] = score * weight; // final score for this position


}


// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

// MAs score calcs
__device__ void Find_MaxMinValues(const float* __restrict__ d_values, int d_size, float* results)
{
	// find min max and init in results[0] and results[1]
	float min = FLT_MAX;
	float max = -FLT_MAX;
	for (int i = 0; i < d_size; i++)
	{
		float val = d_values[i];
		if (val < min) min = val;
		if (val > max) max = val;
	}
	results[0] = min;
	results[1] = max;
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
		d_scores[idx] = 0.0f;
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
	d_scores[idx] = score * weight; // final score for this position
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
									, const int currPatternLen, const int dataSize, const float weight)
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
	int lastStart = dataSize - currPatternLen;
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
	d_scores[idx] = score * weight; // final score for this position
}




// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////
///////// Kernels callers ////////////////////////////////////
//////////////////////////////////////////////////////////////
void TwoIndCalc(float* d_hist_LenX_Val, float* d_hist_LenX_Slope,
				float* d_curr_LenX_Val, float* d_curr_LenX_Slope,
				float* d_hist_LenY_Val, float* d_hist_LenY_Slope,
				float* d_curr_LenY_Val, float* d_curr_LenY_Slope,
				float* d_results, int patternLen, int histSize, float weight)
{
	const int starts = histSize - patternLen + 1;
	const int threads = THREADS_NUM;
	const int blocks = (starts + threads - 1) / threads;
	const size_t shBytes = sizeof(float) * ((2 * patternLen) + 4);
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
