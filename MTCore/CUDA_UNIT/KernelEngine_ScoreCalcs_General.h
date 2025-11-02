#pragma once
#include "CudaMemoryController.h"

__device__ float RelativeScoreCalc(float hist, float curr); // CUDA device function
__device__ void Find_MaxMinValues(const float* __restrict__ d_values, int d_size, float* results);
__device__ float pair_score(float hVal, float cVal, float hSlp, float cSlp);
