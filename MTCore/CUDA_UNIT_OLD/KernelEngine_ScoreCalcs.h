#pragma once

////////////////////////////////////////////////////////////////////////////////
void TwoIndCalc(float* d_histIndX_values, float* d_histIndX_slopes, float* d_currIndX_values, float* d_currIndX_slopes,
				float* d_histIndY_values, float* d_histIndY_slopes, float* d_currIndY_values, float* d_currIndY_slopes,
				float* d_results, int patternLen, int histSize, float weight);
void calcMA(float* d_histMA_values, float* d_histMA_slopes, float* d_currMA_values, float* d_currMA_slopes, float* d_results, int patternLen, int histSize, float weight);
void calcRSI(float* d_histRSI_values, float* d_histRSI_slopes, float* d_currRSI_values, float* d_currRSI_slopes, float* results, int patternLen, int histSize, float weight);