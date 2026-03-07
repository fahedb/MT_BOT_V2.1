#pragma once
void FullScan_CPU_RSI(const float* histRSI_values /*array for rsi hist values */
	, const float* histRSI_slopes /*array for rsi hist slopes */
	, const float* currRSI_values /* array for rsi curr values*/
	, const float* currRSI_slopes /* array for rsi curr slopes*/
	, float* scores /*output scores, where Score(idx) is the score of currRSI(t)&&histRSI(idx + t), t is the index of the current candle */
	, const int startIdx, const int endIdx
	, const int currPatternLen, const int outLen, const float weight);

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
	, const int currPatternLen, const int outLen, const float weight);