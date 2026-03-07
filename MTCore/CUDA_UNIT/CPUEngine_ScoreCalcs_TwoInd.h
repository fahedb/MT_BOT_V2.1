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
	, const int currPatternLen, const int outLen, const float weight);

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
	, const int currPatternLen, const int outLen, const float weight);