#include "ChangeData.h"
#include "SelectionNodes.h"

#pragma once

class ChangeDataBuilder
{
public:
	static ChangeNode* BuildChangeNode(JsonSelectionNode* selectionJ, const std::string& symbol);
	int ExtractMonth(long long tp);
};

