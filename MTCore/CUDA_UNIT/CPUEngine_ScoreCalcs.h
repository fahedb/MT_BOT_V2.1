#pragma once

void calcRSI_CPU(const float* d_histRSI_values, const float* d_histRSI_slopes,
	const float* d_currRSI_values, const float* d_currRSI_slopes,
	float* results,
	int patternLen, int histSize, float weight);


void CandidateCalcRSI_CPU(
	const int* candidateIndices,
	const int candidateCount,
	const float* d_histRSI_values, const float* d_histRSI_slopes,
	const float* d_currRSI_values, const float* d_currRSI_slopes,
	float* results,
	int patternLen, int histSize, float weight);


void calcMA_CPU(
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


void CandidateCalcMA_CPU(
	const int* candidateIndices,
	const int candidateCount,
	/////////////////////////////////// Hist MA arrays /////////////////////////////////////////////
	const float* histMA_values, const float* histMA_slopes,									////
	////
	const float* histMA_MinValues, const float* histMA_MaxValues,							////
	const float* histMA_MinSlopes, const float* histMA_MaxSlopes,							////
	////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Current MA arrays /////////////////////////////////////////////
	const float* currMA_values, const float* currMA_slopes,									////
	////
	const float* currMA_MinValues, const float* currMA_MaxValues,							////
	const float* currMA_MinSlopes, const float* currMA_MaxSlopes,							////
	////////////////////////////////////////////////////////////////////////////////////////////////
	float* results,
	int patternLen, int histSize, float weight);


void TwoIndCalc_CPU(
	//////////////////////////////////// Hist len X arrays /////////////////////////////////////////////
	const float* histIndX_values, const float* histIndX_slopes							////
	, const float* histIndX_MinValues, const float* histIndX_MaxValues						////
	, const float* histIndX_MinSlopes, const float* histIndX_MaxSlopes						////
	////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////// Current len X arrays //////////////////////////////////////////
	, const float* currIndX_values, const float* currIndX_slopes							////
	, const float* currIndX_MinValues, const float* currIndX_MaxValues						////
	, const float* currIndX_MinSlopes, const float* currIndX_MaxSlopes						////
	////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////// Hist len Y arrays /////////////////////////////////////////////
	, const float* histIndY_values, const float* histIndY_slopes								////
	, const float* histIndY_MinValues, const float* histIndY_MaxValues						////
	, const float* histIndY_MinSlopes, const float* histIndY_MaxSlopes						////
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////// Current len Y arrays //////////////////////////////////////////
	, const float* currIndY_values, const float* currIndY_slopes								////
	, const float* currIndY_MinValues, const float* currIndY_MaxValues						////
	, const float* currIndY_MinSlopes, const float* currIndY_MaxSlopes						////
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	, float* results,
	int patternLen, int histSize, float weight);


void CandidateTwoIndCalc_CPU(
	const int* candidateIndices,
	const int candidateCount,
	//////////////////////////////////// Hist len X arrays /////////////////////////////////////////////
	const float* histIndX_values, const float* histIndX_slopes							////
	, const float* histIndX_MinValues, const float* histIndX_MaxValues						////
	, const float* histIndX_MinSlopes, const float* histIndX_MaxSlopes						////
	////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////// Current len X arrays //////////////////////////////////////////
	, const float* currIndX_values, const float* currIndX_slopes							////
	, const float* currIndX_MinValues, const float* currIndX_MaxValues						////
	, const float* currIndX_MinSlopes, const float* currIndX_MaxSlopes						////
	////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////// Hist len Y arrays /////////////////////////////////////////////
	, const float* histIndY_values, const float* histIndY_slopes								////
	, const float* histIndY_MinValues, const float* histIndY_MaxValues						////
	, const float* histIndY_MinSlopes, const float* histIndY_MaxSlopes						////
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////// Current len Y arrays //////////////////////////////////////////
	, const float* currIndY_values, const float* currIndY_slopes								////
	, const float* currIndY_MinValues, const float* currIndY_MaxValues						////
	, const float* currIndY_MinSlopes, const float* currIndY_MaxSlopes						////
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	, float* results,
	int patternLen, int histSize, float weight);

