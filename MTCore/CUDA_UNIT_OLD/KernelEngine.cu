#include <cuda_runtime.h>
#include "device_launch_parameters.h"
#include <chrono>
#include "KernelEngine.h"
#include <iostream>

__global__ void normalizeKernel(float* d_in, float* d_out, int N) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < N) {
        d_out[idx] = d_in[idx] / 255.0f;
    }
}

void RegularNormalize(float* in, float* out, int N) {
    for (int i = 0; i < N; i++) {
        out[i] = in[i] / 255.0f;
    }
}

void TestingFunction()
{
	std::cout << "TestingFunction called!" << std::endl;

    const int N = 4000000;  // גודל המערך
	float* h_in = new float[N];  // מערכים ב-host
	float* h_out = new float[N];

	std::cout << "Array size: " << N << std::endl;

    // אתחול המערך (מילוי נתונים)
    for (int i = 0; i < N; i++) {
        h_in[i] = float(i) * 2;  // נתונים לדוגמה (0, 10, 20, 30, ...)
    }

	std::cout << "Initialization done!" << std::endl;
	int mySizeinBytes = N * sizeof(float);
	std::cout << "Array size in bytes: " << mySizeinBytes << " bytes" << std::endl;

    // אלוקציה ב-GPU
    float* d_in, * d_out;
    cudaMalloc(&d_in, N * sizeof(float));  // יצירת זיכרון ב-GPU
    cudaMalloc(&d_out, N * sizeof(float));

    // העתקת נתונים מ-host ל-GPU
    cudaMemcpy(d_in, h_in, N * sizeof(float), cudaMemcpyHostToDevice);

    // קריאה ל-kernel
    int blockSize = 256;  // מספר ה-threads לכל block
    int numBlocks = (N + blockSize - 1) / blockSize;  // חישוב מספר ה-blocks
    // calc time
	auto start = std::chrono::high_resolution_clock::now();
    normalizeKernel << <numBlocks, blockSize >> > (d_in, d_out, N);  // הרצת ה-kernel
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration = end - start;
	std::cout << "Kernel execution time: " << duration.count() << " ms" << std::endl;
    // העתקת התוצאות מ-GPU ל-host
    cudaMemcpy(h_out, d_out, N * sizeof(float), cudaMemcpyDeviceToHost);

	auto start2 = std::chrono::high_resolution_clock::now();
	RegularNormalize(h_in, h_out, N);
	auto end2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration2 = end2 - start2;
	std::cout << "Regular execution time: " << duration2.count() << " ms" << std::endl;
    // הדפסת התוצאות
    //for (int i = 0; i < N; i++) {
    //    std::cout << "Input: " << h_in[i] << " => Normalized: " << h_out[i] << std::endl;
    //}

    // ניקוי הזיכרון ב-GPU
    cudaFree(d_in);
    cudaFree(d_out);
}
