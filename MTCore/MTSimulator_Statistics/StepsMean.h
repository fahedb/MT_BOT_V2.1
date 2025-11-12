#pragma once

#include <vector>

struct StepsMean
{
	std::vector<int> stepsSum; // Sum of entries for each candle index -- Every cell should contain the sum of all entries added for that candle index
	std::vector<int> stepsCount; // Count of entries for each candle index -- Every cell should contain how many entries were added for that candle index
	std::vector<double> stepsMeans; // Mean of entries for each candle index -- Every cell should contain the mean of all entries added for that candle index
	void addStep(int pipChange, int candleIdx); // Add a step entry for a specific candle index

	void SetImportedMeans(const std::vector<double>& importedMeans); // Set imported means directly
	double GetMeanForCandleIdx(int candleIdx) const; // Get mean for a specific candle index
	std::vector<double> GetAllMeansUpTo(int maxIndex = -1) const; // Get means for all indexes up to maxIndex -- defualt is to get till the end
private:
	bool m_importedMeansFlag = false; // Flag to indicate if means have been imported or calculated -- If imported , we skip calculation
};
