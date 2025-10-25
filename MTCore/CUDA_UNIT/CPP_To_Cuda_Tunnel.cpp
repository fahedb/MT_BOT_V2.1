#include "CPP_To_Cuda_Tunnel.h"
#include "KernelEngine.h"
#include "KernelEngine_ScoreCalcs.h"

#include "CudaMemoryController.h"

#include "StreamSetters.h"

#include <iostream>

#define USE_TILED_CALCS_SCORECALCS 1

dataStream_t* g_HistDataStream = nullptr;
dataStream_t* g_CurrDataStream = nullptr;

float* g_totalScores_D = nullptr;
int g_totalScoresSize_D = 0;

void CPP_To_Cuda_Tunnel::Set_TotalScore_D_Memory(size_t size)
{
	CudaMemoryAllocation(&g_totalScores_D, size);

	g_totalScoresSize_D = size;
	cudaMemset(g_totalScores_D, 0, sizeof(float) * size);
}
void CPP_To_Cuda_Tunnel::Reset_TotalScores_D_To0()
{
	if (g_totalScores_D && g_totalScoresSize_D > 0)
	{
		cudaMemset(g_totalScores_D, 0, sizeof(float) * g_totalScoresSize_D);
	}
}


void CPP_To_Cuda_Tunnel::Calc_TwoIndicatorsRatio_ScoreCalcs(float* h_outScores, int numOfResults, IndicatorType indT, IndicatorLen indL1, BarPeriod barP1, IndicatorLen indL2, BarPeriod barP2, int weight)
{
	if(h_outScores == nullptr || numOfResults <= 0 || !g_HistDataStream || !g_CurrDataStream)
	{
		std::cout << "Invalid input to Calc_TwoIndicatorsRatio_ScoreCalcs" << std::endl;
		return;
	}
	int indTIdx = static_cast<int>(indT);
	
	int indL_X_Idx = static_cast<int>(indL1);
	int barP_X_Idx = static_cast<int>(barP1);
	float* histInd_X_values = g_HistDataStream->m_indicators[indTIdx].m_lens[indL_X_Idx].m_timeFrames[barP_X_Idx].m_valueTypes[static_cast<int>(IndicatorValueType::Value)].m_d_data;
	float* histInd_X_slopes = g_HistDataStream->m_indicators[indTIdx].m_lens[indL_X_Idx].m_timeFrames[barP_X_Idx].m_valueTypes[static_cast<int>(IndicatorValueType::Slope)].m_d_data;
	float* currInd_X_values = g_CurrDataStream->m_indicators[indTIdx].m_lens[indL_X_Idx].m_timeFrames[barP_X_Idx].m_valueTypes[static_cast<int>(IndicatorValueType::Value)].m_d_data;
	float* currInd_X_slopes = g_CurrDataStream->m_indicators[indTIdx].m_lens[indL_X_Idx].m_timeFrames[barP_X_Idx].m_valueTypes[static_cast<int>(IndicatorValueType::Slope)].m_d_data;

	int indL_Y_Idx = static_cast<int>(indL2);
	int barP_Y_Idx = static_cast<int>(barP2);
	float* histInd_Y_values = g_HistDataStream->m_indicators[indTIdx].m_lens[indL_Y_Idx].m_timeFrames[barP_Y_Idx].m_valueTypes[static_cast<int>(IndicatorValueType::Value)].m_d_data;
	float* histInd_Y_slopes = g_HistDataStream->m_indicators[indTIdx].m_lens[indL_Y_Idx].m_timeFrames[barP_Y_Idx].m_valueTypes[static_cast<int>(IndicatorValueType::Slope)].m_d_data;
	float* currInd_Y_values = g_CurrDataStream->m_indicators[indTIdx].m_lens[indL_Y_Idx].m_timeFrames[barP_Y_Idx].m_valueTypes[static_cast<int>(IndicatorValueType::Value)].m_d_data;
	float* currInd_Y_slopes = g_CurrDataStream->m_indicators[indTIdx].m_lens[indL_Y_Idx].m_timeFrames[barP_Y_Idx].m_valueTypes[static_cast<int>(IndicatorValueType::Slope)].m_d_data;
	float* d_results = g_totalScores_D;

	int histSize = g_HistDataStream->m_indicators[indTIdx].m_lens[indL_X_Idx].m_timeFrames[barP_X_Idx].m_valueTypes[static_cast<int>(IndicatorValueType::Value)].m_numElements;
	int currSize = g_CurrDataStream->m_indicators[indTIdx].m_lens[indL_X_Idx].m_timeFrames[barP_X_Idx].m_valueTypes[static_cast<int>(IndicatorValueType::Value)].m_numElements;
	if (USE_TILED_CALCS_SCORECALCS)
	{
		TwoIndCalc_tiled(histInd_X_values, histInd_X_slopes, currInd_X_values, currInd_X_slopes,
			histInd_Y_values, histInd_Y_slopes, currInd_Y_values, currInd_Y_slopes,
			d_results, currSize, histSize, weight);
	}
	else
	{
		TwoIndCalc(histInd_X_values, histInd_X_slopes, currInd_X_values, currInd_X_slopes,
			histInd_Y_values, histInd_Y_slopes, currInd_Y_values, currInd_Y_slopes,
			d_results, currSize, histSize, weight);
	}
	
	CudaMemoryCopyToHost<float>(h_outScores, d_results, numOfResults);
}

