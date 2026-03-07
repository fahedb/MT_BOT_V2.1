#include "KernelEngine_ScoreCalcs_General.h"

#include "CPUEngine_ScoreCalcs.h"

#include "CPUEngine_ScoreCalcs_RSI.h"
#include "CPUEngine_ScoreCalcs_MA.h"
#include "CPUEngine_ScoreCalcs_TwoInd.h"

#include "ThreadPool.h"
#include <vector>
#include <future>

#define MINIMUM_CANDIDATE_COUNT_PER_THREAD 1000

void calcRSI_CPU(const float* d_histRSI_values, const float* d_histRSI_slopes,
const float* d_currRSI_values, const float* d_currRSI_slopes,
float* results,
int patternLen, int histSize, float weight)
{
	MTThreadPool* threadPool = MTThreadPool::getInstance();
	threadPool->Set_PauseLowPriorityTasks(true); // Pause low priority tasks to prioritize this CPU-bound work

	int threadCount = threadPool->Get_ThreadsNum();
	int outLen = histSize - patternLen + 1;
	int chunkSize = outLen / threadCount;
	std::vector<std::future<void>> futures;

	for (int threadIdx = 0; threadIdx < threadCount; threadIdx++)
	{

		int startIdx = threadIdx * chunkSize;
		int endIdx = (threadIdx == threadCount - 1) ? outLen : startIdx + chunkSize;

		auto prom = std::make_shared<std::promise<void>>();
		futures.push_back(prom->get_future()); // Store the future to wait for completion later

		threadPool->enqueueTask([d_histRSI_values, d_histRSI_slopes, d_currRSI_values, d_currRSI_slopes
			, results, startIdx, endIdx, patternLen, outLen, weight, prom]() {
				/////////////////////////////////////////////////////////////////////////////////////////
				try {

					FullScan_CPU_RSI(d_histRSI_values, d_histRSI_slopes, d_currRSI_values, d_currRSI_slopes
						, results, startIdx, endIdx, patternLen, outLen, weight);
					prom->set_value(); // Signal that this thread's work is done
				}
				catch (const std::exception& ex) { // Catch any standard exceptions
					// Handle exceptions and set the promise to avoid hanging
					std::cerr << "Exception in thread: " << ex.what() << std::endl;
					prom->set_exception(std::current_exception()); // Set the exception to the promise to propagate it to the future
				}
				/////////////////////////////////////////////////////////////////////////////////////////
			}, TaskPriority::High);

	}
	for (auto& f : futures)
		f.get(); // Wait till the end
	threadPool->Set_PauseLowPriorityTasks(false); // Resume low priority tasks after this CPU-bound work is done

}

