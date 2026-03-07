#pragma once

////////////////////////////////////////////////////////////////////////////////
void calcRSI(const float* d_histRSI_values, const float* d_histRSI_slopes, 
			 const float* d_currRSI_values, const float* d_currRSI_slopes, 
			float* results,
			int patternLen, int histSize, float weight);
void CandidateCalcRSI(
	const int* d_candidateIndices,
	const int candidateCount,
	const float* d_histRSI_values, const float* d_histRSI_slopes,
	const float* d_currRSI_values, const float* d_currRSI_slopes,
	float* d_results,
	int patternLen, int histSize, float weight);


void calcMA(
	/////////////////////////////////// Hist MA arrays /////////////////////////////////////////////
	const float* d_histMA_values, const float* d_histMA_slopes,									////
																								////
	const float* d_histMA_MinValues, const float* d_histMA_MaxValues,							////
	const float* d_histMA_MinSlopes, const float* d_histMA_MaxSlopes,							////
	////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////// Current MA arrays //////////////////////////////////////////
	const float* d_currMA_values, const float* d_currMA_slopes,									////
																								////
	const float* d_currMA_MinValues, const float* d_currMA_MaxValues,							////
	const float* d_currMA_MinSlopes, const float* d_currMA_MaxSlopes,							////
	////////////////////////////////////////////////////////////////////////////////////////////////

	float* d_results,
	int patternLen, int histSize, float weight);

void CandidateCalcMA(
	const int* d_candidateIndices,
	const int candidateCount,
	/////////////////////////////////// Hist MA arrays /////////////////////////////////////////////
	const float* d_histMA_values, const float* d_histMA_slopes,									////
	////
	const float* d_histMA_MinValues, const float* d_histMA_MaxValues,							////
	const float* d_histMA_MinSlopes, const float* d_histMA_MaxSlopes,							////
	////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Current MA arrays /////////////////////////////////////////////
	const float* d_currMA_values, const float* d_currMA_slopes,									////
	////
	const float* d_currMA_MinValues, const float* d_currMA_MaxValues,							////
	const float* d_currMA_MinSlopes, const float* d_currMA_MaxSlopes,							////
	////////////////////////////////////////////////////////////////////////////////////////////////
	float* d_results,
	int patternLen, int histSize, float weight);


void TwoIndCalc(
	//////////////////////////////////// Hist len X arrays /////////////////////////////////////////////
		const float* d_histIndX_values,		const float* d_histIndX_slopes							////
	,	const float* d_histIndX_MinValues,	const float* d_histIndX_MaxValues						////
	,	const float* d_histIndX_MinSlopes,	const float* d_histIndX_MaxSlopes						////
	////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////// Current len X arrays //////////////////////////////////////////
	,	const float* d_currIndX_values,		const float* d_currIndX_slopes							////
	,	const float* d_currIndX_MinValues,	const float* d_currIndX_MaxValues						////
	,	const float* d_currIndX_MinSlopes,	const float* d_currIndX_MaxSlopes						////
	////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////// Hist len Y arrays /////////////////////////////////////////////
	,	const float* d_histIndY_values, const float* d_histIndY_slopes								////
	,	const float* d_histIndY_MinValues, const float* d_histIndY_MaxValues						////
	,	const float* d_histIndY_MinSlopes, const float* d_histIndY_MaxSlopes						////
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////// Current len Y arrays //////////////////////////////////////////
	,	const float* d_currIndY_values, const float* d_currIndY_slopes								////
	,	const float* d_currIndY_MinValues, const float* d_currIndY_MaxValues						////
	,	const float* d_currIndY_MinSlopes, const float* d_currIndY_MaxSlopes						////
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	,	float* d_results,
	int patternLen, int histSize, float weight);


void CandidateTwoIndCalc(
	const int* d_candidateIndices,
	const int candidateCount,
	//////////////////////////////////// Hist len X arrays /////////////////////////////////////////////
	const float* d_histIndX_values, const float* d_histIndX_slopes							////
	, const float* d_histIndX_MinValues, const float* d_histIndX_MaxValues						////
	, const float* d_histIndX_MinSlopes, const float* d_histIndX_MaxSlopes						////
	////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////// Current len X arrays //////////////////////////////////////////
	, const float* d_currIndX_values, const float* d_currIndX_slopes							////
	, const float* d_currIndX_MinValues, const float* d_currIndX_MaxValues						////
	, const float* d_currIndX_MinSlopes, const float* d_currIndX_MaxSlopes						////
	////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////// Hist len Y arrays /////////////////////////////////////////////
	, const float* d_histIndY_values, const float* d_histIndY_slopes								////
	, const float* d_histIndY_MinValues, const float* d_histIndY_MaxValues						////
	, const float* d_histIndY_MinSlopes, const float* d_histIndY_MaxSlopes						////
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////// Current len Y arrays //////////////////////////////////////////
	, const float* d_currIndY_values, const float* d_currIndY_slopes								////
	, const float* d_currIndY_MinValues, const float* d_currIndY_MaxValues						////
	, const float* d_currIndY_MinSlopes, const float* d_currIndY_MaxSlopes						////
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	, float* d_results,
	int patternLen, int histSize, float weight);
////////////////////////////////////////////////////////////////////////////////