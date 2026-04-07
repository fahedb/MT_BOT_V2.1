#include "BaseScoringEngine.h"
#include "PatternGodUtils.h"
#include "TimeLineArrayBuilder.h"
#include "StreamSetters.h"

#define COMAPRE_WEIGHT 10
#define THRESHOLD_FOR_SCAN 0.1f //Need to make this configurable later, this is the minimum score for a candidate to be included


// Sort indices ascending (by index), optionally remove duplicates.
// Useful before launching Candidate kernels to improve memory locality and avoid races on d_scores[idx] += ...
inline void SortIndices(std::vector<int>& indices, bool makeUnique = true)
{
	// Sort by index (ascending) -- This is important for memory coalescing in CUDA kernels and CPU cache locality
	std::sort(indices.begin(), indices.end());

	if (makeUnique)
	{
		// Remove duplicates to avoid non-deterministic
		indices.erase(std::unique(indices.begin(), indices.end()), indices.end());
	}
}

std::vector<int> Get_TopKScoresIndices(float* totalScores, int numOfResults, int topKNum)
{
	// Stage 1: Build indices
	std::vector<int> idx(numOfResults);
	for (int i = 0; i < numOfResults; i++)
		idx[i] = i;

	// Stage 2: nth_element – O(N)
	std::nth_element(
		idx.begin(), idx.begin() + topKNum, idx.end(),
		[&](int a, int b) { return totalScores[a] > totalScores[b]; }
	);

	// Stage 3: sort only the top K – O(K log K)
	std::sort(
		idx.begin(), idx.begin() + topKNum,
		[&](int a, int b) { return totalScores[a] > totalScores[b]; }
	);

	// shrink to topK
	idx.resize(topKNum);
	return idx;
}

bool AreLenPairsScanned(const std::vector<std::pair<IndicatorLen, IndicatorLen>>& scannedLens, IndicatorLen lenX, IndicatorLen lenY)
{
	for (const auto& pair : scannedLens)
	{
		if ((pair.first == lenX && pair.second == lenY) || (pair.first == lenY && pair.second == lenX))
		{
			return true;
		}
	}
	return false;
}
bool AreBarPeriodPairsScanned(const std::vector<std::pair<BarPeriod, BarPeriod>>& scannedBarPs, BarPeriod barPX, BarPeriod barPY)
{
	for (const auto& pair : scannedBarPs)
	{
		if ((pair.first == barPX && pair.second == barPY) || (pair.first == barPY && pair.second == barPX))
		{
			return true;
		}
	}
	return false;
}

float* Get_DataPointer_KernelMemoryContextHelper(dataStream_t* dataSt, IndicatorType indT, IndicatorLen indL, BarPeriod barP, IndicatorValueType val)
{
	return dataSt->m_indicators.at(static_cast<int>(indT))
		.m_lens.at(static_cast<int>(indL))
		.m_timeFrames.at(static_cast<int>(barP))
		.m_valueTypes.at(static_cast<int>(val)).m_d_data;
}

size_t Get_NumElements_KernelMemoryContextHelper(dataStream_t* dataSt, IndicatorType indT, IndicatorLen indL, BarPeriod barP, IndicatorValueType val)
{
	return dataSt->m_indicators.at(static_cast<int>(indT))
		.m_lens.at(static_cast<int>(indL))
		.m_timeFrames.at(static_cast<int>(barP))
		.m_valueTypes.at(static_cast<int>(val)).m_numElements;
}

BaseScoringEngine::BaseScoringEngine(float threshold)
{
	// Default constructor with no filters, meaning all lengths, bar periods, and indicator types will be included
	IndicatorLenList lenList;
	m_indicatorLen_Filter = lenList.lengths;
	BarPeriodList barPList;
	m_barPeriod_Filter = barPList.periods;
	IndicatorTypeList indTList;
	m_indicatorType_Filter = indTList.types;
	m_threshold = threshold;
}

