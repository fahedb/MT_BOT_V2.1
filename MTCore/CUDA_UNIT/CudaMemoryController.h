#pragma once
#include <cuda_runtime.h>
#include "device_launch_parameters.h"
#include <cuda_runtime_api.h>

#include <iostream>

template<typename T>
void CudaMemoryAllocation(T** myVar, int varNum)
{
	cudaError_t lastError = cudaGetLastError();
	if (lastError != cudaSuccess) {
		std::cout << "\nPrevious CUDA error: " << cudaGetErrorString(lastError) << std::endl;
	}
	cudaError_t cudaMallocError = cudaMalloc((void**)myVar, varNum * sizeof(T));
	if (cudaMallocError != cudaSuccess)
	{
		std::cerr << "CUDA malloc failed: " << cudaGetErrorString(cudaMallocError) << std::endl;
		exit(EXIT_FAILURE);
	}
}
template<typename T>
void CudaMemoryCopyToHost(T* hostVar, T* deviceVar, int varNum)
{
	cudaError_t lastError = cudaGetLastError();
	if (lastError != cudaSuccess) {
		std::cout << "\nPrevious CUDA error: " << cudaGetErrorString(lastError) << std::endl;
	}
	cudaError_t cudaMemcpyError = cudaMemcpy(hostVar, deviceVar, varNum * sizeof(T), cudaMemcpyDeviceToHost);
	if (cudaMemcpyError != cudaSuccess)
	{
		std::cerr << "CUDA memcpy to host failed: " << cudaGetErrorString(cudaMemcpyError) << std::endl;
		exit(EXIT_FAILURE);
	}
}
template<typename T>
void CudaMemoryCopyToDevice(T* deviceVar, T* hostVar, int varNum)
{
	cudaError_t lastError = cudaGetLastError();
	if (lastError != cudaSuccess) {
		std::cout << "\nPrevious CUDA error: " << cudaGetErrorString(lastError) << std::endl;
	}
	cudaError_t cudaMemcpyError = cudaMemcpy(deviceVar, hostVar, varNum * sizeof(T), cudaMemcpyHostToDevice);
	if (cudaMemcpyError != cudaSuccess)
	{
		std::cerr << "CUDA memcpy to device failed: " << cudaGetErrorString(cudaMemcpyError) << std::endl;
		exit(EXIT_FAILURE);
	}
}