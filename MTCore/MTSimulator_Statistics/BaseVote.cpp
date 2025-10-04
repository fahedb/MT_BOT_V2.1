#include "BaseVote.h"
#include "Normalizer.h"

std::vector<double> BaseVote::CalcVoteWeights(std::vector<std::pair<ChangeNode*, ScoresResult>>* potentioalRes)
{
	std::vector<double> totalScores(potentioalRes->size(), 0.0);
	for (size_t i = 0; i < potentioalRes->size(); i++)
	{
		totalScores[i] = static_cast<double>(potentioalRes->at(i).second.totalScore.total);
	}

	DoubleRangeNormalizer normalizer(*std::min_element(totalScores.begin(), totalScores.end()), *std::max_element(totalScores.begin(), totalScores.end()));
	std::vector<double> normalizedScores;
	normalizedScores.reserve(totalScores.size());
	for (double score : totalScores)
		normalizedScores.push_back(normalizer.normalize(score));

	return CalculateWeights(normalizedScores);
}

std::vector<double> BaseVote::CalculateWeights(const std::vector<double>& normalizedScores)
{
	std::vector<double> weights;
	weights.reserve(normalizedScores.size());
	for (double score : normalizedScores)
	{
		weights.push_back(CalculateWeight(score));
	}
	return weights;
}
