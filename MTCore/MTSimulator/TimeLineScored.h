#include "TimeLine.h"

#pragma once

class TimeLineScored {
private:
    std::unique_ptr<TimeLine> m_timeLine;
    ScoresResult m_score;

public:
    // Constructor - copy
    TimeLineScored(const TimeLine& timeLine, ScoresResult score)
        : m_timeLine(std::make_unique<TimeLine>(timeLine))
        , m_score(score) {
    }

    // Constructor - move
    TimeLineScored(TimeLine&& timeLine, ScoresResult score) noexcept
        : m_timeLine(std::make_unique<TimeLine>(std::move(timeLine)))
        , m_score(score) {
    }

    // Copy constructor
    TimeLineScored(const TimeLineScored& other)
        : m_timeLine(std::make_unique<TimeLine>(*other.m_timeLine))
        , m_score(other.m_score) {
    }

    // Move constructor
    TimeLineScored(TimeLineScored&& other) noexcept = default;

    // Copy assignment
    TimeLineScored& operator=(const TimeLineScored& other) {
        if (this != &other) {
            m_timeLine = std::make_unique<TimeLine>(*other.m_timeLine);
            m_score = other.m_score;
        }
        return *this;
    }

    // Move assignment
    TimeLineScored& operator=(TimeLineScored&& other) noexcept = default;

    // Accessors
    TimeLine& Get_TimeLine() const { return *m_timeLine; }
    ScoresResult Get_Score() const { return m_score; }

    void Set_Score(ScoresResult score) { m_score = score; }
};