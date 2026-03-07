#include "CPUEngine_ScoreCalcs_TwoInd.h"
#include "KernelEngine_ScoreCalcs_General.h"
#include <iostream>

inline float TwoIndScoreCalc(
	const float histXv,
	const float histXs,
	const float currXv,
	const float currXs,

	const float histYv,
	const float histYs,
	const float currYv,
	const float currYs,

	const float currX_low,
	const float currY_low,
	const float invCurrX,
	const float invCurrY,

	const float histX_low,
	const float histY_low,
	const float invHistX,
	const float invHistY)
{

	/////////////////////////////////////////////////////////////////////////////////
	// Calc normalized diff between  LenX and  LenY for hist vs curr ///////////
	// //////////////////////////////////////////////////////////////////////////////
	// Normalize LenX values

	const float currValDiff = currXv * invCurrX - currYv * invCurrY - (currX_low * invCurrX - currY_low * invCurrY);

	const float histValDiff = histXv * invHistX - histYv * invHistY - (histX_low * invHistX - histY_low * invHistY);

	float score = 0;

	score += RelativeScoreCalc(histValDiff, currValDiff); // compare Ind value differences
	///////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	// Calc slope diff between LenX and LenY for hist and curr ///////////
	////////////////////////////////////////////////////////////////////////////////
	const float currSlopeDiff = currXs - currYs;
	const float histSlopeDiff = histXs - histYs;
	score += RelativeScoreCalc(histSlopeDiff, currSlopeDiff); // compare Ind slope differences
	//////////////////////////////////////////////////////////////////////////////
	return score;
}

void FullScan_CPU_TwoInd(
	/////////////////////////////////// Hist len X arrays //////////////////////////////////////////////
	const float* histLenX_values /*array for  Ind1 hist values */					////
	, const float* histLenX_slopes /*array for  Ind1 hist slopes */					////
	////
	, const float* histLenX_MinValues /*array for  Ind1 hist min values */			////
	, const float* histLenX_MaxValues /*array for  Ind1 hist max values */			////
	, const float* histLenX_MinSlopes /*array for  Ind1 hist min slopes */			////
	, const float* histLenX_MaxSlopes /*array for  Ind1 hist max slopes */			////
	////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////// Current len X arrays ///////////////////////////////////////////
	, const float* currLenX_values /*array for Ind1 curr values*/					////
	, const float* currLenX_slopes /* array for Ind1 curr slopes*/					////

	, const float* currLenX_MinValues /* array for Ind1 curr min values*/			////
	, const float* currLenX_MaxValues /* array for Ind1 curr max values*/			////
	, const float* currLenX_MinSlopes /* array for Ind1 curr min slopes*/			////
	, const float* currLenX_MaxSlopes /* array for Ind1 curr max slopes*/			////
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////// Hist len Y arrays //////////////////////////////////////////////
	, const float* histLenY_values /*array for  Ind2 hist values */					////
	, const float* histLenY_slopes /*array for  Ind2 hist slopes */					////
	////
	, const float* histLenY_MinValues /*array for  Ind2 hist min values */			////
	, const float* histLenY_MaxValues /*array for  Ind2 hist max values */			////
	, const float* histLenY_MinSlopes /*array for  Ind2 hist min slopes */			////
	, const float* histLenY_MaxSlopes /*array for  Ind2 hist max slopes */			////
	////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////// Current len Y arrays ///////////////////////////////////////////
	, const float* currLenY_values /* array for Ind2 curr values*/					////
	, const float* currLenY_slopes /* array for Ind2 curr slopes*/					////
	////
	, const float* currLenY_MinValues /* array for Ind2 curr min values*/			////
	, const float* currLenY_MaxValues /* array for Ind2 curr max values*/			////
	, const float* currLenY_MinSlopes /* array for Ind2 curr min slopes*/			////
	, const float* currLenY_MaxSlopes /* array for Ind2 curr max slopes*/			////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	, const int startIdx, const int endIdx
	, float* scores /*output scores, where Score(idx) is the score of currMAs(t)&&histMAs(idx + t), t is the index of the current candle */
	, const int currPatternLen, const int outLen, const float weight)
{
	if(currPatternLen <= 0 || outLen <= 0 || startIdx < 0 || endIdx <= startIdx || endIdx > outLen)
	{
		std::cout << "Warning: Invalid input parameters. Skipping scoring for this pattern." << std::endl;
		return;
	}

	const float currRange_LenX = currLenX_MaxValues[0] - currLenX_MinValues[0];
	const float currLowest_LenX = currLenX_MinValues[0]; // Get from constant memory
	const float currRange_LenY = currLenY_MaxValues[0] - currLenY_MinValues[0];
	const float currLowest_LenY = currLenY_MinValues[0]; // Get from constant memory
	const float eps = 1e-6f;

	const float invCurrRange_LenX = 1.0f / fmaxf(currRange_LenX, eps);
	const float invCurrRange_LenY = 1.0f / fmaxf(currRange_LenY, eps);

	if (currRange_LenX <= eps || currRange_LenY <= eps) // If the current range is zero or negative, we cannot normalize properly, so we should skip the scoring for this pattern to avoid invalid calculations
	{
		std::cout << "Warning: Zero or negative current range. Skipping scoring for this pattern." << std::endl;
		return;
	}

	for(int idx = startIdx; idx < endIdx; idx++)
	{
		// Normalize values to [0, 1] range based on the lowest and highest values for MA LenX
		const float histRange_LenX = histLenX_MaxValues[idx] - histLenX_MinValues[idx];
		const float histLowest_LenX = histLenX_MinValues[idx]; // Get from constant memory
		// Normalize values to [0, 1] range based on the lowest and highest values for MA LenY
		const float histRange_LenY = histLenY_MaxValues[idx] - histLenY_MinValues[idx];
		const float histLowest_LenY = histLenY_MinValues[idx]; // Get from constant memory


		if (histRange_LenX <= eps || histRange_LenY <= eps) // If the historical range is zero or negative, skip this index to avoid invalid calculations
		{
			std::cout << "Warning: Zero or negative historical range at index " << idx << ". Skipping this index." << std::endl;
			continue;
		}

		const float invHistRange_LenX = 1.0f / fmaxf(histRange_LenX, eps);

		const float invHistRange_LenY = 1.0f / fmaxf(histRange_LenY, eps);

		float score = 0.0f;

		for(int t = 0; t < currPatternLen; t++)
		{
			// Len X inits
			const float histValue_LenX = histLenX_values[idx + t];
			const float histSlope_LenX = histLenX_slopes[idx + t];
			const float currValue_LenX = currLenX_values[t];
			const float currSlope_LenX = currLenX_slopes[t];

			/////////////////////////////////////////////////////////////////////////////////
			// len Y inits
			const float histValue_LenY = histLenY_values[idx + t];
			const float histSlope_LenY = histLenY_slopes[idx + t];
			const float currValue_LenY = currLenY_values[t]; 
			const float currSlope_LenY = currLenY_slopes[t]; 

			score += TwoIndScoreCalc(
				histValue_LenX, histSlope_LenX, currValue_LenX, currSlope_LenX,
				histValue_LenY, histSlope_LenY, currValue_LenY, currSlope_LenY,
				currLowest_LenX, currLowest_LenY, invCurrRange_LenX, invCurrRange_LenY,
				histLowest_LenX, histLowest_LenY, invHistRange_LenX, invHistRange_LenY);
		}
		scores[idx] += score * weight; // Write the final score to global memory
	}

}

