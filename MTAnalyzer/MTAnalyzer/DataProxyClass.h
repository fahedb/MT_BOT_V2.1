#include <vector>
#include "GraphContainer.h"
#include "GraphContainerCache.h"
#include "GraphTypeEnum.h"
#include "FilterTypeEnum.h"

#pragma once
ref class DataProxyClass
{
	std::pair<std::vector<GraphContainer>, std::vector<GraphContainer>> Get_Graphs()
	{
		return GraphContainerCache::getInstance()->Get_AllContainers(PatternsCompare_FilterType::ALL);
	}
};

