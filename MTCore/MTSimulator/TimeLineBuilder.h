#include "TimeLine.h"
#include "TimeLineScored.h"

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

	static std::vector<TimeLine> Build_TimeLine_Patterns(TimeLine* history,std::vector<ChangeNode*>* heads, int patternLen,int futureLen = 0)
	{
		std::vector<TimeLine> similarPatterns;
		int timeLineLen = patternLen;
		for (int i = 0; i < heads->size(); i++)
		{
			int indexStart = history->Get_DBMap()->GetIdx_Of_TimePoint(heads->at(i)->point_DateTime);

			size_t histSize = history->Get_TimeLine()->size();
			if (indexStart >= timeLineLen && static_cast<size_t>(indexStart) + static_cast<size_t>(futureLen) <= histSize) {
				auto* currTimeLine = new std::vector<ChangeNode*>(
					history->Get_TimeLine()->begin() + (static_cast<size_t>(indexStart) - static_cast<size_t>(timeLineLen)),
					history->Get_TimeLine()->begin() + (static_cast<size_t>(indexStart) + static_cast<size_t>(futureLen)));
				similarPatterns.emplace_back(currTimeLine, true);
			}

		}
		return similarPatterns;
	}

	static std::vector<TimeLineScored> Build_TimeLineScored_Patterns(TimeLine* history, std::vector<std::pair<ChangeNode*,ScoresResult>>* heads, int patternLen, int futureLen = 0)
	{
		std::vector<TimeLineScored> similarPatterns;
		similarPatterns.reserve(heads->size());

		int timeLineLen = patternLen;
		for (int i = 0; i < heads->size(); i++)
		{
			int indexStart = history->Get_DBMap()->GetIdx_Of_TimePoint(heads->at(i).first->point_DateTime);

			size_t histSize = history->Get_TimeLine()->size();
			if (indexStart >= timeLineLen && static_cast<size_t>(indexStart) + static_cast<size_t>(futureLen) <= histSize) {
				auto* currTimeLine = new std::vector<ChangeNode*>(
					history->Get_TimeLine()->begin() + (static_cast<size_t>(indexStart) - static_cast<size_t>(timeLineLen)),
					history->Get_TimeLine()->begin() + (static_cast<size_t>(indexStart) + static_cast<size_t>(futureLen)));

				TimeLine tl(currTimeLine, true);
				TimeLineScored tlScored(tl, heads->at(i).second);
				similarPatterns.push_back(tlScored);
			}

		}
		return similarPatterns;
	}


	static std::vector<ChangeNode*> SplitChangeNodeFromScores(std::vector<std::pair<ChangeNode*, ScoresResult>>* line)
	{
		std::vector<ChangeNode*> resNodes;
		resNodes.reserve(line->size());
		for (int i = 0; i < line->size(); i++)
		{
			resNodes.push_back(line->at(i).first);
		}
		return resNodes;
	}
};