std::vector<int> BaseScoringEngine::RunScan(const std::vector<int>& inputIndices) // This function will run the scan based on the enabled scan types and filters, and return the resulting indices that passed the scan
{
	if (m_kernelMemoryContext == nullptr || !m_indicatorType_Filter.size() || !m_indicatorLen_Filter.size() || !m_barPeriod_Filter.size() )
	{
		// Handle uninitialized kernel memory context, e.g., log an error or throw an exception
		Logger::getInstance().WriteLog("BaseScoringEngine::RunScan - Kernel memory context is not initialized or no filters initialized", false, true, LogType::ERR);
		return {};
	}

	m_kernelMemoryContext->PrepareForNewScan(); // Prepare the kernel memory context for a new scan, e.g., reset results, sync data if needed, etc.

	if (m_singleIndicatorScanEnabled)
	{
		for (auto indT : m_indicatorType_Filter)
		{
			RunScan_IndicatorType(indT, inputIndices);
			// Update resultIndices based on the scan results for this indicator type
			// This is a placeholder - actual implementation would depend on how results are stored and accessed
		}
	}
	if (m_twoIndicatorsRatioScanEnabled)
	{
		for (auto indT : m_indicatorType_Filter)
		{
			RunScan_DualIndicators_IndicatorType(indT, inputIndices);
			// Update resultIndices based on the scan results for this two indicators ratio scan
			// This is a placeholder - actual implementation would depend on how results are stored and accessed
		}
	}
	MemoryLocation myMemLoc = GetEngineType() == ScanEngineType::CPUEngine ? MemoryLocation::HOST : MemoryLocation::DEVICE;
	if (myMemLoc == MemoryLocation::DEVICE)
		m_kernelMemoryContext->SyncResultsToHost(); // Sync results back to host if we are using GPU engine, so we can access the total scores array on the host for further processing

	int histSize = Get_NumElements_KernelMemoryContextHelper(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::HISTORIC, myMemLoc), m_indicatorType_Filter.front(), m_indicatorLen_Filter.front(), m_barPeriod_Filter.front(), IndicatorValueType::Value);
	int currSize = Get_NumElements_KernelMemoryContextHelper(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::CURRENT, myMemLoc), m_indicatorType_Filter.front(), m_indicatorLen_Filter.front(), m_barPeriod_Filter.front(), IndicatorValueType::Value);
	
	int limit = static_cast<int>(histSize - currSize + 1);
	float* totalScores = m_kernelMemoryContext->Get_HostTotalScoresPtr();
	TimeLineArrayBuilder::BuildTotalScoreArray(m_kernelMemoryContext->Get_ResultsDataStreamPtr(MemoryLocation::HOST), totalScores, limit);
	
	
	// Get top K scores indices based on the total scores array
	int KtopScoresCount = inputIndices.size() ? inputIndices.size() * m_threshold : limit * m_threshold; // This is a placeholder for how many top scores to consider, can be made configurable

	KtopScoresCount = std::min(KtopScoresCount, limit); // Ensure we don't exceed the limit of available scores
	KtopScoresCount = std::max(KtopScoresCount, 1); // Ensure at least one score is considered

	std::vector<int> topKScoresIndices = Get_TopKScoresIndices(
		m_kernelMemoryContext->Get_HostTotalScoresPtr(),
		limit,
		KtopScoresCount);

	PatternGodUtils::FilterDuplicateIndicesByScoreWindow(totalScores, topKScoresIndices, 30); // Filter out duplicate indices that are within a window of the current size, to avoid redundant processing of very similar patterns. This is especially useful when the scoring logic can produce high scores for patterns that are very close to each other in time, which might not be desirable to consider as separate candidates.

	SortIndices(topKScoresIndices); // Sort the resulting indices for better memory access patterns in subsequent processing

	for (int i = 0; i < 100; i++)
	{
		std::cout << totalScores[i];
	}

	return topKScoresIndices;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////// Single Indicator Scan Functions ////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BaseScoringEngine::RunScan_IndicatorType(IndicatorType indT, const std::vector<int>& inputIndices)
{
	for(auto indL : m_indicatorLen_Filter)
	{
		RunScan_IndicatorLen(indT, indL, inputIndices);
	}
}