void CPP_To_Cuda_Tunnel::Calc_Indicator_ScoreCalcs(float* h_outScores, int numOfResults, IndicatorType indT, IndicatorLen indL, BarPeriod barP, int weight)
{
	if (!h_outScores || numOfResults <= 0 || !g_totalScores_D|| !g_HistDataStream || !g_CurrDataStream)
	{
		std::cout << "Invalid input to Calc_Indicator_ScoreCalcs" << std::endl;
		return;
	}
	int indTIdx = static_cast<int>(indT);
	int indLIdx = static_cast<int>(indL);
	int barPIdx = static_cast<int>(barP);
	float* histInd_values = g_HistDataStream->m_indicators[indTIdx].m_lens[indLIdx].m_timeFrames[barPIdx].m_valueTypes[static_cast<int>(IndicatorValueType::Value)].m_d_data;
	float* histInd_slopes = g_HistDataStream->m_indicators[indTIdx].m_lens[indLIdx].m_timeFrames[barPIdx].m_valueTypes[static_cast<int>(IndicatorValueType::Slope)].m_d_data;
	
	float* currInd_values = g_CurrDataStream->m_indicators[indTIdx].m_lens[indLIdx].m_timeFrames[barPIdx].m_valueTypes[static_cast<int>(IndicatorValueType::Value)].m_d_data;
	float* currInd_slopes = g_CurrDataStream->m_indicators[indTIdx].m_lens[indLIdx].m_timeFrames[barPIdx].m_valueTypes[static_cast<int>(IndicatorValueType::Slope)].m_d_data;
	
	float* d_results = g_totalScores_D;

	int histSize = g_HistDataStream->m_indicators[indTIdx].m_lens[indLIdx].m_timeFrames[barPIdx].m_valueTypes[static_cast<int>(IndicatorValueType::Value)].m_numElements;
	int currSize = g_CurrDataStream->m_indicators[indTIdx].m_lens[indLIdx].m_timeFrames[barPIdx].m_valueTypes[static_cast<int>(IndicatorValueType::Value)].m_numElements;

	if(indT == IndicatorType::EMA)
	{
		// Call EMA scoring function
		if (USE_TILED_CALCS_SCORECALCS)
			calcMA_tiled(histInd_values, histInd_slopes, currInd_values, currInd_slopes, d_results, currSize, histSize, weight);
		else
			calcMA(histInd_values, histInd_slopes, currInd_values, currInd_slopes, d_results, currSize, histSize, weight);
	}
	else if(indT == IndicatorType::RSI)
	{
		// Call RSI scoring function
		if (USE_TILED_CALCS_SCORECALCS)
			calcRSI_tiled(histInd_values, histInd_slopes, currInd_values, currInd_slopes, d_results, currSize, histSize, weight);
		else
			calcRSI(histInd_values, histInd_slopes, currInd_values, currInd_slopes, d_results, currSize, histSize, weight);
	}
	else if(indT == IndicatorType::ZLSMA)
	{
		// Call ZLSMA scoring function
		if (USE_TILED_CALCS_SCORECALCS)
			calcMA_tiled(histInd_values, histInd_slopes, currInd_values, currInd_slopes, d_results, currSize, histSize, weight);
		else
			calcMA(histInd_values, histInd_slopes, currInd_values, currInd_slopes, d_results, currSize, histSize, weight);
	}
	CudaMemoryCopyToHost<float>(h_outScores, d_results, numOfResults);
}
// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setter function to set data for a specific indicator, length, and timeframe in the current data stream for the GPU device /////////
// If data already exists for the specified combination, it will not be overwritten unless forceOverWrite/////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CPP_To_Cuda_Tunnel::Set_CurrDataTimeFreameStream(float* h_data, size_t h_dataSize, IndicatorType indT, IndicatorLen indL, BarPeriod barP, IndicatorValueType valT ,bool forceOverWrite)
{
	StreamSetters::CheckDataStreamTree(&g_CurrDataStream, indT, indL, barP, valT); // Ensure the data stream tree is properly initialized
	int indTypeIndex = static_cast<int>(indT);
	int indicatorLenIndex = static_cast<int>(indL);
	int barPeriodIndex = static_cast<int>(barP);
	int valTypeIndex = static_cast<int>(valT);
	ValueTypeStream_t* valTStream = &g_CurrDataStream->m_indicators[indTypeIndex].m_lens[indicatorLenIndex].m_timeFrames[barPeriodIndex].m_valueTypes[valTypeIndex];

	float* existingData = valTStream->m_d_data;

	if(existingData != nullptr && !forceOverWrite)
	{
		// Data already exists, do not overwrite
		return;
	}
	else if(existingData != nullptr && forceOverWrite)
	{
		// Free existing device memory before overwriting
		cudaFree(existingData);
		valTStream->m_d_data = nullptr;
	}

	valTStream->m_valueType = valT;
	valTStream->m_numElements = static_cast<int>(h_dataSize);
	CudaMemoryAllocation(&(valTStream->m_d_data), h_dataSize);
	CudaMemoryCopyToDevice<float>(valTStream->m_d_data, h_data, h_dataSize);
}

