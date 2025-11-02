#include "KernelEngine_ScoreCalcs.h"

#include "KernelEngine_ScoreCalcs_RSI.h"
#include "KernelEngine_ScoreCalcs_MA.h"
#include "KernelEngine_ScoreCalcs_TwoInd.h"

#include <chrono>
// fabsf,... etc include
#include <math.h>

#include "CudaMemoryController.h"

//#define FULL_CUDA_SYNC_CHECKS

#include <iostream>
#define THREADS_NUM 256
#define TILE_SIZE 512
#include <string>

void CheckCudaLanuchError(const std::string& kernelName, int blocks, int threads, int shBytes, int currPatternLen, int histSize ) {
	cudaError_t launchErr = cudaGetLastError();
	if (launchErr != cudaSuccess) {
		std::cout << "Kernel: " << kernelName << "\n"
			<< "Kernel launch error: " << cudaGetErrorString(launchErr) << "\n"
			<< " blocks=" << blocks << "\n"
			<< " threads=" << threads << "\n"
			<< " shBytes=" << shBytes << "\n"
			<< " patternLen=" << currPatternLen << "\n"
			<< " histSize=" << histSize
			<< std::endl;
		return;
	}
#ifdef FULL_CUDA_SYNC_CHECKS
	cudaError_t syncErr = cudaDeviceSynchronize();
	if (syncErr != cudaSuccess) {
		std::cout <<"Kernek Name: "<< kernelName << "\nKernel runtime error: " << cudaGetErrorString(syncErr) << std::endl;
		return;
	}
#endif
}

////////////////////////////////////////////////////////////////////////////////
// Two Ind lanunchers: /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Tiled Two Ind score kernel launcher
void TwoIndCalc_tiled(const float* d_hist_LenX_Val, const float* d_hist_LenX_Slope,
	const float* d_curr_LenX_Val, const float* d_curr_LenX_Slope,
	const float* d_hist_LenY_Val, const float* d_hist_LenY_Slope,
	const float* d_curr_LenY_Val, const float* d_curr_LenY_Slope,
	float* d_results, float* d_prevIdx0Score,
	int K, int histSize, float weight)
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

	TiledScan_Kernel_TwoInd << <blocks, threads, shBytes >> > (
		d_hist_LenX_Val, d_hist_LenX_Slope,
		d_curr_LenX_Val, d_curr_LenX_Slope,
		d_hist_LenY_Val, d_hist_LenY_Slope,
		d_curr_LenY_Val, d_curr_LenY_Slope,
		d_results, d_prevIdx0Score, K, starts, weight, TILE);

	CheckCudaLanuchError("TwoIndCalc_tiled", blocks, threads, shBytes, K, histSize);

#ifndef NDEBUG
	cudaError_t err = cudaGetLastError();
	if (err != cudaSuccess) {
		printf("TwoIndScoreCalcKernel_Tiled launch error: %s\n", cudaGetErrorString(err));
	}
	cudaDeviceSynchronize();
#endif
}

// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
// Full Two Ind score kernel launcher
void TwoIndCalc(float* d_hist_LenX_Val, float* d_hist_LenX_Slope,
				float* d_curr_LenX_Val, float* d_curr_LenX_Slope,
				float* d_hist_LenY_Val, float* d_hist_LenY_Slope,
				float* d_curr_LenY_Val, float* d_curr_LenY_Slope,
				float* d_results, float* d_prevIdx0Score, int patternLen, int histSize, float weight)
{
	const int starts = histSize - patternLen + 1;
	const int threads = THREADS_NUM;
	const int blocks = (starts + threads - 1) / threads;
	const size_t shBytes = sizeof(float) * ((4 * patternLen) + 4);
	
	FullScan_Kernel_TwoInd << <blocks, threads, shBytes >> > (
		d_hist_LenX_Val, d_hist_LenX_Slope,
		d_curr_LenX_Val, d_curr_LenX_Slope,
		d_hist_LenY_Val, d_hist_LenY_Slope,
		d_curr_LenY_Val, d_curr_LenY_Slope,
		d_results, d_prevIdx0Score,
		patternLen,
		starts,
		weight
		);
	CheckCudaLanuchError("TwoIndCalc", blocks, threads, shBytes, patternLen, histSize);

}

