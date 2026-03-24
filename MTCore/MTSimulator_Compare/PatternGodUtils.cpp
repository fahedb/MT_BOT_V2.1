#include "PatternGodUtils.h"
// Include for unordered_set
#include <unordered_set>

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

void PatternGodUtils::FilterDuplicateIndicesByScoreWindow(
	const float* scores,
	std::vector<int>& indices,
	int window)
{
	if (scores == nullptr || indices.empty())
		return;

	// Sort candidate indices by score descending
	std::sort(indices.begin(), indices.end(),
		[scores](int a, int b)
		{
			return scores[a] > scores[b];
		});

	const int maxIdx = *std::max_element(indices.begin(), indices.end());

	// blocked[i] == 1 means index i is suppressed by a stronger nearby result
	std::vector<uint8_t> blocked(static_cast<size_t>(maxIdx + window + 1), 0);

	std::vector<int> filtered;
	filtered.reserve(indices.size());

	for (int idx : indices)
	{
		if (idx < 0)
			continue;

		if (idx < static_cast<int>(blocked.size()) && blocked[idx])
			continue;

		filtered.push_back(idx);

		const int from = std::max(0, idx - window);
		const int to = std::min(maxIdx, idx + window);

		for (int i = from; i <= to; ++i)
			blocked[i] = 1;
	}

	// Optional: return in chronological order
	std::sort(filtered.begin(), filtered.end());

	indices = std::move(filtered);
}


void PatternGodUtils::FilterDuplicatesTimeLines(
	std::vector<std::pair<ChangeNode*, ScoresResult>>& nodesResults2D,
	DBMap* historyDB_Map)
{
	if (nodesResults2D.empty() || historyDB_Map == nullptr)
		return;

	constexpr int kWindow = 30;

	struct Candidate
	{
		ChangeNode* node;
		ScoresResult scoreResult;
		int idx;
	};

	std::vector<Candidate> candidates;
	candidates.reserve(nodesResults2D.size());

	int maxIdx = 0;

	// Build flat candidate list with precomputed indices
	for (const auto& entry : nodesResults2D)
	{
		ChangeNode* node = entry.first;
		if (node == nullptr)
			continue;

		const int idx = historyDB_Map->GetIdx_Of_TimePoint(node->point_DateTimeLong);
		if (idx < 0)
			continue;

		candidates.push_back(Candidate{ node, entry.second, idx });

		if (idx > maxIdx)
			maxIdx = idx;
	}

	if (candidates.empty())
	{
		nodesResults2D.clear();
		return;
	}

	// Sort by score descending
	std::sort(candidates.begin(), candidates.end(),
		[](const Candidate& a, const Candidate& b)
		{
			return a.scoreResult.totalScore.total > b.scoreResult.totalScore.total;
		});

	// blocked[i] == 1 means this timeline index is already covered by a stronger result
	std::vector<uint8_t> blocked(static_cast<size_t>(maxIdx + kWindow + 1), 0);

	std::vector<std::pair<ChangeNode*, ScoresResult>> filtered;
	filtered.reserve(candidates.size());

	for (const Candidate& candidate : candidates)
	{
		const int idx = candidate.idx;

		if (blocked[idx])
			continue;

		filtered.push_back({ candidate.node, candidate.scoreResult });

		const int from = std::max(0, idx - kWindow);
		const int to = std::min(maxIdx, idx + kWindow);

		for (int i = from; i <= to; ++i)
			blocked[i] = 1;
	}

	// Optional: sort back by timeline order
	std::sort(filtered.begin(), filtered.end(),
		[historyDB_Map](const auto& a, const auto& b)
		{
			const int idxA = historyDB_Map->GetIdx_Of_TimePoint(a.first->point_DateTimeLong);
			const int idxB = historyDB_Map->GetIdx_Of_TimePoint(b.first->point_DateTimeLong);
			return idxA < idxB;
		});

	nodesResults2D = std::move(filtered);
}



void PatternGodUtils::filterByHighScores(std::vector<std::pair<ChangeNode*, ScoresResult>>& nodesResults2D, int numOfTopResultToFilterOut)
{

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

void PatternGodUtils::filterByThresholdScore(std::vector<std::pair<ChangeNode*, ScoresResult>>& nodesResults2D, double thresholdPercent, int minPatterns)
{
	if (nodesResults2D.empty() || nodesResults2D.size() <= minPatterns) return;

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
