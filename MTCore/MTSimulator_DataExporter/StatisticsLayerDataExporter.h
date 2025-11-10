#pragma once
#include "BaseLayerDataExporter.h"
#include "DirectionStats.h"

#define VOTED_DIRECTIONS_JSON_KEY "VotedDirections"
class StatisticsLayerDataExporter : public BaseLayerDataExporter
{
	protected:
		virtual LayerType GetLayerType() const override { return LayerType::StatisticsLayer; }
public:
	StatisticsLayerDataExporter(std::string name = "") : BaseLayerDataExporter(name) {}
	void AddStatisticsData(DirectionStats* stats);
	static void BuildVotedDirectionsJson(nlohmann::json& parentJson, DirectionStats* stats, StatsType sType);
	static void BuildStepsMeanJson(nlohmann::json& parentJson, DirectionStats* stats, CommonDirection commonDir);
};

