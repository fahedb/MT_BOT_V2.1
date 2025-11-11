#include "FilterLayerDataExporter.h"
#include "LayersJsonKeys.h"

void FilterLayerDataExporter::AddFilteredPatternData(std::string currTP, std::vector<std::pair<ChangeNode*, ScoresResult>>& data)
{
	nlohmann::json j;
	for (int i = 0; i < data.size(); i++)
	{
		j[currTP][JsonKey_ScoredTimePoints].push_back({
			{JsonKey_datetime, data[i].first->point_DateTime},
			{JsonKey_score, data[i].second.totalScore.total}
			});
	}
	m_dataSummary.push_back(j);
}
