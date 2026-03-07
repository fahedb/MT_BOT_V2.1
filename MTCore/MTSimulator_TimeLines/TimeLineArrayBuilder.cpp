#include "TimeLineArrayBuilder.h"
#include "Logger.h"
#include "ThreadPool.h"
#include <future>

IndicatorTypeList glob_indType_TimeLineArrayBuilder;
IndicatorLenList glob_indLen_TimeLineArrayBuilder;
BarPeriodList glob_barP_TimeLineArrayBuilder;
IndicatorValueTypeList glob_valType_TimeLineArrayBuilder;

void TimeLineArrayBuilder::BuildMinMaxArray(const float* inData, float* outMinData, float* outMaxData, int resNum, int patternLen)
{
	for(int i =0; i< resNum; i++)
	{
		float minVal = inData[i];
		float maxVal = inData[i];
		for(int j =0; j< patternLen; j++)
		{
			float val = inData[i + j];
			if(val < minVal)
			{
				minVal = val;
			}
			if(val > maxVal)
			{
				maxVal = val;
			}
		}
		outMinData[i] = minVal;
		outMaxData[i] = maxVal;
	}
}




void TimeLineArrayBuilder::BuildTimeLineArray(float* outData, std::vector<ChangeNode*>* tl, IndicatorType indT, IndicatorLen indL, BarPeriod barP, IndicatorValueType valT)
{
	for (size_t i = 0; i < tl->size(); i++)
	{
		ChangeNode* node = tl->at(i);
		if (node)
		{
			IndicatorValues* indicatorVals = node->Get_Indicator_Pointer(indT, indL)->Get_IndicatorValues_Pointer();
			if (indicatorVals)
			{
				double value = indicatorVals->Get_Value(valT, barP);
				outData[i] = static_cast<float>(value);
			}
			else
			{
				outData[i] = -1.0f; // Default value if indicator is missing
			}
		}
		else
		{
			outData[i] = -1.0f; // Default value if node is null
		}
	}
}

void TimeLineArrayBuilder::BuildTimeLineArrays(dataStream_t* outData, std::vector<ChangeNode*>* tl)
{
	if (!tl)
	{
		Logger::getInstance().WriteLog("CudaEngine_ScoreCalcs::DataStreamSetup - Invalid data pointer", false, true, LogType::ERR);
		return;
	}

	MTThreadPool* threadPool = MTThreadPool::getInstance();
	std::vector<std::future<void>> futuresPromisses;
	threadPool->Set_PauseLowPriorityTasks(true); // Pause low priority tasks to prioritize this setup

	for (auto indT : glob_indType_TimeLineArrayBuilder.types)
	{
		IndStream_t& indStream = outData->m_indicators.at(static_cast<int>(indT));
		for (auto indL : glob_indLen_TimeLineArrayBuilder.lengths)
		{
			LenIndicatorStream_t& lenStream = indStream.m_lens.at(static_cast<int>(indL));
			for (auto barP : glob_barP_TimeLineArrayBuilder.periods)
			{
				TimeFrameStream_t& timeFrameStream = lenStream.m_timeFrames.at(static_cast<int>(barP));
				for (auto valT : glob_valType_TimeLineArrayBuilder.valueTypes)
				{
					ValueTypeStream_t& valStream = timeFrameStream.m_valueTypes.at(static_cast<int>(valT));
					float* dataArray = valStream.m_d_data;

					auto prom = std::make_shared<std::promise<void>>(); // Create a promise to signal completion
					futuresPromisses.push_back(prom->get_future()); // Store the future for later synchronization

					threadPool->enqueueTask([tl, prom, dataArray, indT, indL, barP, valT]() {
						try {
							TimeLineArrayBuilder::BuildTimeLineArray(dataArray, tl, indT, indL, barP, valT);
							prom->set_value();
						}
						catch (const std::exception& ex) { // Catch any standard exceptions
							// Handle exceptions and set the promise to avoid hanging
							std::cerr << "Exception in thread: " << ex.what() << std::endl;
							prom->set_exception(std::current_exception()); // Set the exception to the promise to propagate it to the future
						}
						}, TaskPriority::High);


				}
			}
		}
	}
	// Wait for all tasks to complete
	for (auto& f : futuresPromisses)
	{
		f.wait();
	}
	threadPool->Set_PauseLowPriorityTasks(false); // Resume low priority tasks

}

