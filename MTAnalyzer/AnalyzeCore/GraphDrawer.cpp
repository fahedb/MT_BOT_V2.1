#include "pch.h"
#include "GraphDrawer.h"

// Include Graph Settings
#include "GraphSettings.h"

// Include Drawers 
#include "GraphLinearDrawer.h"
#include "GraphBarsDrawer.h"

void GraphDrawer::draw_chart(const GraphStructure& points, const char* filename) {
	PatternCompare_GraphType graphType = GraphSettings::getInstance()->Get_Policy()->m_graphType;
	if (graphType == PatternCompare_GraphType::Linear)
	{
		GraphLinearDrawer graph(m_pipsDiff, m_Bars);
		graph.DrawGraph(points, std::string(filename));
	}

	if (graphType == PatternCompare_GraphType::Bars)
	{
		GraphBarsDrawer graph(m_pipsDiff, m_Bars);
		graph.DrawGraph(points, std::string(filename));
	}
}