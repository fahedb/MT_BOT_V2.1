#include "KernelEngine_ScoreCalcs_General.h"


// MAs score calcs
__device__ void Find_MaxMinValues(const float* __restrict__ d_values, int d_size, float* results)
{
	// find min max and init in results[0] and results[1]
	float min = 1000000.0f;
	float max = 0.0f;
	for (int i = 0; i < d_size; i++)
	{
		float val = d_values[i];
		if (val < min) min = val;
		if (val > max) max = val;
	}
	results[0] = min;
	results[1] = max;
}

// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------

// Inline helper for pair score calculation
// Calculates the combined score for a pair of (value, slope)
// This function uses RelativeScoreCalc for both value and slope comparisons
// Can be used for RSI, MAs, etc.
// Do not forget to multiply by weight outside this function
__device__
float pair_score(float hVal, float cVal, float hSlp, float cSlp) {
	float s = RelativeScoreCalc(hVal, cVal);
	s += RelativeScoreCalc(hSlp, cSlp);
	return s;
}

// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------

// CPP:
//{
//	if (hist == curr)
//		return static_cast<int>(weight);
//
//	const double absHist = std::abs(hist);
//	const double absCurr = std::abs(curr);
//	const double diff = std::abs(hist - curr);
//	const double maxV = absHist > absCurr ? absHist : absCurr;
//
//	const double matching = std::fmax(0.0, 1.0 - (diff / maxV));
//	return static_cast<int>(matching * weight);
//}
__device__ float RelativeScoreCalc(float hist, float curr) // CUDA device function
{

	if (hist == curr)
		return 1;
	const float absHist = fabsf(hist);
	const float absCurr = fabsf(curr);
	const float diff = fabsf(hist - curr);
	const float maxV = absHist > absCurr ? absHist : absCurr;
	const float matching = fmaxf(0.0f, 1.0f - (diff / maxV));
	return matching;
}