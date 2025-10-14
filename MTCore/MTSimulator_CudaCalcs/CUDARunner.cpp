#include "CUDARunner.h"
#include "CPP_To_Cuda_Tunnel.h"

void CUDARunner::RunTest()
{
	CPP_To_Cuda_Tunnel::MyTestFunction();
}

void CUDARunner::RunRSITest()
{
	CPP_To_Cuda_Tunnel::RSI_ScoreCalcs_TestFunction();
}
