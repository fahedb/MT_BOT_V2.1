#include "ScoringLayerDataExporter.h"
#include "LayersJsonKeys.h"

void ScoringLayerDataExporter::AddScoringDataSummery(std::string currTP, std::vector<std::pair<ChangeNode*, ScoresResult>> data)
{
	nlohmann::json j;
	for (int i = 0; i < data.size(); i++)
	{
		j[currTP][JsonKey_ScoredTimePoints].push_back({
			{JsonKey_datetime, data[i].first->point_DateTime},
			{JsonKey_score, data[i].second.totalScore.total},
			/////////////////// Testing data: ////////////////////////
			{"prevIdx0_NewScore", static_cast<int>(data[i].second.totalScore.m_scores[0])},
			{"prevIdx0_OldScore", static_cast<int>(data[i].second.totalScore.m_scores[1])}
			//////////////////////////////////////////////////////////
			});
	}
	m_dataSummary.push_back(j);
	AutoSubmitAction();
}
