#include "StepsMean.h"

void StepsMean::addStep(int pipChange, int candleIdx)
{
	if (stepsSum.size() <= candleIdx) {
		stepsSum.resize(static_cast<size_t>(candleIdx) + static_cast<size_t>(1), 0); // Resize the vector with candleIdx + 1, and init it with 0
		stepsCount.resize(static_cast<size_t>(candleIdx) + static_cast<size_t>(1), 0);
	}
	stepsSum[candleIdx] += pipChange; // Add the pipChange to the sum for that candle index
	stepsCount[candleIdx] += 1; // Increment the count for that candle index
}

double StepsMean::GetMeanForCandleIdx(int candleIdx) const {

	if (candleIdx >= stepsSum.size() || stepsCount[candleIdx] == 0)
		return 0.0;
	return static_cast<double>(stepsSum[candleIdx]) / stepsCount[candleIdx];
}
void StepsMean::SetImportedMeans(const std::vector<double>& importedMeans)
{
	m_importedMeansFlag = true; // Set the flag to indicate means were imported
	stepsMeans = importedMeans; // Directly set the means
}

std::vector<double> StepsMean::GetAllMeansUpTo(int maxIndex) const {
	if(m_importedMeansFlag)
		return stepsMeans; // If means were imported, return them directly

	std::vector<double> result;
	if (maxIndex < 0 || maxIndex >= stepsSum.size())
		maxIndex = stepsSum.size() - 1;

	result.reserve(static_cast<size_t>(maxIndex + 1));

	for (int i = 0; i <= maxIndex; ++i)
		result.push_back(GetMeanForCandleIdx(i));
	return result;
}
