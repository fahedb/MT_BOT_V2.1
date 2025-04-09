#include "TimeLine.h"
#pragma once
/// <summary>
/// Class to build time lines, there is diffrent methods that can help building time line 
/// head should be the end of the timeline, patternLen is how many candles to take backward 
/// future len, should be how many candles to look forward, 
/// for example:
/// if head is the candle of the index 500 and pattern len is 50 and futureLen is 5 
/// so the time line is: from 450 to 505
/// </summary>
class TimeLineBuilder
{
public:
	static TimeLine Build_TimeLine_Pattern(TimeLine* history, ChangeNode* head, int patternLen, int futureLen =0 )
	{
		int timeLineLen = patternLen;
		int indexStart = history->Get_DBMap()->GetIdx_Of_TimePoint(head->point_DateTime);
		std::vector<ChangeNode*>* vec = new std::vector<ChangeNode*>(history->Get_TimeLine()->begin() + (static_cast<size_t>(indexStart) - static_cast<size_t>(timeLineLen)), history->Get_TimeLine()->begin() + (static_cast<size_t>(indexStart) + static_cast<size_t>(futureLen)));
		TimeLine similarPattern(vec, true);
		return similarPattern;
	}

	static std::vector<TimeLine> Build_TimeLine_Patterns(TimeLine* history,std::vector<ChangeNode*>* head, int patternLen,int futureLen = 0)
	{
		std::vector<TimeLine> similarPatterns;
		int timeLineLen = patternLen;
		for (int i = 0; i < head->size(); i++)
		{
			int indexStart = history->Get_DBMap()->GetIdx_Of_TimePoint(head->at(i)->point_DateTime);

			std::vector<ChangeNode*>* currTimeLine = new std::vector<ChangeNode*>(history->Get_TimeLine()->begin() + (static_cast<size_t>(indexStart) - static_cast<size_t>(timeLineLen)), history->Get_TimeLine()->begin() + (static_cast<size_t>(indexStart) + static_cast<size_t>(futureLen)));
			similarPatterns.push_back(TimeLine(currTimeLine, true));
		}
		return similarPatterns;
	}
};

