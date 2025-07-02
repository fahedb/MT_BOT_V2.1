#include <cairo/cairo.h>
#include <vector>
#include <utility>
#include <string>
#include "GraphStructres.h"

#pragma once
/// <MTChart>
/// MTChart class function, for drawing charts using x,y points
/// <MTChart>

class MTChart
{
    double m_pipsDiff; // PipsDiff -- represent the Y Size of the Chart
    double m_Bars; // Bars -- represent The X Size of the Chart
public:
    MTChart(double pips, double bars) : m_pipsDiff(pips), m_Bars(bars) {} // Initialize the X and Y sizes of the chart
    void draw_chart(const GraphStructure& points, const char* filename); // Function that draw a chart
public:
    void DrawGraph(const GraphStructure& points,const std::string& pngName) { draw_chart(points, pngName.c_str()); }

};

