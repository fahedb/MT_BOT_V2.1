#include "CPP_To_Cuda_Tunnel.h"
#include "KernelEngine_ScoreCalcs.h"

#include "CudaMemoryController.h"

#include "StreamSetters.h"

#include <iostream>
void CPP_To_Cuda_Tunnel::Calc_CandidateTwoIndicatorRatio_ScoreCalcs(
	const int* d_candidateIndices,
	const int candidateCount,
	float* d_results
	///////////////////// Hist len X arrays ////////////////////////////////////////////
	, const float* d_histIndX_Values, const float* d_histIndX_Slopes				////
	, const float* d_histIndX_MinValues, const float* d_histIndX_MaxValues			////
	, const float* d_histIndX_MinSlopes, const float* d_histIndX_MaxSlopes			////
	////////////////////////////////////////////////////////////////////////////////////

	/////////////////////// Current len X arrays //////////////////////////////////////
	, const float* d_currIndX_Values, const float* d_currIndX_Slopes				///
	, const float* d_currIndX_MinValues, const float* d_currIndX_MaxValues			///
	, const float* d_currIndX_MinSlopes, const float* d_currIndX_MaxSlopes			///
	///////////////////////////////////////////////////////////////////////////////////

	////////////////////// Hist len Y arrays //////////////////////////////////////////
	, const float* d_histIndY_Values, const float* d_histIndY_Slopes				///				
	, const float* d_histIndY_MinValues, const float* d_histIndY_MaxValues			///
	, const float* d_histIndY_MinSlopes, const float* d_histIndY_MaxSlopes			///
	///////////////////////////////////////////////////////////////////////////////////

	///////////////////// Current len Y arrays ////////////////////////////////////////
	, const float* d_currIndY_Values, const float* d_currIndY_Slopes				///			
	, const float* d_currIndY_MinValues, const float* d_currIndY_MaxValues			///
	, const float* d_currIndY_MinSlopes, const float* d_currIndY_MaxSlopes			///
	///////////////////////////////////////////////////////////////////////////////////

	, int currTimeLineLen, int numOfResults, int weight)
{
	if (!d_results
		|| !d_histIndX_Values || !d_histIndX_Slopes || !d_currIndX_Values || !d_currIndX_Slopes
		|| !d_histIndY_Values || !d_histIndY_Slopes || !d_currIndY_Values || !d_currIndY_Slopes)
	{
		std::cout << "Invalid input to Calc_TwoIndicatorsRatio_ScoreCalcs" << std::endl;
		return;
	}

	CandidateTwoIndCalc(
		d_candidateIndices, candidateCount,
		d_histIndX_Values, d_histIndX_Slopes,
		d_histIndX_MinValues, d_histIndX_MaxValues,
		d_histIndX_MinSlopes, d_histIndX_MaxSlopes,

		d_currIndX_Values, d_currIndX_Slopes,
		d_currIndX_MinValues, d_currIndX_MaxValues,
		d_currIndX_MinSlopes, d_currIndX_MaxSlopes,

		d_histIndY_Values, d_histIndY_Slopes,
		d_histIndY_MinValues, d_histIndY_MaxValues,
		d_histIndY_MinSlopes, d_histIndY_MaxSlopes,

		d_currIndY_Values, d_currIndY_Slopes,
		d_currIndY_MinValues, d_currIndY_MaxValues,
		d_currIndY_MinSlopes, d_currIndY_MaxSlopes,
		d_results,
		currTimeLineLen, numOfResults, weight
	);
}



void CPP_To_Cuda_Tunnel::Calc_TwoIndicatorsRatio_ScoreCalcs(float* d_results

	///////////////////// Hist len X arrays ////////////////////////////////////////////
	, const float* d_histIndX_Values, const float* d_histIndX_Slopes				////
	, const float* d_histIndX_MinValues, const float* d_histIndX_MaxValues			////
	, const float* d_histIndX_MinSlopes, const float* d_histIndX_MaxSlopes			////
	////////////////////////////////////////////////////////////////////////////////////

	/////////////////////// Current len X arrays //////////////////////////////////////
	, const float* d_currIndX_Values, const float* d_currIndX_Slopes				///
	, const float* d_currIndX_MinValues, const float* d_currIndX_MaxValues			///
	, const float* d_currIndX_MinSlopes, const float* d_currIndX_MaxSlopes			///
	///////////////////////////////////////////////////////////////////////////////////

	////////////////////// Hist len Y arrays //////////////////////////////////////////
	, const float* d_histIndY_Values, const float* d_histIndY_Slopes				///				
	, const float* d_histIndY_MinValues, const float* d_histIndY_MaxValues			///
	, const float* d_histIndY_MinSlopes, const float* d_histIndY_MaxSlopes			///
	///////////////////////////////////////////////////////////////////////////////////

	///////////////////// Current len Y arrays ////////////////////////////////////////
	, const float* d_currIndY_Values, const float* d_currIndY_Slopes				///			
	, const float* d_currIndY_MinValues, const float* d_currIndY_MaxValues			///
	, const float* d_currIndY_MinSlopes, const float* d_currIndY_MaxSlopes			///
	///////////////////////////////////////////////////////////////////////////////////

	, int currTimeLineLen, int numOfResults, int weight)
{
	if (!d_results
		|| !d_histIndX_Values || !d_histIndX_Slopes || !d_currIndX_Values || !d_currIndX_Slopes
		|| !d_histIndY_Values || !d_histIndY_Slopes || !d_currIndY_Values || !d_currIndY_Slopes)
	{
		std::cout << "Invalid input to Calc_TwoIndicatorsRatio_ScoreCalcs" << std::endl;
		return;
	}

	TwoIndCalc(d_histIndX_Values, d_histIndX_Slopes,
		d_histIndX_MinValues, d_histIndX_MaxValues,
		d_histIndX_MinSlopes, d_histIndX_MaxSlopes,

		d_currIndX_Values, d_currIndX_Slopes,
		d_currIndX_MinValues, d_currIndX_MaxValues,
		d_currIndX_MinSlopes, d_currIndX_MaxSlopes,

		d_histIndY_Values, d_histIndY_Slopes,
		d_histIndY_MinValues, d_histIndY_MaxValues,
		d_histIndY_MinSlopes, d_histIndY_MaxSlopes,

		d_currIndY_Values, d_currIndY_Slopes,
		d_currIndY_MinValues, d_currIndY_MaxValues,
		d_currIndY_MinSlopes, d_currIndY_MaxSlopes,
		d_results,
		currTimeLineLen, numOfResults, weight
	);
}

