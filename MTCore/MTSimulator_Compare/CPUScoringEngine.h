#pragma once
#include "BaseScoringEngine.h"

class CPUScoringEngine : public BaseScoringEngine
{
	public:
	CPUScoringEngine(const std::vector<IndicatorLen>& lenFilter,
					 const std::vector<BarPeriod>& barPeriodFilter,
					 const std::vector<IndicatorType>& indicatorTypeFilter, float threshold)
		: BaseScoringEngine(lenFilter, barPeriodFilter, indicatorTypeFilter, threshold)
	{
	}
protected:
	virtual ScanEngineType GetEngineType() const override { return ScanEngineType::CPUEngine; }

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
		, int currTimeLineLen, int numOfResults, IndicatorType indT, int weight, const int* inputIndices, int inputIndicesCount) override;

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
		, int currTimeLineLen, int numOfResults, int weight, const int* inputIndices, const int inputIndicesCount) override;
private:
	void CandidateScan_SingleIndicator(float* results, const float* histValues, const float* histSlopes, const float* hist_MinValues, const float* hist_MaxValues, const float* hist_MinSlopes, const float* hist_MaxSlopes, const float* currValues, const float* currSlopes, const float* curr_MinValues, const float* curr_MaxValues, const float* curr_MinSlopes, const float* curr_MaxSlopes, int currTimeLineLen, int numOfResults, IndicatorType indT, int weight, const int* candidateIndices, int candidateCount);
	void FullScan_SingleIndicator(float* results, const float* histValues, const float* histSlopes, const float* hist_MinValues, const float* hist_MaxValues, const float* hist_MinSlopes, const float* hist_MaxSlopes, const float* currValues, const float* currSlopes, const float* curr_MinValues, const float* curr_MaxValues, const float* curr_MinSlopes, const float* curr_MaxSlopes, int currTimeLineLen, int numOfResults, IndicatorType indT, int weight);
};

