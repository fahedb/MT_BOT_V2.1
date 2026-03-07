#pragma once
#include <vector>
#include "IndicatorEnums.h"
#include "KernelExecutionContext.h"
#include "Logger.h"

class BaseScoringEngine
{
private:
	std::vector<IndicatorLen> m_indicatorLen_Filter;
	std::vector<BarPeriod> m_barPeriod_Filter;
	std::vector<IndicatorType> m_indicatorType_Filter;
	
	KernelExecutionContext_PatternGod* m_kernelMemoryContext = nullptr; // This will be used by the scoring engine to manage memory for CUDA calculations, if needed. It can be initialized in the constructor or through a separate initialization function.
	
	bool m_singleIndicatorScanEnabled = true;
	bool m_twoIndicatorsRatioScanEnabled = true;
public:
	BaseScoringEngine(		const std::vector<IndicatorLen>& lenFilter,
							const std::vector<BarPeriod>& barPeriodFilter, 
							const std::vector<IndicatorType>& indicatorTypeFilter)
	{
		m_indicatorLen_Filter = lenFilter;
		m_barPeriod_Filter = barPeriodFilter;
		m_indicatorType_Filter = indicatorTypeFilter;
	}

	BaseScoringEngine();

	void Set_IndicatorLen_Filter(std::vector<IndicatorLen> lenFilter) { m_indicatorLen_Filter = lenFilter; }
	void Set_BarPeriod_Filter(std::vector<BarPeriod> barPeriodFilter) { m_barPeriod_Filter = barPeriodFilter; }
	void Set_IndicatorType_Filter(std::vector<IndicatorType> indicatorTypeFilter) { m_indicatorType_Filter = indicatorTypeFilter; }

	void Enable_SingleIndicatorScan(bool enable) { m_singleIndicatorScanEnabled = enable; }
	void Enable_TwoIndicatorsRatioScan(bool enable) { m_twoIndicatorsRatioScanEnabled = enable; }
	
	void Set_KernelMemoryContext(KernelExecutionContext_PatternGod* kernelMemoryContext) { m_kernelMemoryContext = kernelMemoryContext; }

	std::vector<int> RunScan(const std::vector<int>& inputIndices);
protected:
	virtual void Scan_SingleIndicator(float* d_results
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
									, int currTimeLineLen, int numOfResults, IndicatorType indT, int weight, const int* inputIndices, int inputIndicesCount) = 0; // Pure virtual function for single indicator scan logic, to be implemented by derived classes


	virtual void Scan_DualIndicatorsRatio(float* d_results
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
										, int currTimeLineLen, int numOfResults, int weight, const int* inputIndices, const int inputIndicesCount) = 0; // Pure virtual function for two indicators ratio scan logic, to be implemented by derived classes

private:
	// --------------------------------------------------------------------------------------------------------------
	////////////////////////////////////////////////////////////
	////////// Single Indicator Scan Routines //////////////////
	////////////////////////////////////////////////////////////
	void RunScan_IndicatorType(IndicatorType indT, const std::vector<int>& inputIndices);
	void RunScan_IndicatorLen(IndicatorType indT, IndicatorLen indL, const std::vector<int>& inputIndices);
	void RunScan_BarPeriod(IndicatorType indT, IndicatorLen indL, BarPeriod barP, const std::vector<int>& inputIndices);
	// --------------------------------------------------------------------------------------------------------------
	////////////////////////////////////////////////////////////////
	////////// Two Indicators Ratio Scan Routines //////////////////
	///////////////////////////////////////////////////////////////
	void RunScan_DualIndicators_IndicatorType(IndicatorType indT, const std::vector<int>& inputIndices);
	void RunScan_DualIndicators_IndicatorLen(IndicatorType indT, IndicatorLen lenX, IndicatorLen LenY, const std::vector<int>& inputIndices);
	void RunScan_DualIndicators_BarPeriod(IndicatorType indT, IndicatorLen lenX, BarPeriod barPX, IndicatorLen LenY, BarPeriod barPY, const std::vector<int>& inputIndices);
	// --------------------------------------------------------------------------------------------------------------
};

