#include "TimeLine.h"
#include "PolynomChangeNodePackage.h"
#include "MemoryUtils.h"
#include <memory>

#pragma once

class PolynomTimeLine
{
public:
    // Default constructor
    PolynomTimeLine(const std::vector<ChangeNode*>& nodes)
        : m_segment(nodes.at(0)->point_DateTimeLong, nodes.at(nodes.size() - 1)->point_DateTimeLong),
          m_timeLine(std::make_unique<TimeLine>(nodes)),
          m_polynomPackage(std::make_unique<PolynomChangeNodePackage>(nodes))
    {}

    // Constructor for PolynomTimeLine with ownership transfer
    PolynomTimeLine(TimeLineSegment segment, std::unique_ptr<PolynomChangeNodePackage> polynomPackage, std::unique_ptr<TimeLine> timeLine)
        : m_segment(segment),
          m_polynomPackage(std::move(polynomPackage)),
          m_timeLine(std::move(timeLine))
    {}

    // Move constructor
    PolynomTimeLine(PolynomTimeLine&& other) noexcept = default;
    // Move assignment
    PolynomTimeLine& operator=(PolynomTimeLine&& other) noexcept = default;

    // Delete copy constructor and copy assignment to enforce unique ownership
    PolynomTimeLine(const PolynomTimeLine&) = delete;
    PolynomTimeLine& operator=(const PolynomTimeLine&) = delete;

    // No need for a custom destructor; unique_ptr handles cleanup

    TimeLine* Get_TimeLine() const { return m_timeLine.get(); }
    PolynomChangeNodePackage* Get_PolynomPackage() const { return m_polynomPackage.get(); }
    TimeLineSegment Get_Segment() const { return m_segment; }
private:
    TimeLineSegment m_segment;
    std::unique_ptr<PolynomChangeNodePackage> m_polynomPackage;
    std::unique_ptr<TimeLine> m_timeLine;
};

