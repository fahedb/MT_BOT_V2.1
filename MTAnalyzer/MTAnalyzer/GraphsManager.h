#include "GraphTypeEnum.h"
#include "TimeLineTypeEnum.h"
#include "GraphContainerCache.h"
#include "FilterTypeEnum.h"
#include "NativeToManagedConverter.h"

using namespace System;
using namespace System::Collections::Generic;

#pragma once
ref class GraphsManager
{
public:
	static String^ Get_GraphPath(long long mainId, long long sonId, PatternsCompare_FilterType fT, PatternsCompare_TimeLineType tlT, PatternCompare_GraphType gT)
	{
		GraphContainerCache* graphCache = GraphContainerCache::getInstance();
		GraphContainer* grphContainer = graphCache->Get_Container(std::to_string(mainId), std::to_string(sonId), tlT);
		if (grphContainer != nullptr)
		{
			std::string pngPath = grphContainer->Get_GraphPngPath(gT);
			if (!pngPath.empty())
			{
				return NativeToManagedConverter::ConvertString(pngPath);
			}
		}
		return NativeToManagedConverter::ConvertString("");
	}

	static std::vector<GraphPack>* Get_AllPacks(PatternsCompare_FilterType fT) { return GraphContainerCache::getInstance()->Get_GraphPacks(fT); }
	static GraphPack* Get_Pack(PatternsCompare_FilterType fT, const std::string& parentId) { return GraphContainerCache::getInstance()->Get_GraphPack(fT, parentId); }

};

