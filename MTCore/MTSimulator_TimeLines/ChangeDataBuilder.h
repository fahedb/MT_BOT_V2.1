#include "ChangeData.h"
#include "SelectionNodes.h"

#pragma once

class ChangeDataBuilder
{
public:
	static bool BuildChangeNode(JsonSelectionNode* selectionJ, ChangeNode* chN, const std::string& symbol);
	int ExtractMonth(long long tp);
};

