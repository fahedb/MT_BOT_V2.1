#pragma once
#include <vector>
#include "ChangeData.h"

class MTSimulatorUtils
{
public:
	static void MergeResultsNodes(std::vector<std::pair<ChangeNode*, ScoresResult>>* dest, std::vector<std::vector<std::pair<ChangeNode*, ScoresResult>>>* src)
	{
		for (int t = 0; t < src->size(); t++) dest->insert(dest->end(), src->at(t).begin(), src->at(t).end());
	}
};

