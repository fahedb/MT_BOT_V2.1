#include "KernelEngine_ScoreCalcs.h"

#include "KernelEngine_ScoreCalcs_RSI.h"
#include "KernelEngine_ScoreCalcs_MA.h"
#include "KernelEngine_ScoreCalcs_TwoInd.h"

#include <chrono>
// fabsf,... etc include
#include <math.h>

#include "CudaMemoryController.h"
#include "GPU_LaunchesParameters.h"

//#define FULL_CUDA_SYNC_CHECKS

#include <iostream>
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

// Full Two Ind score kernel launcher
void TwoIndCalc(
	//////////////////////////////////// Hist len X arrays /////////////////////////////////////////////
	const float* d_histIndX_values, const float* d_histIndX_slopes							////
	, const float* d_histIndX_MinValues, const float* d_histIndX_MaxValues						////
	, const float* d_histIndX_MinSlopes, const float* d_histIndX_MaxSlopes						////
	////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////// Current len X arrays //////////////////////////////////////////
	, const float* d_currIndX_values, const float* d_currIndX_slopes							////
	, const float* d_currIndX_MinValues, const float* d_currIndX_MaxValues						////
	, const float* d_currIndX_MinSlopes, const float* d_currIndX_MaxSlopes						////
	////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////// Hist len Y arrays /////////////////////////////////////////////
	, const float* d_histIndY_values, const float* d_histIndY_slopes								////
	, const float* d_histIndY_MinValues, const float* d_histIndY_MaxValues						////
	, const float* d_histIndY_MinSlopes, const float* d_histIndY_MaxSlopes						////
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////// Current len Y arrays //////////////////////////////////////////
	, const float* d_currIndY_values, const float* d_currIndY_slopes								////
	, const float* d_currIndY_MinValues, const float* d_currIndY_MaxValues						////
	, const float* d_currIndY_MinSlopes, const float* d_currIndY_MaxSlopes						////
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	, float* d_results,
	int patternLen, int histSize, float weight)
{
	const int starts = histSize - patternLen + 1;
	const int threads = GPU_THREADS_NUM;
	const int blocks = (starts + threads - 1) / threads;
	size_t shBytes = sizeof(float) * (4 * patternLen); // for curr vals and slopes caching
	int histTileLen = threads + patternLen - 1;
	shBytes += sizeof(float) * (histTileLen) * 4; // for hist vals and slopes caching
	shBytes += 4 * sizeof(float); // for min/max caching

	FullScan_Kernel_TwoInd << <blocks, threads, shBytes >> > (
		d_histIndX_values, d_histIndX_slopes,
		d_histIndX_MinValues, d_histIndX_MaxValues,
		d_histIndX_MinSlopes, d_histIndX_MaxSlopes,

		d_currIndX_values, d_currIndX_slopes,
		d_currIndX_MinValues, d_currIndX_MaxValues,
		d_currIndX_MinSlopes, d_currIndX_MaxSlopes,
		
		d_histIndY_values, d_histIndY_slopes,
		d_histIndY_MinValues, d_histIndY_MaxValues,
		d_histIndY_MinSlopes, d_histIndY_MaxSlopes,
		
		d_currIndY_values, d_currIndY_slopes,
		d_currIndY_MinValues, d_currIndY_MaxValues,
		d_currIndY_MinSlopes, d_currIndY_MaxSlopes,
		
		d_results,
		patternLen,
		starts,
		weight
		);
	CheckCudaLanuchError("TwoIndCalc", blocks, threads, shBytes, patternLen, histSize);
}