void TwoIndCalc_SlidingWindow(
	const float* d_hist_LenX_Val, const float* d_hist_LenX_Slope,
	const float* d_curr_LenX_Val, const float* d_curr_LenX_Slope,
	const float* d_hist_LenY_Val, const float* d_hist_LenY_Slope,
	const float* d_curr_LenY_Val, const float* d_curr_LenY_Slope,
	float* d_newResults, float* d_oldResults, float* d_prevIdx0Score,
	int patternLen, int histSize, float weight)
{
	const int starts = histSize - patternLen + 1;   // number of starting points in hist where pattern can fit
	const int threads = THREADS_NUM;                         // 128/256, depending on GPU
	const int blocks = (starts + threads - 1) / threads;
	const size_t shBytes = sizeof(float) * ((4 * patternLen) + 4); // curr values+slopes for LenX and LenY
	SlideCalc_Kernel_TwoInd << <blocks, threads, shBytes >> > (
		d_hist_LenX_Val, d_hist_LenX_Slope,
		d_curr_LenX_Val, d_curr_LenX_Slope,
		d_hist_LenY_Val, d_hist_LenY_Slope,
		d_curr_LenY_Val, d_curr_LenY_Slope,
		d_newResults, d_oldResults, d_prevIdx0Score,
		patternLen,
		starts,
		weight
		);
	CheckCudaLanuchError("TwoIndCalc_SlidingWindow", blocks, threads, shBytes, patternLen, histSize);

}

// ---------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////
// MA lanunchers: /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

// Tiled MA score kernel launcher
void calcMA_tiled(const float* d_histMA_values,
	const float* d_histMA_slopes,
	const float* d_currMA_values,
	const float* d_currMA_slopes,
	float* d_results, float* d_prevIdx0Score,
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

	TiledScan_Kernel_MA << <blocks, threads, shBytes >> > (
		d_histMA_values, d_histMA_slopes,
		d_currMA_values, d_currMA_slopes,
		d_results, d_prevIdx0Score,
		K, starts, weight, TILE);
	CheckCudaLanuchError("calcMA_tiled", blocks, threads, shBytes, patternLen, histSize);

#ifndef NDEBUG
	cudaError_t err = cudaGetLastError();
	if (err != cudaSuccess) {
		printf("MAScoreCalcKernel_Tiled launch error: %s\n", cudaGetErrorString(err));
	}
	cudaDeviceSynchronize();
#endif
}

// ----------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------

// Full MA score kernel launcher
void calcMA(float* d_histMA_values, float* d_histMA_slopes,
			float* d_currMA_values, float* d_currMA_slopes, 
			float* d_results, float* d_prevIdx0Score, const float* d_oldScores,
			int patternLen, int histSize, float weight)
{
	const int starts = histSize - patternLen + 1;
	const int threads = THREADS_NUM;
	const int blocks = (starts + threads - 1) / threads;
	const size_t shBytes = sizeof(float) * ((2 * patternLen) + 2);
	FullScan_Kernel_MA << <blocks, threads, shBytes >> > (
		d_histMA_values,
		d_histMA_slopes,
		d_currMA_values,
		d_currMA_slopes,
		d_results ,d_prevIdx0Score ,
		patternLen,
		starts,
		weight
		);
	CheckCudaLanuchError("calcMA", blocks, threads, shBytes, patternLen, histSize);
}

void calcMA_SlidingWindow(
	const float* d_histMA_values, const float* d_histMA_slopes,
	const float* d_currMA_values, const float* d_currMA_slopes,
	float* d_newResults, float* d_oldResults, float* d_prevIdx0Score,
	int patternLen, int histSize, float weight)
{
	const int starts = histSize - patternLen + 1;   // number of starting points in hist where pattern can fit
	const int threads = THREADS_NUM;                         // 128/256, depending on GPU
	const int blocks = (starts + threads - 1) / threads;
	const size_t shBytes = sizeof(float) * (2 * patternLen); // curr values+slopes
	SlideScan_Kernel_MA << <blocks, threads, shBytes >> > (
		d_histMA_values,
		d_histMA_slopes,
		d_currMA_values,
		d_currMA_slopes,
		d_newResults, d_oldResults, d_prevIdx0Score,
		patternLen,
		starts,
		weight
		);
	CheckCudaLanuchError("calcMA_SlidingWindow", blocks, threads, shBytes, patternLen, histSize);
}
// ----------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////
//////////////// RSI launchers: //////////////////////////////////
//////////////////////////////////////////////////////////////////