void CandidateCalcRSI_CPU(
	const int* candidateIndices,
	const int candidateCount,
	const float* d_histRSI_values, const float* d_histRSI_slopes,
	const float* d_currRSI_values, const float* d_currRSI_slopes,
	float* results,
	int patternLen, int histSize, float weight)
{
	MTThreadPool* threadPool = MTThreadPool::getInstance();
	threadPool->Set_PauseLowPriorityTasks(true); // Pause low priority tasks to prioritize this CPU-bound work

	int threadCount = threadPool->Get_ThreadsNum();
	int chunkSize = candidateCount / threadCount;

	if (chunkSize < MINIMUM_CANDIDATE_COUNT_PER_THREAD) // If the chunk size is too small, reduce the number of threads to avoid overhead and ensure efficient processing
	{
		chunkSize = MINIMUM_CANDIDATE_COUNT_PER_THREAD; // Ensure we have a minimum number of candidates per thread to avoid overhead
		threadCount = (candidateCount + chunkSize - 1) / chunkSize; // Recalculate thread count based on the new chunk size
	}

	std::vector<std::future<void>> futures;
	for(int threadIdx = 0; threadIdx < threadCount; threadIdx++)
	{
		int startIdx = threadIdx * chunkSize;
		int endIdx = std::min(startIdx + chunkSize, candidateCount);
		auto prom = std::make_shared<std::promise<void>>();
		int outLen = histSize - patternLen + 1;
		futures.push_back(prom->get_future()); // Store the future to wait for completion later

		threadPool->enqueueTask([candidateIndices,candidateCount ,d_histRSI_values, d_histRSI_slopes, d_currRSI_values, d_currRSI_slopes
								,results, startIdx, endIdx, patternLen, outLen, weight, prom]() {
				/////////////////////////////////////////////////////////////////////////////////////////
				try { // Wrap the work in a try-catch block to handle exceptions gracefully

					CandidateScan_CPU_RSI(candidateIndices, candidateCount
						, startIdx, endIdx
						, d_histRSI_values, d_histRSI_slopes, d_currRSI_values, d_currRSI_slopes
						, results, patternLen, outLen, weight);

					prom->set_value(); // Signal that this thread's work is done
				}
				catch (const std::exception& ex) { // Catch any standard exceptions
					// Handle exceptions and set the promise to avoid hanging
					std::cerr << "Exception in thread: " << ex.what() << std::endl;
					prom->set_exception(std::current_exception()); // Set the exception to the promise to propagate it to the future
				}
				/////////////////////////////////////////////////////////////////////////////////////////
			}, TaskPriority::High);
	}
	for (auto& f : futures)
		f.get(); // Wait till the end
	threadPool->Set_PauseLowPriorityTasks(false); // Resume low priority tasks pause after this CPU-bound work is done
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


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
	int patternLen, int histSize, float weight)
{
	MTThreadPool* threadPool = MTThreadPool::getInstance();
	threadPool->Set_PauseLowPriorityTasks(true); // Pause low priority tasks to prioritize this CPU-bound work

	int threadCount = threadPool->Get_ThreadsNum();
	int outLen = histSize - patternLen + 1;
	int chunkSize = outLen / threadCount;
	std::vector<std::future<void>> futures;

	for (int threadIdx = 0; threadIdx < threadCount; threadIdx++)
	{

		int startIdx = threadIdx * chunkSize;
		int endIdx = (threadIdx == threadCount - 1) ? outLen : startIdx + chunkSize;

		auto prom = std::make_shared<std::promise<void>>();
		futures.push_back(prom->get_future()); // Store the future to wait for completion later

		threadPool->enqueueTask([d_histMA_values, d_histMA_slopes, d_histMA_MinValues, d_histMA_MaxValues, d_histMA_MinSlopes, d_histMA_MaxSlopes,
			d_currMA_values, d_currMA_slopes, d_currMA_MinValues, d_currMA_MaxValues, d_currMA_MinSlopes, d_currMA_MaxSlopes,
			d_results, startIdx, endIdx, patternLen, outLen, weight, prom]() {
				/////////////////////////////////////////////////////////////////////////////////////////
				try {
					FullScan_CPU_MA(d_histMA_values, d_histMA_slopes,
						d_histMA_MinValues, d_histMA_MaxValues,
						d_histMA_MinSlopes, d_histMA_MaxSlopes,
						d_currMA_values, d_currMA_slopes,
						d_currMA_MinValues, d_currMA_MaxValues,
						d_currMA_MinSlopes, d_currMA_MaxSlopes,
						d_results, startIdx, endIdx, patternLen, outLen, weight);
					prom->set_value(); // Signal that this thread's work is done
				}
				catch (const std::exception& ex) { // Catch any standard exceptions
					// Handle exceptions and set the promise to avoid hanging
					std::cerr << "Exception in thread: " << ex.what() << std::endl;
					prom->set_exception(std::current_exception()); // Set the exception to the promise to propagate it to the future
				}
				/////////////////////////////////////////////////////////////////////////////////////////
			}, TaskPriority::High);
	}
	for (auto& f : futures)
		f.get(); // Wait till the end
	threadPool->Set_PauseLowPriorityTasks(false); // Resume low priority tasks after this CPU-bound work is done

}


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
	int patternLen, int histSize, float weight)
{
	MTThreadPool* threadPool = MTThreadPool::getInstance();
	threadPool->Set_PauseLowPriorityTasks(true); // Pause low priority tasks to prioritize this CPU-bound work

	int threadCount = threadPool->Get_ThreadsNum();
	int chunkSize = candidateCount / threadCount;

	if (chunkSize < MINIMUM_CANDIDATE_COUNT_PER_THREAD) // If the chunk size is too small, reduce the number of threads to avoid overhead and ensure efficient processing
	{
		chunkSize = MINIMUM_CANDIDATE_COUNT_PER_THREAD; // Ensure we have a minimum number of candidates per thread to avoid overhead
		threadCount = (candidateCount + chunkSize - 1) / chunkSize; // Recalculate thread count based on the new chunk size
	}

	std::vector<std::future<void>> futures;
	for (int threadIdx = 0; threadIdx < threadCount; threadIdx++)
	{
		int startIdx = threadIdx * chunkSize;
		int endIdx = std::min(startIdx + chunkSize, candidateCount);
		auto prom = std::make_shared<std::promise<void>>();
		int outLen = histSize - patternLen + 1;
		futures.push_back(prom->get_future()); // Store the future to wait for completion later

		threadPool->enqueueTask([candidateIndices, candidateCount
			, histMA_values, histMA_slopes, histMA_MinValues, histMA_MaxValues, histMA_MinSlopes, histMA_MaxSlopes,
			currMA_values, currMA_slopes, currMA_MinValues, currMA_MaxValues, currMA_MinSlopes, currMA_MaxSlopes,
			results, startIdx, endIdx, patternLen, outLen, weight, prom]() {
				/////////////////////////////////////////////////////////////////////////////////////////
				try { // Wrap the work in a try-catch block to handle exceptions gracefully
					CandidateScan_CPU_MA(candidateIndices, candidateCount,
						histMA_values, histMA_slopes,
						histMA_MinValues, histMA_MaxValues,
						histMA_MinSlopes, histMA_MaxSlopes,
						currMA_values, currMA_slopes,
						currMA_MinValues, currMA_MaxValues,
						currMA_MinSlopes, currMA_MaxSlopes,
						results, startIdx, endIdx, patternLen, outLen, weight);
					prom->set_value(); // Signal that this thread's work is done
				}
				catch (const std::exception& ex) { // Catch any standard exceptions
					// Handle exceptions and set the promise to avoid hanging
					std::cerr << "Exception in thread: " << ex.what() << std::endl;
					prom->set_exception(std::current_exception()); // Set the exception to the promise to propagate it to the future
				}
				/////////////////////////////////////////////////////////////////////////////////////////
			}, TaskPriority::High);
	}
	for (auto& f : futures)
		f.get(); // Wait till the end
	threadPool->Set_PauseLowPriorityTasks(false); // Resume low priority tasks after this CPU-bound work is done
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////


void TwoIndCalc_CPU(
	//////////////////////////////////// Hist len X arrays /////////////////////////////////////////////
	const float*   histIndX_values, const float* histIndX_slopes							////
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
	int patternLen, int histSize, float weight)
{
	MTThreadPool* threadPool = MTThreadPool::getInstance();
	threadPool->Set_PauseLowPriorityTasks(true); // Pause low priority tasks to prioritize this CPU-bound work

	int threadCount = threadPool->Get_ThreadsNum();
	int outLen = histSize - patternLen + 1;
	int chunkSize = outLen / threadCount;
	std::vector<std::future<void>> futures;

	for (int threadIdx = 0; threadIdx < threadCount; threadIdx++)
	{

		int startIdx = threadIdx * chunkSize;
		int endIdx = (threadIdx == threadCount - 1) ? outLen : startIdx + chunkSize;

		auto prom = std::make_shared<std::promise<void>>();
		futures.push_back(prom->get_future()); // Store the future to wait for completion later

		threadPool->enqueueTask([histIndX_values, histIndX_slopes, histIndX_MinValues, histIndX_MaxValues, histIndX_MinSlopes, histIndX_MaxSlopes,
			currIndX_values, currIndX_slopes, currIndX_MinValues, currIndX_MaxValues, currIndX_MinSlopes, currIndX_MaxSlopes,
			histIndY_values, histIndY_slopes, histIndY_MinValues, histIndY_MaxValues, histIndY_MinSlopes, histIndY_MaxSlopes,
			currIndY_values, currIndY_slopes, currIndY_MinValues, currIndY_MaxValues, currIndY_MinSlopes, currIndY_MaxSlopes,
			results, startIdx, endIdx, patternLen, outLen, weight, prom]() {
				/////////////////////////////////////////////////////////////////////////////////////////
				try {
					FullScan_CPU_TwoInd(
						histIndX_values, histIndX_slopes,
						histIndX_MinValues, histIndX_MaxValues,
						histIndX_MinSlopes, histIndX_MaxSlopes,
						currIndX_values, currIndX_slopes,
						currIndX_MinValues, currIndX_MaxValues,
						currIndX_MinSlopes, currIndX_MaxSlopes,
						histIndY_values, histIndY_slopes,
						histIndY_MinValues, histIndY_MaxValues,
						histIndY_MinSlopes, histIndY_MaxSlopes,
						currIndY_values, currIndY_slopes,
						currIndY_MinValues, currIndY_MaxValues,
						currIndY_MinSlopes, currIndY_MaxSlopes,
						startIdx, endIdx, results, patternLen, outLen, weight);
					prom->set_value(); // Signal that this thread's work is done
				}
				catch (const std::exception& ex) { // Catch any standard exceptions
					std::cerr << "Exception in thread: " << ex.what() << std::endl;
					prom->set_exception(std::current_exception()); // Set the)
				}
			}, TaskPriority::High);
	}
	for (auto& f : futures)
		f.get(); // Wait till the end
	threadPool->Set_PauseLowPriorityTasks(false); // Resume low priority tasks after this CPU-bound work is done
}

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
	int patternLen, int histSize, float weight)
{
	MTThreadPool* threadPool = MTThreadPool::getInstance();
	threadPool->Set_PauseLowPriorityTasks(true); // Pause low priority tasks to prioritize this CPU-bound work

	int threadCount = threadPool->Get_ThreadsNum();
	int chunkSize = candidateCount / threadCount;

	if (chunkSize < MINIMUM_CANDIDATE_COUNT_PER_THREAD) // If the chunk size is too small, reduce the number of threads to avoid overhead and ensure efficient processing
	{
		chunkSize = MINIMUM_CANDIDATE_COUNT_PER_THREAD; // Ensure we have a minimum number of candidates per thread to avoid overhead
		threadCount = (candidateCount + chunkSize - 1) / chunkSize; // Recalculate thread count based on the new chunk size
	}

	std::vector<std::future<void>> futures;
	for (int threadIdx = 0; threadIdx < threadCount; threadIdx++)
	{
		int startIdx = threadIdx * chunkSize;
		int endIdx = std::min(startIdx + chunkSize, candidateCount);
		auto prom = std::make_shared<std::promise<void>>();
		int outLen = histSize - patternLen + 1;
		futures.push_back(prom->get_future()); // Store the future to wait for completion later

		threadPool->enqueueTask([candidateIndices, candidateCount, histIndX_values, histIndX_slopes, histIndX_MinValues, histIndX_MaxValues, histIndX_MinSlopes, histIndX_MaxSlopes,
			currIndX_values, currIndX_slopes, currIndX_MinValues, currIndX_MaxValues, currIndX_MinSlopes, currIndX_MaxSlopes,
			histIndY_values, histIndY_slopes, histIndY_MinValues, histIndY_MaxValues, histIndY_MinSlopes, histIndY_MaxSlopes,
			currIndY_values, currIndY_slopes, currIndY_MinValues, currIndY_MaxValues, currIndY_MinSlopes, currIndY_MaxSlopes,
			results, startIdx, endIdx, patternLen, outLen, weight, prom]() {
				/////////////////////////////////////////////////////////////////////////////////////////
				try { // Wrap the work in a try-catch block to handle exceptions gracefully
					CandidateScan_CPU_TwoInd(candidateIndices, candidateCount,
						histIndX_values, histIndX_slopes,
						histIndX_MinValues, histIndX_MaxValues,
						histIndX_MinSlopes, histIndX_MaxSlopes,
						currIndX_values, currIndX_slopes,
						currIndX_MinValues, currIndX_MaxValues,
						currIndX_MinSlopes, currIndX_MaxSlopes,
						histIndY_values, histIndY_slopes,
						histIndY_MinValues, histIndY_MaxValues,
						histIndY_MinSlopes, histIndY_MaxSlopes,
						currIndY_values, currIndY_slopes,
						currIndY_MinValues, currIndY_MaxValues,
						currIndY_MinSlopes, currIndY_MaxSlopes,
						startIdx, endIdx, results, patternLen, outLen, weight);
					prom->set_value(); // Signal that this thread's work is done
				}
				catch (const std::exception& ex) { // Catch any standard exceptions
					// Handle exceptions and set the promise to avoid hanging
					std::cerr << "Exception in thread: " << ex.what() << std::endl;
					prom->set_exception(std::current_exception()); // Set the exception to the promise to propagate it to the future
				}
				/////////////////////////////////////////////////////////////////////////////////////////
			}, TaskPriority::High);
	}
	for (auto& f : futures)
		f.get(); // Wait till the end
	threadPool->Set_PauseLowPriorityTasks(false); // Resume low priority tasks after this CPU-bound work is done

}