// Candidate Two Ind score kernel launcher
void CandidateTwoIndCalc(
	const int* d_candidateIndices,
	const int candidateCount,
	//////////////////////////////////// Hist len X arrays /////////////////////////////////////////////
	const float* d_histIndX_values, const float* d_histIndX_slopes							////
	, const float* d_histIndX_MinValues, const float* d_histIndX_MaxValues						////
	, const float* d_histIndX_MinSlopes, const float* d_histIndX_MaxSlopes						////
	////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////// Current len X arrays //////////////////////////////////////////
	, const float* d_currIndX_values, const float* d_currIndX_slopes							////
	, const float* d_currIndX_MinValues, const float* d_currIndX_MaxValues						////
	, const float* d_currIndX_MinSlopes, const float* d_currIndX_MaxSlopes						////
	////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////// Hist len Y arrays /////////////////////////////////////////////
	, const float* d_histIndY_values, const float* d_histIndY_slopes								////
	, const float* d_histIndY_MinValues, const float* d_histIndY_MaxValues						////
	, const float* d_histIndY_MinSlopes, const float* d_histIndY_MaxSlopes						////
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////// Current len Y arrays //////////////////////////////////////////
	, const float* d_currIndY_values, const float* d_currIndY_slopes								////
	, const float* d_currIndY_MinValues, const float* d_currIndY_MaxValues						////
	, const float* d_currIndY_MinSlopes, const float* d_currIndY_MaxSlopes						////
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	, float* d_results,
	int patternLen, int histSize, float weight)
{
	const int starts = histSize - patternLen + 1;
	const int threads = GPU_THREADS_NUM;
	const int blocks = (starts + threads - 1) / threads;
	size_t shBytes = sizeof(float) * (4 * patternLen); // for curr vals and slopes caching
	int histTileLen = threads + patternLen - 1;
	shBytes += sizeof(float) * (histTileLen) * 4; // for hist vals and slopes caching
	shBytes += 4 * sizeof(float); // for min/max caching

	CandidateScan_Kernel_TwoInd << <blocks, threads, shBytes >> > (
		d_candidateIndices, candidateCount,
		d_histIndX_values, d_histIndX_slopes,
		d_histIndX_MinValues, d_histIndX_MaxValues,
		d_histIndX_MinSlopes, d_histIndX_MaxSlopes,

		d_currIndX_values, d_currIndX_slopes,
		d_currIndX_MinValues, d_currIndX_MaxValues,
		d_currIndX_MinSlopes, d_currIndX_MaxSlopes,

		d_histIndY_values, d_histIndY_slopes,
		d_histIndY_MinValues, d_histIndY_MaxValues,
		d_histIndY_MinSlopes, d_histIndY_MaxSlopes,

		d_currIndY_values, d_currIndY_slopes,
		d_currIndY_MinValues, d_currIndY_MaxValues,
		d_currIndY_MinSlopes, d_currIndY_MaxSlopes,

		d_results,
		patternLen,
		starts,
		weight
		);
	CheckCudaLanuchError("TwoIndCalc", blocks, threads, shBytes, patternLen, histSize);
}


// ----------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------

// Full MA score kernel launcher
void calcMA(
	/////////////////////////////////// Hist MA arrays /////////////////////////////////////////////
	const float* d_histMA_values, const float* d_histMA_slopes,									////
	////
	const float* d_histMA_MinValues, const float* d_histMA_MaxValues,							////
	const float* d_histMA_MinSlopes, const float* d_histMA_MaxSlopes,							////
	////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////// Current MA arrays //////////////////////////////////////////
	const float* d_currMA_values, const float* d_currMA_slopes,									////
	////
	const float* d_currMA_MinValues, const float* d_currMA_MaxValues,							////
	const float* d_currMA_MinSlopes, const float* d_currMA_MaxSlopes,							////
	////////////////////////////////////////////////////////////////////////////////////////////////

	float* d_results,
	int patternLen, int histSize, float weight)
{
	const int starts = histSize - patternLen + 1;
	const int threads = GPU_THREADS_NUM;
	const int blocks = (starts + threads - 1) / threads;
	size_t shBytes = sizeof(float) * (2 * patternLen); // curr values+slopes
	shBytes += sizeof(float) * (threads + patternLen - 1) * 2; // hist values+slopes
	shBytes += 2 * sizeof(float); // min/max caching

	FullScan_Kernel_MA << <blocks, threads, shBytes >> > (
		d_histMA_values, d_histMA_slopes,
		d_histMA_MinValues, d_histMA_MaxValues,
		d_histMA_MinSlopes, d_histMA_MaxSlopes,

		d_currMA_values, d_currMA_slopes,
		d_currMA_MinValues, d_currMA_MaxValues,
		d_currMA_MinSlopes, d_currMA_MaxSlopes,

		d_results,
		patternLen,
		starts,
		weight
		);
	CheckCudaLanuchError("calcMA", blocks, threads, shBytes, patternLen, histSize);
}

// ----------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------

// Full RSI score kernel launcher
void calcRSI(
	const float* d_histRSI_values, const float* d_histRSI_slopes,
	const float* d_currRSI_values, const float* d_currRSI_slopes,
	float* d_results,
	int patternLen, int histSize, float weight)
{
	const int starts = histSize - patternLen + 1;   // number of starting points in hist where pattern can fit
	const int threads = GPU_THREADS_NUM;                         // 128/256, depending on GPU
	const int blocks = (starts + threads - 1) / threads;
	size_t shBytes = sizeof(float) * (2 * patternLen); // curr values+slopes
	int histTileLen = threads + patternLen - 1;
	shBytes += sizeof(float) * (histTileLen) * 2; // hist values+slopes

	FullScan_Kernel_RSI << <blocks, threads, shBytes >> > (
		d_histRSI_values,
		d_histRSI_slopes,
		d_currRSI_values,
		d_currRSI_slopes,
		d_results,
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

