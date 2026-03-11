#include "CPUEngine_ScoreCalcs.h"
#include "KernelEngine_ScoreCalcs_General.h"
#include <iostream>
inline float CalcMAScore(const float histValue,
	const float histSlope,
	const float currValue,
	const float currSlope,
	const float histLowest,
	const float currLowest,
	const float invHistRange,
	const float invCurrRange)
{
	// Normalize values to [0, 1] range based on the lowest and highest values
	const float normHistValue = (histValue - histLowest) * invHistRange;
	const float normCurrValue = (currValue - currLowest) * invCurrRange;

	float score = pair_score(normHistValue, normCurrValue, histSlope, currSlope);
	return score;
}

void FullScan_CPU_MA(
	////////////////////////////////// Hist MAs arrays ////////////////////////////////////////////////////
	const float* histMAs_values /*array for MAs hist values */							
	, const float* histMAs_slopes /*array for MAs hist slopes */							
	///
	, const float* histMAs_MinValues, const float* d_histMAs_MaxValues		
	, const float* histMAs_MinSlopes, const float* d_histMAs_MaxSlopes		
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////// Current MAs arrays /////////////////////////////////////////////////
	, const float* currMAs_values /* array for MAs curr values*/							
	, const float* currMAs_slopes /* array for MAs curr slopes*/							
	///
	, const float* currMAs_MinValues, const float* currMAs_MaxValues		
	, const float* currMAs_MinSlopes, const float* currMAs_MaxSlopes		
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	, float* scores /*output scores, where Score(idx) is the score of currMAs(t)&&histMAs(idx + t), t is the index of the current candle */
	, const int startIdx, const int endIdx
	, const int currPatternLen, const int outLen, const float weight)
{
	const float currLowest = currMAs_MinValues[0]; // Assuming the current pattern's lowest value is at index 0
	const float currHighest = currMAs_MaxValues[0]; // Assuming the current pattern's highest value is at index 0
	
	const float eps = 1e-6f; // Small epsilon to prevent division by zero

	const float currRange = currHighest - currLowest;
	if(currRange <= eps) // If the current range is zero or negative, we cannot normalize properly, so we should skip the scoring for this pattern to avoid invalid calculations
	{
		return;
	}

	const float invCurrRange = 1.0f / fmaxf(currRange, eps); // Inverse of current range for normalization, with check to prevent division by zero

	for (int idx = startIdx; idx < endIdx; idx++)
	{
		const float histLowest = histMAs_MinValues[idx];
		const float histHighest = d_histMAs_MaxValues[idx];
		const float histRange = histHighest - histLowest;

		if(histRange <= eps) // If the historical range is zero or negative, skip this index to avoid invalid calculations
		{
			continue;
		}

		const float invHistRange = 1.0f / fmaxf(histRange, eps); // Inverse of historical range for normalization, with check to prevent division by zero
		

		float score = 0.0f;
		
		for(int t = 0; t < currPatternLen; t++)
		{
			const float histValue = histMAs_values[idx + t];
			const float histSlope = histMAs_slopes[idx + t];
			const float currValue = currMAs_values[t]; 
			const float currSlope = currMAs_slopes[t]; 
			score += CalcMAScore(histValue, histSlope, currValue, currSlope, histLowest, currLowest, invHistRange, invCurrRange);
		}
		scores[idx] += score * weight; // Multiply the final score by the weight
	}
}

void CandidateScan_CPU_MA(
	const int* candidateIndices /*array of candidate indices to process */
	, int candidateCount
	////////////////////////////////// Hist MAs arrays ////////////////////////////////////////////////////
	, const float* histMAs_values /*array for MAs hist values */
	, const float* histMAs_slopes /*array for MAs hist slopes */
	///
	, const float* histMAs_MinValues, const float* d_histMAs_MaxValues
	, const float* histMAs_MinSlopes, const float* d_histMAs_MaxSlopes
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////// Current MAs arrays /////////////////////////////////////////////////
	, const float* currMAs_values /* array for MAs curr values*/
	, const float* currMAs_slopes /* array for MAs curr slopes*/
	///
	, const float* currMAs_MinValues, const float* currMAs_MaxValues
	, const float* currMAs_MinSlopes, const float* currMAs_MaxSlopes
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	, float* scores /*output scores, where Score(idx) is the score of currMAs(t)&&histMAs(idx + t), t is the index of the current candle */
	, const int candidateStartIdx, const int candidateEndIdx
	, const int currPatternLen, const int outLen, const float weight)
{
	if(candidateEndIdx > candidateCount)
	{
		std::cout << "Warning: candidateEndIdx " << candidateEndIdx << " is greater than candidateCount " << candidateCount << ". Return ! ." << std::endl;
		return;
	}

	const float currLowest = currMAs_MinValues[0]; // Assuming the current pattern's lowest value is at index 0
	const float currHighest = currMAs_MaxValues[0]; // Assuming the current pattern's highest value is at index 0

	const float eps = 1e-6f; // Small epsilon to prevent division by zero

	const float currRange = currHighest - currLowest;
	if(currRange <= eps) // If the current range is zero or negative, we cannot normalize properly, so we should skip the scoring for this pattern to avoid invalid calculations
	{
		return;
	}
	const float invCurrRange = 1.0f / fmaxf(currRange, eps); // Inverse of current range for normalization, with check to prevent division by zero

	for(int candidateIdx = candidateStartIdx; candidateIdx < candidateEndIdx; candidateIdx++)
	{
		int idx = candidateIndices[candidateIdx];
		if (idx < 0 || idx >= outLen) // Validate candidate index to ensure it's within the valid range of historical data indices
		{
			std::cout << "Warning: Candidate index " << idx << " is out of bounds. Skipping this index." << std::endl;
			continue; // Skip invalid candidate index
		}

		const float histLowest = histMAs_MinValues[idx];
		const float histHighest = d_histMAs_MaxValues[idx];
		const float histRange = histHighest - histLowest;
		if(histRange <= eps) // If the historical range is zero or negative, skip this index to avoid invalid calculations
		{
			continue;
		}
		const float invHistRange = 1.0f / fmaxf(histRange, eps); // Inverse of historical range for normalization, with check to prevent division by zero
		float score = 0.0f;

		for(int t = 0; t < currPatternLen; t++)
		{
			const float histValue = histMAs_values[idx + t];
			const float histSlope = histMAs_slopes[idx + t];
			const float currValue = currMAs_values[t]; 
			const float currSlope = currMAs_slopes[t]; 
			score += CalcMAScore(histValue, histSlope, currValue, currSlope, histLowest, currLowest, invHistRange, invCurrRange);
		}
		scores[idx] += score * weight; // Multiply the final score by the weight
	}

}