void TimeLineArrayBuilder::BuildMinMaxArrays(dataStream_t* inData, dataStream_t* outMinData, dataStream_t* outMaxData, int resNum, int patternLen)
{
	for(auto indT : glob_indType_TimeLineArrayBuilder.types)
	{
		IndStream_t& indStream = inData->m_indicators.at(static_cast<int>(indT));
		IndStream_t& outMinIndStream = outMinData->m_indicators.at(static_cast<int>(indT));
		IndStream_t& outMaxIndStream = outMaxData->m_indicators.at(static_cast<int>(indT));
		for (auto indL : glob_indLen_TimeLineArrayBuilder.lengths)
		{
			LenIndicatorStream_t& lenStream = indStream.m_lens.at(static_cast<int>(indL));
			LenIndicatorStream_t& outMinLenStream = outMinIndStream.m_lens.at(static_cast<int>(indL));
			LenIndicatorStream_t& outMaxLenStream = outMaxIndStream.m_lens.at(static_cast<int>(indL));
			for (auto barP : glob_barP_TimeLineArrayBuilder.periods)
			{
				TimeFrameStream_t& timeFrameStream = lenStream.m_timeFrames.at(static_cast<int>(barP));
				TimeFrameStream_t& outMinTimeFrameStream = outMinLenStream.m_timeFrames.at(static_cast<int>(barP));
				TimeFrameStream_t& outMaxTimeFrameStream = outMaxLenStream.m_timeFrames.at(static_cast<int>(barP));
				for (auto valT : glob_valType_TimeLineArrayBuilder.valueTypes)
				{
					ValueTypeStream_t& valStream = timeFrameStream.m_valueTypes.at(static_cast<int>(valT));
					ValueTypeStream_t& outMinValStream = outMinTimeFrameStream.m_valueTypes.at(static_cast<int>(valT));
					ValueTypeStream_t& outMaxValStream = outMaxTimeFrameStream.m_valueTypes.at(static_cast<int>(valT));
					float* dataArray = valStream.m_d_data;
					float* outMinDataArray = outMinValStream.m_d_data;
					float* outMaxDataArray = outMaxValStream.m_d_data;
					BuildMinMaxArray(dataArray, outMinDataArray, outMaxDataArray, resNum, patternLen);
				}
			}
		}
	}
}

void TimeLineArrayBuilder::BuildTotalScoreArray(dataStream_t* inData, float* outData, int resNum)
{
	MTThreadPool* threadPool = MTThreadPool::getInstance();

	int threadsNum = static_cast<int>(threadPool->Get_ThreadsNum());
	int chunkSize = (resNum + threadsNum - 1) / threadsNum; // Ceiling division to cover all results

	std::vector<std::future<void>> futuresPromisses;
	threadPool->Set_PauseLowPriorityTasks(true); // Pause low priority tasks to prioritize this setup

	for (int i = 0; i < threadsNum; i++)
	{
		auto prom = std::make_shared<std::promise<void>>(); // Create a promise to signal completion
		futuresPromisses.push_back(prom->get_future()); // Store the future for later synchronization

		threadPool->enqueueTask([inData, prom, outData, resNum, i, chunkSize]() {
			try {

				BuildTotalScoreArray_ChunkTask(inData, outData, resNum, i, chunkSize);
				prom->set_value();
			}
			catch (const std::exception& ex) { // Catch any standard exceptions
				// Handle exceptions and set the promise to avoid hanging
				std::cerr << "Exception in thread: " << ex.what() << std::endl;
				prom->set_exception(std::current_exception()); // Set the exception to the promise to propagate it to the future
			}

			}, TaskPriority::High);


	}
	// Wait for all tasks to complete
	for (auto& f : futuresPromisses)
	{
		f.wait();
	}

	threadPool->Set_PauseLowPriorityTasks(false); // Resume low priority tasks
}

void TimeLineArrayBuilder::BuildTotalScoreArray_ChunkTask(dataStream_t* inData, float* outData, int resNum, int threadIdx, int chunkSize)
{
	for (auto indT : glob_indType_TimeLineArrayBuilder.types)
	{
		IndStream_t& indStream = inData->m_indicators[static_cast<int>(indT)];
		for (auto indL : glob_indLen_TimeLineArrayBuilder.lengths)
		{
			LenIndicatorStream_t& lenStream = indStream.m_lens[static_cast<int>(indL)];
			for (auto barP : glob_barP_TimeLineArrayBuilder.periods)
			{
				TimeFrameStream_t& timeFrameStream = lenStream.m_timeFrames[static_cast<int>(barP)];
				BuildTotalScoreArray_SinglePattern(timeFrameStream, outData, resNum, threadIdx, chunkSize);
			}
		}
	}
}

void TimeLineArrayBuilder::BuildTotalScoreArray_SinglePattern(TimeFrameStream_t& timeFrameStream, float* outData, int resNum, int threadIdx, int chunkSize)
{ // Per thread chunking

	ValueTypeStream_t& valStream = timeFrameStream.m_valueTypes[static_cast<int>(IndicatorValueType::Value)];
	ValueTypeStream_t& slopeStream = timeFrameStream.m_valueTypes[static_cast<int>(IndicatorValueType::Slope)];
	float* valueDataArray = valStream.m_d_data;
	float* slopeDataArray = slopeStream.m_d_data;

	int start = threadIdx * chunkSize;
	int end = std::min(start + chunkSize, resNum);

	for(int i = start;i < end; i++)
	{
		float totalScore = 0.0f;
		totalScore += valueDataArray[i];
		totalScore += slopeDataArray[i];
		outData[i] += totalScore;
	}
}
