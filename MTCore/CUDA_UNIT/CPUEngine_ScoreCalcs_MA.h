#pragma once
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
	, const int currPatternLen, const int outLen, const float weight);

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
	, const int currPatternLen, const int outLen, const float weight);