void CPP_To_Cuda_Tunnel::Calc_CandidateIndicator_ScoreCalcs(
	const int* d_candidateIndices,
	const int candidateCount,
	float* d_results
	//////////////////////////////////// Hist arrays //////////////////////////////////////////////////
	, const float* d_histValues, const float* d_histSlopes											///
	, const float* d_hist_MinValues, const float* d_hist_MaxValues									///
	, const float* d_hist_MinSlopes, const float* d_hist_MaxSlopes									///
	///////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////// Current arrays ///////////////////////////////////////////////
	, const float* d_currValues, const float* d_currSlopes											///
	, const float* d_curr_MinValues, const float* d_curr_MaxValues									///
	, const float* d_curr_MinSlopes, const float* d_curr_MaxSlopes									///
	///////////////////////////////////////////////////////////////////////////////////////////////////
	, int currTimeLineLen, int numOfResults, IndicatorType indT, int weight)
{
	if(!d_results || numOfResults <= 0 || !d_candidateIndices || candidateCount <= 0)
	{
		std::cout << "Invalid input to Calc_CandidateIndicator_ScoreCalcs" << std::endl;
		return;
	}
	if (indT == IndicatorType::EMA || indT == IndicatorType::ZLSMA)
	{
		CandidateCalcMA(d_candidateIndices, candidateCount,
			d_histValues, d_histSlopes,
			d_hist_MinValues, d_hist_MaxValues,
			d_hist_MinSlopes, d_hist_MaxSlopes,
			d_currValues, d_currSlopes,
			d_curr_MinValues, d_curr_MaxValues,
			d_curr_MinSlopes, d_curr_MaxSlopes,
			d_results, currTimeLineLen, numOfResults, weight);
	}
	else if (indT == IndicatorType::RSI)
	{
		CandidateCalcRSI(d_candidateIndices, candidateCount,
			d_histValues, d_histSlopes,
			d_currValues, d_currSlopes,
			d_results, currTimeLineLen, numOfResults, weight);
	}
}

void CPP_To_Cuda_Tunnel::Calc_Indicator_ScoreCalcs(float* d_results
	//////////////////////////////////// Hist arrays //////////////////////////////////////////////////
	, const float* d_histValues, const float* d_histSlopes											///
	, const float* d_hist_MinValues, const float* d_hist_MaxValues									///
	, const float* d_hist_MinSlopes, const float* d_hist_MaxSlopes									///
	///////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////// Current arrays ///////////////////////////////////////////////
	, const float* d_currValues, const float* d_currSlopes											///
	, const float* d_curr_MinValues, const float* d_curr_MaxValues									///
	, const float* d_curr_MinSlopes, const float* d_curr_MaxSlopes									///
	///////////////////////////////////////////////////////////////////////////////////////////////////
	, int currTimeLineLen, int numOfResults, IndicatorType indT, int weight)
{
	if (!d_results || numOfResults <= 0)
	{
		std::cout << "Invalid input to Calc_Indicator_ScoreCalcs" << std::endl;
		return;
	}

	if(indT == IndicatorType::EMA || indT == IndicatorType::ZLSMA)
	{
		// Call EMA scoring function
		calcMA(d_histValues, d_histSlopes,
			d_hist_MinValues, d_hist_MaxValues,
			d_hist_MinSlopes, d_hist_MaxSlopes,

			d_currValues, d_currSlopes,
			d_curr_MinValues, d_curr_MaxValues,
			d_curr_MinSlopes, d_curr_MaxSlopes,
			d_results, currTimeLineLen, numOfResults, weight);

	}
	else if(indT == IndicatorType::RSI)
	{
		calcRSI(d_histValues, d_histSlopes,
			d_currValues, d_currSlopes,
			d_results, currTimeLineLen, numOfResults, weight);
		
	}
}


bool CPP_To_Cuda_Tunnel::IsCudaAvailable()
{
	int deviceCount = 0;
	cudaError_t error = cudaGetDeviceCount(&deviceCount);

	if (error != cudaSuccess) {
		std::cout << "\nCUDA runtime error: " << cudaGetErrorString(error) << std::endl;
		return false;
	}

	if (deviceCount == 0) {
		std::cout << "\nNo CUDA-compatible GPU found." << std::endl;
		return false;
	}
	else {
		std::cout << "\nFound " << deviceCount << " CUDA-compatible device(s)." << std::endl;

		for (int i = 0; i < deviceCount; ++i) {
			cudaDeviceProp prop;
			cudaGetDeviceProperties(&prop, i);
			std::cout << "\nDevice " << i << ": " << prop.name
				<< " (Compute Capability " << prop.major << "." << prop.minor << ")\n";
		}
		return true;
	}

}

void CPP_To_Cuda_Tunnel::SyncCUDADevice()
{
	SyncDevice();
}
