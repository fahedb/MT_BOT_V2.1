#include "ChangeData.h"
#include "SelectionNodes.h"

#pragma once

class ChangeDataBuilder
{
public:
	static ChangeNode* BuildChangeNode(JsonSelectionNode* selectionJ);
	int ExtractMonth(long long tp);
};