// Full RSI score kernel launcher
void calcRSI(
	float* d_histRSI_values, float* d_histRSI_slopes, 
	float* d_currRSI_values, float* d_currRSI_slopes,
	float* d_results, float* d_prevIdx0Score,
	int patternLen, int histSize, float weight)
{
	const int starts = histSize - patternLen + 1;   // number of starting points in hist where pattern can fit
	const int threads = THREADS_NUM;                         // 128/256, depending on GPU
	const int blocks = (starts + threads - 1) / threads;
	const size_t shBytes = sizeof(float) * (2 * patternLen); // curr values+slopes

	FullScan_Kernel_RSI << <blocks, threads, shBytes >> > (
		d_histRSI_values,
		d_histRSI_slopes,
		d_currRSI_values,
		d_currRSI_slopes,
		d_results, d_prevIdx0Score,
		patternLen,
		starts,
		weight
		);
	CheckCudaLanuchError("calcRSI", blocks, threads, shBytes, patternLen, histSize);

	// Log any errors from launch if not in release mode
#ifndef NDEBUG
	cudaError_t err = cudaGetLastError();
	if (err != cudaSuccess) {
		printf("RSIScoreCalcKernel launch error: %s\n", cudaGetErrorString(err));
	}
	cudaDeviceSynchronize();
#endif
}
// ---------------------------------------------------------------------

void calcRSI_SlidingWindow(
	const float* d_histRSI_values, const float* d_histRSI_slopes,
	const float* d_currRSI_values, const float* d_currRSI_slopes,
	float* d_newResults, float* d_oldResults, float* d_prevIdx0Score,
	int patternLen, int histSize, float weight)
{
	const int starts = histSize - patternLen + 1;   // number of starting points in hist where pattern can fit
	const int threads = THREADS_NUM;                         // 128/256, depending on GPU
	const int blocks = (starts + threads - 1) / threads;
	const size_t shBytes = sizeof(float) * (2 * patternLen); // curr values+slopes

	SlideScan_Kernel_RSI << <blocks, threads, shBytes >> > (
		d_histRSI_values,
		d_histRSI_slopes,
		d_currRSI_values,
		d_currRSI_slopes,
		d_newResults, d_oldResults, d_prevIdx0Score,
		patternLen,
		starts,
		weight
		);
	CheckCudaLanuchError("calcRSI_SlidingWindow", blocks, threads, shBytes, patternLen, histSize);
			
			// Log any errors from launch if not in release mode
#ifndef NDEBUG
			cudaError_t err = cudaGetLastError();
			if (err != cudaSuccess) {
				printf("RSIScoreCalcKernel launch error: %s\n", cudaGetErrorString(err));
			}
			cudaDeviceSynchronize();
#endif
}

// ----------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------

// Tiled RSI score kernel launcher
void calcRSI_tiled(const float* d_histVal, const float* d_histSlope,
	const float* d_currVal, const float* d_currSlope,
	float* d_resOut, float* d_prevIdx0Score, int currPatternLen, int histSize, float weight)
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

	TiledScan_Kernel_RSI << <blocks, threads, shBytes >> > (
		d_histVal, d_histSlope, d_currVal, d_currSlope,
		d_resOut, d_prevIdx0Score, currPatternLen, starts, weight, TILE);
	CheckCudaLanuchError("calcRSI_tiled", blocks, threads, shBytes, currPatternLen, histSize);

#ifndef NDEBUG
	cudaError_t err = cudaGetLastError();
	if (err != cudaSuccess) {
		printf("CorrTileKernel launch error: %s\n", cudaGetErrorString(err));
	}
	cudaDeviceSynchronize();
#endif
}