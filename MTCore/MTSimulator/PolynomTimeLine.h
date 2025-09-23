#include "TimeLine.h"
#include "PolynomChangeNodePackage.h"
#include "MemoryUtils.h"
#include <memory>

#pragma once

class PolynomTimeLine
{
public:
    // Default constructor
    PolynomTimeLine(const std::vector<ChangeNode*>& nodes,CacheType cT)
        : m_segment(nodes.at(0)->point_DateTimeLong, nodes.at(nodes.size() - 1)->point_DateTimeLong),
          m_timeLine(std::make_unique<TimeLine>(nodes)),
          m_polynomPackage(std::make_unique<PolynomChangeNodePackage>(nodes, cT))
    {}

    // Constructor for PolynomTimeLine with ownership transfer
    PolynomTimeLine(TimeLineSegment segment, std::unique_ptr<PolynomChangeNodePackage> polynomPackage, std::unique_ptr<TimeLine> timeLine)
        : m_segment(segment),
          m_polynomPackage(std::move(polynomPackage)),
          m_timeLine(std::move(timeLine))
    {}

    // Move constructor
    PolynomTimeLine(PolynomTimeLine&& other) noexcept
        : m_segment(other.m_segment),
          m_polynomPackage(std::move(other.m_polynomPackage)),
          m_timeLine(std::move(other.m_timeLine))
    {
        // Leave other in a valid state
        other.m_segment = {0, 0};
	}

	// Copy constructor
    PolynomTimeLine(const PolynomTimeLine& other)
        : m_segment(other.m_segment),
          m_polynomPackage(other.m_polynomPackage ? std::make_unique<PolynomChangeNodePackage>(*other.m_polynomPackage) : nullptr),
          m_timeLine(other.m_timeLine ? std::make_unique<TimeLine>(*other.m_timeLine) : nullptr)
	{
	}
    
    // Move assignment
    PolynomTimeLine& operator=(PolynomTimeLine&& other) noexcept = default;

    // Delete copy constructor and copy assignment to enforce unique ownership
    
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

