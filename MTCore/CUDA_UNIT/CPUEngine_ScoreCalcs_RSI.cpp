#include "KernelEngine_ScoreCalcs_General.h"
#include <iostream>

void FullScan_CPU_RSI(const float*  histRSI_values /*array for rsi hist values */
	, const float* histRSI_slopes /*array for rsi hist slopes */
	, const float* currRSI_values /* array for rsi curr values*/
	, const float* currRSI_slopes /* array for rsi curr slopes*/
	, float* scores /*output scores, where Score(idx) is the score of currRSI(t)&&histRSI(idx + t), t is the index of the current candle */
	, const int startIdx, const int endIdx
	, const int currPatternLen, const int outLen, const float weight)
{
	for (int i = startIdx; i < endIdx; i++)
	{
		float score = 0.0f;
		for (int t = 0; t < currPatternLen; t++)
		{
			score += pair_score(histRSI_values[i + t], currRSI_values[t], histRSI_slopes[i + t], currRSI_slopes[t]);
		}
		scores[i] += score * weight;
	}
}

// Candidate scan kernel for RSI calculation, where only the candidate indices will be processed to calculate their scores
void CandidateScan_CPU_RSI(
	const int* candidateIndices /*array of candidate indices to process */
	, int candidateCount
	, const int startIdxForIndices, const int endIdxForIndices
	, const float* histRSI_values /*array for rsi hist values */
	, const float* histRSI_slopes /*array for rsi hist slopes */
	, const float* currRSI_values /* array for rsi curr values*/
	, const float* currRSI_slopes /* array for rsi curr slopes*/
	, float* scores /*output scores, where Score(idx) is the score of currRSI(t)&&histRSI(idx + t), t is the index of the current candle */
	, const int currPatternLen, const int outLen, const float weight)
{
	if(startIdxForIndices < 0 || endIdxForIndices > candidateCount || startIdxForIndices >= endIdxForIndices)
	{
		std::cout << "Invalid index range for CandidateScan_CPU_RSI" << std::endl;
		return;
	}

	for (int i = startIdxForIndices; i < endIdxForIndices; i++)
	{
		int idx = candidateIndices[i];
		if ((unsigned)idx >= (unsigned)outLen) continue;

		float score = 0.0f;
		for (int t = 0; t < currPatternLen; t++)
		{
			score += pair_score(histRSI_values[idx + t], currRSI_values[t], histRSI_slopes[idx + t], currRSI_slopes[t]);
		}
		scores[idx] += score * weight;

	}
}