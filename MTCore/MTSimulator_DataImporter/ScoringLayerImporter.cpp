#include "ScoringLayerImporter.h"
#include "nlohmann\json.hpp"
#include "..\MTSimulator_DataExporter\LayersJsonKeys.h"
#include "Converts.h"

QueueStruct<std::pair<long long, std::vector<std::pair<ChangeNode*, ScoresResult>>>> ScoringLayerImporter::Create_QueueItem(const std::string& line, long lineIndex)
{
	// Data example:
	// {"20250602010000000":{"ScoredTimePoints":[{"datetime":"20241212183500000","score":35810},{"datetime":"20241212185100000","score":51235},{"datetime":"20241212191800000","score":37025}]}}
	nlohmann::json jsonData;
	try {

		jsonData = nlohmann::json::parse(line);
	}
	catch (const std::exception& e) {
		// Handle parsing error (e.g., log the error)
		throw std::runtime_error("ScoringLayerImporter::Create_QueueItem : JSON parsing error at line " + std::to_string(lineIndex) + ": " + e.what());
	}
	std::pair<long long, std::vector<std::pair<ChangeNode*, ScoresResult>>> resultPair;

	auto it = jsonData.begin();
	resultPair.first = Converts::ToLongLong(it.key());

	// Get the ScoredTimePoints array
	auto& scoredArray = it.value().at(JsonKey_ScoredTimePoints);

	// Check if scoredArray is indeed an array
	if (!scoredArray.is_array())
		throw std::runtime_error("ScoredTimePoints is not an array at line " + std::to_string(lineIndex));

	// Iterate through each scored item
	for (const auto& scoredItem : scoredArray)
	{
		long long dt = Converts::ToLongLong(scoredItem.at(JsonKey_datetime).get<std::string>());
		int score = scoredItem.at(JsonKey_score).get<int>();

		// Get change node pointer
		ChangeNode* nodePtr = Get_ChangeNode(dt);

		ScoresResult sr;
		sr.totalScore.total = score;

		resultPair.second.push_back({ nodePtr, sr });
	}
	QueueStruct<std::pair<long long, std::vector<std::pair<ChangeNode*, ScoresResult>>>> queueItem;
	queueItem.item = resultPair;
	queueItem.lineIndex = lineIndex;
	return queueItem;
}
