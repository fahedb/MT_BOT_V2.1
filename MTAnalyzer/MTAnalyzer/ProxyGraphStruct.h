#include "FilterTypeEnum.h"
#include "TimeLineTypeEnum.h"
#include "GraphTypeEnum.h"
#include "GraphContainer.h"
#include "Converts.h"

#pragma once


ref class ProxyGraphStruct
{
public:
	ProxyGraphStruct(GraphContainer* container)
	{
		m_filterType = container->Get_FilterType();
		m_tlType = container->Get_TimeLineType();
		m_pointNum = Converts::ToLongLong(container->Get_Id());
	}
	ProxyGraphStruct(PatternsCompare_FilterType fType, PatternsCompare_TimeLineType tlType, long long pointNum)
	{
		m_filterType = fType;
		m_tlType = tlType;
		m_pointNum = pointNum;
	}
	long long m_pointNum;
	PatternsCompare_FilterType m_filterType;
	PatternsCompare_TimeLineType m_tlType;
};

