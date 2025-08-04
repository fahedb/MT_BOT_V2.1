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
	static TimeLine Build_TimeLine_Pattern(TimeLine* history, ChangeNode* head, int patternLen, int futureLen =0 );
	static std::vector<TimeLine> Build_TimeLine_Patterns(TimeLine* history,std::vector<ChangeNode*>* heads, int patternLen,int futureLen = 0);
	static std::vector<TimeLineScored> Build_TimeLineScored_Patterns(TimeLine* history, std::vector<std::pair<ChangeNode*,ScoresResult>>* heads, int patternLen, int futureLen = 0);
	static std::vector<ChangeNode*> SplitChangeNodeFromScores(std::vector<std::pair<ChangeNode*, ScoresResult>>* line);
	static void Build_TimeLineDB(std::vector<ChangeNode*>* tl, DBMap* dbMap);
};

