#include <string>
#include "GraphStructure.h"
#include "GraphTypeEnum.h"
#include "FilterTypeEnum.h"
#include "TimeLineTypeEnum.h"

#pragma once
class GraphContainer
{
private:
	PatternsCompare_FilterType m_idType;
	PatternsCompare_TimeLineType m_TLType;

	std::string m_id;
	std::string m_LinearPngFile;
	std::string m_BarsPngFile;

	GraphStructure m_BarStructure;
	GraphStructure m_LinearStructure;

public:
	GraphContainer()
	{
		m_TLType = PatternsCompare_TimeLineType::NONE;
		m_idType = PatternsCompare_FilterType::NONE;
		m_id = "";
		m_LinearPngFile = "";
		m_BarsPngFile = "";
	}

	GraphContainer(PatternsCompare_FilterType fType, std::string id, PatternsCompare_TimeLineType tlType)
	{
		m_id = id;
		m_idType = fType;
		m_TLType = tlType;
	}

	std::string Get_GraphPngPath(PatternCompare_GraphType type);
	void Set_GraphPngPath(const std::string& path, PatternCompare_GraphType type);
	
	GraphStructure Get_GraphStructure(PatternCompare_GraphType type);
	void Set_GraphStructure(const GraphStructure& graphStruct, PatternCompare_GraphType type);

	std::string Get_Id() { return m_id; }
	PatternsCompare_FilterType Get_FilterType() { return m_idType; }
	PatternsCompare_TimeLineType Get_TimeLineType() { return m_TLType; }
};

