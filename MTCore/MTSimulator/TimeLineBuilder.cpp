#include "TimeLineBuilder.h"

TimeLine TimeLineBuilder::Build_TimeLine_Pattern(TimeLine* history, ChangeNode* head, int patternLen, int futureLen)
{
	int timeLineLen = patternLen;
	int indexStart = history->Get_DBMap()->GetIdx_Of_TimePoint(head->point_DateTime);
	std::vector<ChangeNode*>* vec = new std::vector<ChangeNode*>(history->Get_TimeLine()->begin() + (static_cast<size_t>(indexStart) - static_cast<size_t>(timeLineLen)), history->Get_TimeLine()->begin() + (static_cast<size_t>(indexStart) + static_cast<size_t>(futureLen)));
	TimeLine similarPattern(vec, true);
	return similarPattern;
}

std::vector<TimeLine> TimeLineBuilder::Build_TimeLine_Patterns(TimeLine* history, std::vector<ChangeNode*>* heads, int patternLen, int futureLen)
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

std::vector<TimeLineScored> TimeLineBuilder::Build_TimeLineScored_Patterns(TimeLine* history, std::vector<std::pair<ChangeNode*, ScoresResult>>* heads, int patternLen, int futureLen)
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
			similarPatterns.emplace_back(std::move(tl), heads->at(i).second);
		}

	}
	return similarPatterns;
}

std::vector<ChangeNode*> TimeLineBuilder::SplitChangeNodeFromScores(std::vector<std::pair<ChangeNode*, ScoresResult>>* line)
{
	std::vector<ChangeNode*> resNodes;
	resNodes.reserve(line->size());
	for (int i = 0; i < line->size(); i++)
	{
		resNodes.push_back(line->at(i).first);
	}
	return resNodes;
}

void TimeLineBuilder::Build_TimeLineDB(std::vector<ChangeNode*>* tl, DBMap* dbMap) {
	
	if (!tl || !dbMap)
		return; // If the time line is null, do nothing

	for (int i = 0; i < tl->size(); i++)
	{
		const std::string& timeKey = tl->at(i)->point_DateTime;
		if (!dbMap->ContainsKey(timeKey)) {
			dbMap->InsetIntoMap(timeKey, i);
		}
		else {
			Logger::getInstance().WriteLog("!!!!! Duplicate point_DateTime in DB: " + timeKey + " !!!!!!", false, true, LogType::ERR);
		}
	}
}
