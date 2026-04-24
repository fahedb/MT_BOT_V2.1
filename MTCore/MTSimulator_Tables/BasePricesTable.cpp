#include "BasePricesTable.h"
#include "Logger.h"
#include "ChangeDataBuilder.h"
#include "ProccessFlags.h"
#include "Converts.h"
#include "ThreadPool.h"

int BasePricesTable::InitFileMapForRead() { // Init indexes and maps, and return chunk size
	InitFileMap();
	size_t linesCount = GetFileLinesCount();
	if (m_changeNodesPool)
	{
		if(!m_indicatorStorage)
			m_indicatorStorage = new IndicatorStorage(static_cast<int>(linesCount));

		m_changeNodesPool->clear();
		m_changeNodesPool->reserve(linesCount);
	}

	if (m_indicatorStorage)
	{

		for (int i = 0; i < linesCount; i++)
		{
			m_changeNodesPool->emplace_back(m_indicatorStorage, i);
		}
	}
	else
	{
		m_changeNodesPool->resize(linesCount);
	}

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
	if (m_symbol.empty())
		m_symbol = Get_Symbol();
	JsonSelectionNode jsonNode;
	bool res = GetNextSelection_newLogic(threadIdx, jsonNode);
	if (res)
	{
		ChangeNode* node = nullptr;
		int lineIdx = GetThreadLinesIdx(threadIdx);

		if (m_changeNodesPool && lineIdx < m_changeNodesPool->size())
			node = &m_changeNodesPool->at(lineIdx);
		else
			node = new ChangeNode();

		ChangeDataBuilder::BuildChangeNode(&jsonNode, node, m_symbol);
		return node;
	}
	return nullptr;
}


ChangeNode* BasePricesTable::GetNextChangeNode()
{
	if (m_symbol.empty())
		m_symbol = Get_Symbol();
	JsonSelectionNode* j = GetNextJsonEntity();
	if (j)
	{
		ChangeNode* node = nullptr;
		if (m_changeNodesPool)
			node = &m_changeNodesPool->at(GetThreadLinesIdx(0));
		else
			node = new ChangeNode();

		ChangeDataBuilder::BuildChangeNode(j, node , m_symbol);
		return node;
	}
	return nullptr;

}