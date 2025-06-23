#include "BasePricesTable.h"
#include "Logger.h"
#include "ChangeDataBuilder.h"

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