#pragma once
#include "BaseLayerDataExporter.h"
#include "ChangeData.h"

class FilterLayerDataExporter : public BaseLayerDataExporter
{
public:
	void AddFilteredPatternData(std::string currTP, std::vector<std::pair<ChangeNode*, ScoresResult>>& data);
	FilterLayerDataExporter(std::string name = "BaseLayerDataExporter") : BaseLayerDataExporter(name) {}
protected:
	virtual LayerType GetLayerType() const override { return LayerType::FilterLayer; }
};

