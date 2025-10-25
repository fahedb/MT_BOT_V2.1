#pragma once

////////////////////////////////////////////////////////////////////////////////
void TwoIndCalc(float* d_histIndX_values, float* d_histIndX_slopes, float* d_currIndX_values, float* d_currIndX_slopes,
				float* d_histIndY_values, float* d_histIndY_slopes, float* d_currIndY_values, float* d_currIndY_slopes,
				float* d_results, int patternLen, int histSize, float weight);
void calcMA(float* d_histMA_values, float* d_histMA_slopes, float* d_currMA_values, float* d_currMA_slopes, float* d_results, int patternLen, int histSize, float weight);
void calcRSI(float* d_histRSI_values, float* d_histRSI_slopes, float* d_currRSI_values, float* d_currRSI_slopes, float* results, int patternLen, int histSize, float weight);

void calcRSI_tiled(const float* d_histVal, const float* d_histSlope,
	const float* d_currVal, const float* d_currSlope,
	float* d_resOut, int currPatternLen, int histSize, float weight);

void calcMA_tiled(const float* d_histMA_values,
	const float* d_histMA_slopes,
	const float* d_currMA_values,
	const float* d_currMA_slopes,
	float* d_results,
	int patternLen, int histSize, float weight);

void TwoIndCalc_tiled(const float* d_hist_LenX_Val, const float* d_hist_LenX_Slope,
	const float* d_curr_LenX_Val, const float* d_curr_LenX_Slope,
	const float* d_hist_LenY_Val, const float* d_hist_LenY_Slope,
	const float* d_curr_LenY_Val, const float* d_curr_LenY_Slope,
	float* d_results, int K, int histSize, float weight);