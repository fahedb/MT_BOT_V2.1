#include "pch.h"
#include "GraphContainer.h"

std::string GraphContainer::Get_GraphPngPath(PatternCompare_GraphType type)
{
	switch (type)
	{
	case PatternCompare_GraphType::Linear:
		return m_LinearPngFile;
	case PatternCompare_GraphType::Bars:
		return m_BarsPngFile;
	case PatternCompare_GraphType::NONE:
		return "";
	default:
		return "";
	}
}

void GraphContainer::Set_GraphPngPath(const std::string& path, PatternCompare_GraphType type)
{
	switch (type)
	{
	case PatternCompare_GraphType::Linear:
		m_LinearPngFile = path;
		break;
	case PatternCompare_GraphType::Bars:
		m_BarsPngFile = path;
		break;
	case PatternCompare_GraphType::NONE:
		break;
	default:
		break;
	}
}

GraphStructure GraphContainer::Get_GraphStructure(PatternCompare_GraphType type) {
	switch (type)
	{
	case PatternCompare_GraphType::Linear:
		return m_LinearStructure;
	case PatternCompare_GraphType::Bars:
		return m_BarStructure;
	case PatternCompare_GraphType::NONE:
		break;
	default:
		break;
	}
}

void GraphContainer::Set_GraphStructure(const GraphStructure& graphStruct, PatternCompare_GraphType type)
{
	switch (type)
	{
	case PatternCompare_GraphType::Linear:
		m_LinearStructure = graphStruct;
		break;
	case PatternCompare_GraphType::Bars:
		m_BarStructure = graphStruct;
		break;
	case PatternCompare_GraphType::NONE:
		break;
	default:
		break;
	}
}
