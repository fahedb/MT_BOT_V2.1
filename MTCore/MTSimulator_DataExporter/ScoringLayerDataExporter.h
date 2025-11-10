#pragma once
#include <string>
#include "Policies.h"
#include "ChangeData.h"
#include "BaseLayerDataExporter.h"


class ScoringLayerDataExporter : public BaseLayerDataExporter
{
public:
	void AddScoringDataSummery(std::string currTP, std::vector<std::pair<ChangeNode*, ScoresResult>> data);
	ScoringLayerDataExporter(std::string name = "ScoringLayerDataExporter") : BaseLayerDataExporter(name) {}
protected:
	virtual LayerType GetLayerType() const override { return LayerType::ScoringLayer; }
};

