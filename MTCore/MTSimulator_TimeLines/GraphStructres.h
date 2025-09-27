#include <vector>

#pragma once

struct GraphStructure
{
    std::vector<std::pair<double, double>> points; // X,Y points for drawing an Chart
    int splitter = 0; // A int splitter to know where the red chart starts (X point)
};
