#include "BasePricesTable.h"
#include "Logger.h"
#include "ChangeDataBuilder.h"
#include "ProccessFlags.h"
#include "Converts.h"

int BasePricesTable::InitFileMapForRead() { // Init indexes and maps, and return chunk size
	InitFileMap();
	long linesCount = GetFileLinesCount();
	int threadsNum = MTThreadPool::getInstance()->Get_ThreadsNum();
	int chunkSize = static_cast<long>(linesCount) / threadsNum;

	for (int t = 0; t < threadsNum; t++)
	{
		int startIdx = t * chunkSize; // Calculate start index with gap
		int endIdx = (t == threadsNum - 1) ? linesCount : (t + 1) * chunkSize;
		InitLineIdx(startIdx, endIdx, t);
	}
	return chunkSize;
}

void BasePricesTable::ClearAll()
{
	RemoveSelection();
}

SQLSelectionNode* BasePricesTable::GetPrevEnt()
{
	if (index == 0)
		return NULL;
	int currIdx = index - 1;
	return GetSelected()->at(currIdx);

}

FileSelectionNode* BasePricesTable::GetPrevFileEnt()
{
	if (index == 0)
		return NULL;
	int currIdx = index - 1;
	return GetSelectedFromFile()->at(currIdx);

}

SQLSelectionNode* BasePricesTable::GetNextEnt()
{
	if (GetSelected()->size() <= index)
		return NULL;
	int currIdx = index;
	index++;
	return GetSelected()->at(currIdx);

}

FileSelectionNode* BasePricesTable::GetNextFileEnt()
{
	if (GetSelectedFromFile()->size() <= index)
		return NULL;
	int currIdx = index;

	index++;
	return GetSelectedFromFile()->at(currIdx);
}

ChangeNode* BasePricesTable::GetNextChangeNode_MultiThreaded(int threadIdx)
{
	JsonSelectionNode* j = GetNextSelection_newLogic(threadIdx);
	if (j)
	{
		ChangeNode* node = ChangeDataBuilder::BuildChangeNode(j);
		delete j;
		return node;
	}
	return nullptr;
}


ChangeNode* BasePricesTable::GetNextChangeNode()
{
	JsonSelectionNode* j = GetNextJsonEntity();
	if (j)
	{
		ChangeNode* chN = ChangeDataBuilder::BuildChangeNode(j);
		delete j;
		j = nullptr;
		return chN;
	}
	return nullptr;

}