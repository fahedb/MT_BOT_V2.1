#include <vector>

#pragma once
struct GraphStructure
{
    std::vector<std::pair<double, double>> points; // X,Y points for drawing an Chart , In bars case, first element is the open and second element is the close point and the index is the X
    int splitter = 0; // A int splitter to know where the red chart starts (X point)
};
