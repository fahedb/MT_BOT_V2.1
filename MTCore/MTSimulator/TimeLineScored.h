#include "TimeLine.h"

#pragma once
class TimeLineScored
{
public:
	TimeLineScored(TimeLine timeLine, ScoresResult score)
	{
		m_timeLine = new TimeLine(*timeLine.Get_TimeLine());
		m_score = score;
	}
	~TimeLineScored()
	{
		if (m_timeLine)
		{
			delete m_timeLine;
			m_timeLine = nullptr;
		}
	}


	TimeLineScored(const TimeLineScored& other)
		: m_timeLine(other.m_timeLine ? new TimeLine(*other.m_timeLine) : nullptr),
		m_score(other.m_score) {
	}
	TimeLineScored& operator=(const TimeLineScored&) = delete;



	TimeLine* GetTimeLine() { return m_timeLine; }
	ScoresResult GetScoreResult() { return m_score; }
private:
	TimeLine* m_timeLine;
	ScoresResult m_score;
};