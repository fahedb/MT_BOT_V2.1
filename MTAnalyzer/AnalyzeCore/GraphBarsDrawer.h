#include "GraphStructres.h"
#include <string>

#pragma once
class GraphBarsDrawer
{
    double m_pipsDiff; // PipsDiff -- represent the Y Size of the Chart
    double m_Bars; // Bars -- represent The X Size of the Chart
public:
    GraphBarsDrawer(double pips, double bars) : m_pipsDiff(pips), m_Bars(bars) {} // Initialize the X and Y sizes of the chart
    void draw_chart(const GraphStructure& points, const char* filename); // Function that draw a chart
public:
    void DrawGraph(const GraphStructure& points, const std::string& pngName) { draw_chart(points, pngName.c_str()); } // Points here should represent the open and close of the bar

};