void CandidateScan_CPU_TwoInd(
	const int* candidateIndices /*array of candidate indices to process */
	, int candidateCount
	/////////////////////////////////// Hist len X arrays //////////////////////////////////////////////
	, const float* histLenX_values /*array for  Ind1 hist values */					////
	, const float* histLenX_slopes /*array for  Ind1 hist slopes */					////
	////
	, const float* histLenX_MinValues /*array for  Ind1 hist min values */			////
	, const float* histLenX_MaxValues /*array for  Ind1 hist max values */			////
	, const float* histLenX_MinSlopes /*array for  Ind1 hist min slopes */			////
	, const float* histLenX_MaxSlopes /*array for  Ind1 hist max slopes */			////
	////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////// Current len X arrays ///////////////////////////////////////////
	, const float* currLenX_values /*array for Ind1 curr values*/					////
	, const float* currLenX_slopes /* array for Ind1 curr slopes*/					////

	, const float* currLenX_MinValues /* array for Ind1 curr min values*/			////
	, const float* currLenX_MaxValues /* array for Ind1 curr max values*/			////
	, const float* currLenX_MinSlopes /* array for Ind1 curr min slopes*/			////
	, const float* currLenX_MaxSlopes /* array for Ind1 curr max slopes*/			////
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////// Hist len Y arrays //////////////////////////////////////////////
	, const float* histLenY_values /*array for  Ind2 hist values */					////
	, const float* histLenY_slopes /*array for  Ind2 hist slopes */					////
	////
	, const float* histLenY_MinValues /*array for  Ind2 hist min values */			////
	, const float* histLenY_MaxValues /*array for  Ind2 hist max values */			////
	, const float* histLenY_MinSlopes /*array for  Ind2 hist min slopes */			////
	, const float* histLenY_MaxSlopes /*array for  Ind2 hist max slopes */			////
	////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////// Current len Y arrays ///////////////////////////////////////////
	, const float* currLenY_values /* array for Ind2 curr values*/					////
	, const float* currLenY_slopes /* array for Ind2 curr slopes*/					////
	////
	, const float* currLenY_MinValues /* array for Ind2 curr min values*/			////
	, const float* currLenY_MaxValues /* array for Ind2 curr max values*/			////
	, const float* currLenY_MinSlopes /* array for Ind2 curr min slopes*/			////
	, const float* currLenY_MaxSlopes /* array for Ind2 curr max slopes*/			////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	, const int candidateStartIdx, const int candidateEndIdx
	, float* scores /*output scores, where Score(idx) is the score of currMAs(t)&&histMAs(idx + t), t is the index of the current candle */
	, const int currPatternLen, const int outLen, const float weight)
{
	if (currPatternLen <= 0 || outLen <= 0 || candidateStartIdx < 0 || candidateEndIdx <= candidateStartIdx || candidateEndIdx > candidateCount)
	{
		std::cout << "Warning: Invalid input parameters. Skipping scoring for this pattern." << std::endl;
		return;
	}

	const float currRange_LenX = currLenX_MaxValues[0] - currLenX_MinValues[0];
	const float currLowest_LenX = currLenX_MinValues[0]; // Get from constant memory
	const float currRange_LenY = currLenY_MaxValues[0] - currLenY_MinValues[0];
	const float currLowest_LenY = currLenY_MinValues[0]; // Get from constant memory
	const float eps = 1e-6f;

	const float invCurrRange_LenX = 1.0f / fmaxf(currRange_LenX, eps);
	const float invCurrRange_LenY = 1.0f / fmaxf(currRange_LenY, eps);

	if (currRange_LenX <= eps || currRange_LenY <= eps) // If the current range is zero or negative, we cannot normalize properly, so we should skip the scoring for this pattern to avoid invalid calculations
	{
		std::cout << "Warning: Zero or negative current range. Skipping scoring for this pattern." << std::endl;
		return;
	}

	for (int i = candidateStartIdx; i < candidateEndIdx; i++)
	{
		int idx = candidateIndices[i];
		if (idx < 0 || idx >= outLen)
		{
			std::cout << "Warning: Candidate index " << idx << " is out of bounds. Skipping this index." << std::endl;
			continue;
		}

		// Normalize values to [0, 1] range based on the lowest and highest values for MA LenX
		const float histRange_LenX = histLenX_MaxValues[idx] - histLenX_MinValues[idx];
		const float histLowest_LenX = histLenX_MinValues[idx]; // Get from constant memory
		// Normalize values to [0, 1] range based on the lowest and highest values for MA LenY
		const float histRange_LenY = histLenY_MaxValues[idx] - histLenY_MinValues[idx];
		const float histLowest_LenY = histLenY_MinValues[idx]; // Get from constant memory


		if (histRange_LenX <= eps || histRange_LenY <= eps) // If the historical range is zero or negative, skip this index to avoid invalid calculations
		{
			std::cout << "Warning: Zero or negative historical range at index " << idx << ". Skipping this index." << std::endl;
			continue;
		}

		const float invHistRange_LenX = 1.0f / fmaxf(histRange_LenX, eps);

		const float invHistRange_LenY = 1.0f / fmaxf(histRange_LenY, eps);

		float score = 0.0f;

		for (int t = 0; t < currPatternLen; t++)
		{
			// Len X inits
			const float histValue_LenX = histLenX_values[idx + t];
			const float histSlope_LenX = histLenX_slopes[idx + t];
			const float currValue_LenX = currLenX_values[t];
			const float currSlope_LenX = currLenX_slopes[t];

			/////////////////////////////////////////////////////////////////////////////////
			// len Y inits
			const float histValue_LenY = histLenY_values[idx + t];
			const float histSlope_LenY = histLenY_slopes[idx + t];
			const float currValue_LenY = currLenY_values[t];
			const float currSlope_LenY = currLenY_slopes[t];

			score += TwoIndScoreCalc(
				histValue_LenX, histSlope_LenX, currValue_LenX, currSlope_LenX,
				histValue_LenY, histSlope_LenY, currValue_LenY, currSlope_LenY,
				currLowest_LenX, currLowest_LenY, invCurrRange_LenX, invCurrRange_LenY,
				histLowest_LenX, histLowest_LenY, invHistRange_LenX, invHistRange_LenY);
		}
		scores[idx] += score * weight; // Write the final score to global memory
	}

}