void CPP_To_Cuda_Tunnel::Set_HistDataTimeFreameStream(float* h_data, size_t h_dataSize, IndicatorType indT, IndicatorLen indL, BarPeriod barP, IndicatorValueType valT ,bool forceOverWrite)
{
	StreamSetters::CheckDataStreamTree(&g_HistDataStream, indT, indL, barP, valT); // Ensure the data stream tree is properly initialized
	int indTypeIndex = static_cast<int>(indT);
	int indicatorLenIndex = static_cast<int>(indL);
	int barPeriodIndex = static_cast<int>(barP);
	int valTypeIndex = static_cast<int>(valT);
	ValueTypeStream_t* valTStream = &g_HistDataStream->m_indicators[indTypeIndex].m_lens[indicatorLenIndex].m_timeFrames[barPeriodIndex].m_valueTypes[valTypeIndex];
	float* existingData = valTStream->m_d_data;
	if (existingData != nullptr && !forceOverWrite)
	{
		// Data already exists, do not overwrite
		return;
	}
	else if (existingData != nullptr && forceOverWrite)
	{
		// Free existing device memory before overwriting
		cudaFree(existingData);
		valTStream->m_d_data = nullptr;
	}
	valTStream->m_valueType = valT;

	valTStream->m_numElements = static_cast<int>(h_dataSize);
	CudaMemoryAllocation(&(valTStream->m_d_data), h_dataSize);
	CudaMemoryCopyToDevice<float>(valTStream->m_d_data, h_data, h_dataSize);
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