void BaseScoringEngine::RunScan_IndicatorLen(IndicatorType indT, IndicatorLen indL, const std::vector<int>& inputIndices)
{
	// Implementation for scanning per indicator and length
	for(auto barP : m_barPeriod_Filter)
	{
		RunScan_BarPeriod(indT, indL, barP, inputIndices);
	}
}

void BaseScoringEngine::RunScan_BarPeriod(IndicatorType indT, IndicatorLen indL, BarPeriod barP, const std::vector<int>& inputIndices)
{
	MemoryLocation myMemLoc = GetEngineType() == ScanEngineType::CPUEngine ? MemoryLocation::HOST : MemoryLocation::DEVICE;

	float* d_resData = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_ResultsDataStreamPtr(myMemLoc), indT, indL, barP, IndicatorValueType::Value);

	const float* d_histValues = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::HISTORIC, myMemLoc), indT, indL, barP, IndicatorValueType::Value);
	const float* d_histSlopes = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::HISTORIC, myMemLoc), indT, indL, barP, IndicatorValueType::Slope);

	const float* d_histMinValues = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::HISTORIC, myMemLoc), indT, indL, barP, IndicatorValueType::Value);
	const float* d_histMaxValues = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::HISTORIC, myMemLoc), indT, indL, barP, IndicatorValueType::Value);

	const float* d_histMinSlopes = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::HISTORIC, myMemLoc), indT, indL, barP, IndicatorValueType::Slope);
	const float* d_histMaxSlopes = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::HISTORIC, myMemLoc), indT, indL, barP, IndicatorValueType::Slope);

	const float* d_currValues = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::CURRENT, myMemLoc), indT, indL, barP, IndicatorValueType::Value);
	const float* d_currSlopes = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::CURRENT, myMemLoc), indT, indL, barP, IndicatorValueType::Slope);

	const float* d_currMinValues = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::CURRENT, myMemLoc), indT, indL, barP, IndicatorValueType::Value);
	const float* d_currMaxValues = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::CURRENT, myMemLoc), indT, indL, barP, IndicatorValueType::Value);

	const float* d_currMinSlopes = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::CURRENT, myMemLoc), indT, indL, barP, IndicatorValueType::Slope);
	const float* d_currMaxSlopes = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::CURRENT, myMemLoc), indT, indL, barP, IndicatorValueType::Slope);

	
	int numResults = Get_NumElements_KernelMemoryContextHelper(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::HISTORIC, myMemLoc), indT, indL, barP, IndicatorValueType::Value);
	int currTimeLineLen = Get_NumElements_KernelMemoryContextHelper(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::CURRENT, myMemLoc), indT, indL, barP, IndicatorValueType::Value);


	const int* h_indices = inputIndices.data();
	int inputIndicesSize = static_cast<int>(inputIndices.size());
	int* d_inputIndices = nullptr;

	if (GetEngineType() == ScanEngineType::GPUEngine && inputIndicesSize) {
		StreamSetters::MemoryAllocate(&d_inputIndices, inputIndicesSize, MemoryLocation::DEVICE);
		StreamSetters::MoveDataToDevice(h_indices, &d_inputIndices, inputIndicesSize);
	}

	Scan_SingleIndicator(d_resData,
		d_histValues, d_histSlopes,
		d_histMinValues, d_histMaxValues,
		d_histMinSlopes, d_histMaxSlopes,
		d_currValues, d_currSlopes,
		d_currMinValues, d_currMaxValues,
		d_currMinSlopes, d_currMaxSlopes,
		currTimeLineLen, numResults, indT, COMAPRE_WEIGHT, d_inputIndices, inputIndicesSize);

	if (GetEngineType() == ScanEngineType::GPUEngine && inputIndicesSize && d_inputIndices) {
		StreamSetters::MemoryFree(&d_inputIndices, MemoryLocation::DEVICE);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////// Two Indicators Ratio Scan Functions ////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BaseScoringEngine::RunScan_DualIndicators_IndicatorType(IndicatorType indT, const std::vector<int>& inputIndices)
{
	// Meed to scan for every LenX and LenY --NOTE Scanned(LenX,LenY) == Scanned(LenY,LenX) so we can skip half of the scans
	std::vector<std::pair<IndicatorLen, IndicatorLen>> scannedLens;
	for(auto lenX : m_indicatorLen_Filter)
	{
		for(auto lenY : m_indicatorLen_Filter)
		{
			if (!AreLenPairsScanned(scannedLens, lenX, lenY))
			{
				RunScan_DualIndicators_IndicatorLen(indT, lenX, lenY, inputIndices);
				scannedLens.push_back(std::make_pair(lenX, lenY));
			}
		}
	}
}

void BaseScoringEngine::RunScan_DualIndicators_IndicatorLen(IndicatorType indT, IndicatorLen lenX, IndicatorLen LenY, const std::vector<int>& inputIndices)
{
	std::vector<std::pair<BarPeriod, BarPeriod>> scannedBarPs;
	for (auto barPX : m_barPeriod_Filter)
	{
		for (auto barPY : m_barPeriod_Filter)
		{
			if (!AreBarPeriodPairsScanned(scannedBarPs, barPX, barPY))
			{
				RunScan_DualIndicators_BarPeriod(indT, lenX, barPX, LenY, barPY, inputIndices);
				scannedBarPs.push_back(std::make_pair(barPX, barPY));
			}
		}
	}
}

void BaseScoringEngine::RunScan_DualIndicators_BarPeriod(IndicatorType indT, IndicatorLen lenX, BarPeriod barPX, IndicatorLen LenY, BarPeriod barPY, const std::vector<int>& inputIndices)
{
	if (lenX == LenY && barPX == barPY)
	{
		return; // Skip same length and same bar period
	}

	MemoryLocation myMemLoc = GetEngineType() == ScanEngineType::CPUEngine ? MemoryLocation::HOST : MemoryLocation::DEVICE;
	float* d_resData = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_ResultsDataStreamPtr(myMemLoc), indT, lenX, barPX, IndicatorValueType::Value);

	////////////// Get IndX data //////////////////	
	const float* d_histIndX_Values = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::HISTORIC, myMemLoc), indT, lenX, barPX, IndicatorValueType::Value);
	const float* d_histIndX_Slopes = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::HISTORIC, myMemLoc), indT, lenX, barPX, IndicatorValueType::Slope);

	const float* d_histIndX_MinValues = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::HISTORIC, myMemLoc), indT, lenX, barPX, IndicatorValueType::Value);
	const float* d_histIndX_MaxValues = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::HISTORIC, myMemLoc), indT, lenX, barPX, IndicatorValueType::Value);
	const float* d_histIndX_MinSlopes = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::HISTORIC, myMemLoc), indT, lenX, barPX, IndicatorValueType::Slope);
	const float* d_histIndX_MaxSlopes = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::HISTORIC, myMemLoc), indT, lenX, barPX, IndicatorValueType::Slope);
	////////////////////////////////////////////////

	////////////// Get curr IndX data //////////////////
	const float* d_currIndX_Values = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::CURRENT, myMemLoc), indT, lenX, barPX, IndicatorValueType::Value);
	const float* d_currIndX_Slopes = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::CURRENT, myMemLoc), indT, lenX, barPX, IndicatorValueType::Slope);

	const float* d_currIndX_MinValues = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::CURRENT, myMemLoc), indT, lenX, barPX, IndicatorValueType::Value);
	const float* d_currIndX_MaxValues = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::CURRENT, myMemLoc), indT, lenX, barPX, IndicatorValueType::Value);
	const float* d_currIndX_MinSlopes = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::CURRENT, myMemLoc), indT, lenX, barPX, IndicatorValueType::Slope);
	const float* d_currIndX_MaxSlopes = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::CURRENT, myMemLoc), indT, lenX, barPX, IndicatorValueType::Slope);
	////////////////////////////////////////////////

	////////////// Get hist IndY data //////////////////
	const float* d_histIndY_Values = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::HISTORIC, myMemLoc), indT, LenY, barPY, IndicatorValueType::Value);
	const float* d_histIndY_Slopes = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::HISTORIC, myMemLoc), indT, LenY, barPY, IndicatorValueType::Slope);

	const float* d_histIndY_MinValues = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::HISTORIC, myMemLoc), indT, LenY, barPY, IndicatorValueType::Value);
	const float* d_histIndY_MaxValues = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::HISTORIC, myMemLoc), indT, LenY, barPY, IndicatorValueType::Value);
	const float* d_histIndY_MinSlopes = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::HISTORIC, myMemLoc), indT, LenY, barPY, IndicatorValueType::Slope);
	const float* d_histIndY_MaxSlopes = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::HISTORIC, myMemLoc), indT, LenY, barPY, IndicatorValueType::Slope);
	/////////////////////////////////////////////////

	////////////// Get curr IndY data //////////////////
	const float* d_currIndY_Values = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::CURRENT, myMemLoc), indT, LenY, barPY, IndicatorValueType::Value);
	const float* d_currIndY_Slopes = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::CURRENT, myMemLoc), indT, LenY, barPY, IndicatorValueType::Slope);

	const float* d_currIndY_MinValues = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::CURRENT, myMemLoc), indT, LenY, barPY, IndicatorValueType::Value);
	const float* d_currIndY_MaxValues = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::CURRENT, myMemLoc), indT, LenY, barPY, IndicatorValueType::Value);
	const float* d_currIndY_MinSlopes = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::CURRENT, myMemLoc), indT, LenY, barPY, IndicatorValueType::Slope);
	const float* d_currIndY_MaxSlopes = Get_DataPointer_KernelMemoryContextHelper(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::CURRENT, myMemLoc), indT, LenY, barPY, IndicatorValueType::Slope);
	/////////////////////////////////////////////////


	int numResults = Get_NumElements_KernelMemoryContextHelper(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::HISTORIC, myMemLoc), indT, LenY, barPY, IndicatorValueType::Value);
	int currTimeLineLen = Get_NumElements_KernelMemoryContextHelper(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::CURRENT, myMemLoc), indT, LenY, barPY, IndicatorValueType::Value);

	const int* h_indices = inputIndices.data();
	int inputIndicesSize = static_cast<int>(inputIndices.size());
	int* d_inputIndices = nullptr;
	
	if (GetEngineType() == ScanEngineType::GPUEngine && inputIndicesSize)
	{
		StreamSetters::MemoryAllocate(&d_inputIndices, inputIndicesSize, MemoryLocation::DEVICE);
		StreamSetters::MoveDataToDevice(h_indices, &d_inputIndices, inputIndicesSize);
	}
	
	// Implementation for scanning per indicator and length
	Scan_DualIndicatorsRatio(
		d_resData,
		///////////////////// Hist len X arrays ////////////////////////////////////////////
		d_histIndX_Values, d_histIndX_Slopes,
		d_histIndX_MinValues, d_histIndX_MaxValues,
		d_histIndX_MinSlopes, d_histIndX_MaxSlopes,
		////////////////////////////////////////////////////////////////////////////////////
		/////////////////////// Current len X arrays //////////////////////////////////////
		d_currIndX_Values, d_currIndX_Slopes,
		d_currIndX_MinValues, d_currIndX_MaxValues,
		d_currIndX_MinSlopes, d_currIndX_MaxSlopes,
		///////////////////////////////////////////////////////////////////////////////////
		////////////////////// Hist len Y arrays //////////////////////////////////////////
		d_histIndY_Values, d_histIndY_Slopes,
		d_histIndY_MinValues, d_histIndY_MaxValues,
		d_histIndY_MinSlopes, d_histIndY_MaxSlopes,
		///////////////////////////////////////////////////////////////////////////////////
		///////////////////// Current len Y arrays ////////////////////////////////////////
		d_currIndY_Values, d_currIndY_Slopes,
		d_currIndY_MinValues, d_currIndY_MaxValues,
		d_currIndY_MinSlopes, d_currIndY_MaxSlopes,
		currTimeLineLen, numResults, COMAPRE_WEIGHT, d_inputIndices, inputIndicesSize);

	if (GetEngineType() == ScanEngineType::GPUEngine && inputIndicesSize && d_inputIndices)
		StreamSetters::MemoryFree(&d_inputIndices, MemoryLocation::DEVICE);
}