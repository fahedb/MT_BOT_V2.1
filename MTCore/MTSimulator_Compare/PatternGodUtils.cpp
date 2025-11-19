#include "PatternGodUtils.h"
#include "CONFIGRATIONS.H"
// Include for unordered_set
#include <unordered_set>

TradeManagerPolicy* m_glob_tradeMngrPolicy_PatternGodUtils = nullptr;

void InitGlobPolicies()
{
	if (!m_glob_tradeMngrPolicy_PatternGodUtils)
		m_glob_tradeMngrPolicy_PatternGodUtils = Configrations::getInstance()->Get_Policy<TradeManagerPolicy>(PolicyType::TradeManagerPolicy);
}

void PatternGodUtils::SortScores(std::vector<std::pair<ChangeNode*, ScoresResult>>& nodesResults2D) // Sort the scores from the high to the low, usuallu need to do this after filtering them
{
	if (nodesResults2D.size() <= 10) {
		// If the vector has 10 or fewer elements, nothing needs to be done
		return;
	}

	// Use partial_sort to sort the vector and get top 10 scores
	std::partial_sort(nodesResults2D.begin(), nodesResults2D.begin() + (nodesResults2D.size() - 1), nodesResults2D.end(),
		[](const std::pair<ChangeNode*, ScoresResult>& a, const std::pair<ChangeNode*, ScoresResult>& b) {
			return a.second.totalScore.total > b.second.totalScore.total; // Sort by 'total' in descending order
		});

	// Implement a logic to to filter by time sessions !!

}

void PatternGodUtils::FilterDuplicatesTimeLines(std::vector<std::pair<ChangeNode*, ScoresResult>>& nodesResults2D, DBMap* historyDB_Map) // Filter out all timePoints that is in the same 30minuts
{

	if (nodesResults2D.empty()) return;

	std::vector<std::pair<ChangeNode*, ScoresResult>> filtered;
	std::unordered_set<int> usedIndices;

	for (const auto& result : nodesResults2D)
	{
		int currIdx = historyDB_Map->GetIdx_Of_TimePoint(result.first->point_DateTimeLong);
		bool isDuplicate = false;


		for (int offset = -15; offset <= 15; ++offset)
		{
			if (usedIndices.find(currIdx + offset) != usedIndices.end()) {
				isDuplicate = true;
				break;
			}
		}

		if (!isDuplicate)
		{
			filtered.push_back(result);
			usedIndices.insert(currIdx);
		}
	}

	nodesResults2D = std::move(filtered);
}

void PatternGodUtils::filterByHighScores(std::vector<std::pair<ChangeNode*, ScoresResult>>& nodesResults2D, int numOfTopResultToFilterOut)
{
	InitGlobPolicies(); // Ensure global policies are initialized

	if (nodesResults2D.size() <= numOfTopResultToFilterOut) {
		// If the vector has X or fewer elements, nothing needs to be done
		return;
	}

	// Use partial_sort to sort the vector and get top X scores
	std::partial_sort(nodesResults2D.begin(), nodesResults2D.begin() + numOfTopResultToFilterOut, nodesResults2D.end(),
		[](const std::pair<ChangeNode*, ScoresResult>& a, const std::pair<ChangeNode*, ScoresResult>& b) {
			return a.second.totalScore.total > b.second.totalScore.total; // Sort by 'total' in descending order
		});

	// Resize the vector to keep only the top X scores
	nodesResults2D.resize(numOfTopResultToFilterOut);
}

void PatternGodUtils::filterByThresholdScore(std::vector<std::pair<ChangeNode*, ScoresResult>>& nodesResults2D, double thresholdPercent)
{
	if (nodesResults2D.empty()) return;
	double maxScore = nodesResults2D.front().second.totalScore.total;
	double thresholdRatio = thresholdPercent; // score that is at least 95% of the max score

	std::vector<std::pair<ChangeNode*, ScoresResult>> topResults;

	for (const auto& result : nodesResults2D) { // Filter all scores less than X% threshold ratio from the maxScore found
		if (result.second.totalScore.total >= maxScore * thresholdRatio)
			topResults.push_back(result);
	}

	nodesResults2D = std::move(topResults);
}

std::string PatternGodUtils::BuildScanResultLog(const std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D)
{
	std::ostringstream logStream;
	logStream << "All patterns time points:\n";

	for (const auto& result : resultNodes2D) {
		const auto& time = result.first->point_DateTime;
		const auto& score = result.second;

		logStream
			<< "\nTimePoint: " << time << "\n"
			<< "    | Total Score: " << score.totalScore.total << "\n";

		IndicatorTypeList indTypes;
		for (const auto& type : indTypes.types) {
			std::string strType = Http_Utils::Get_String_IndicatorType(type);
			logStream << "    | " << strType << ": " << score.indicatorScores.at(static_cast<int>(type)).total << "\n";
		}
	}
	return logStream.str();
}
