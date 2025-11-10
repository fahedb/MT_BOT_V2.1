#pragma once

enum class LayerType
{
	ScoringLayer,
	FilterLayer,
	StatisticsLayer,
	TradeManagementLayer,
	NONE = -1
};

struct LayerTypeList {
	std::vector<LayerType> types = { LayerType::ScoringLayer, LayerType::FilterLayer, LayerType::StatisticsLayer, LayerType::TradeManagementLayer };
	
	// ScoringLayer -> .bktst1
	// FilterLayer -> .bktst2
	// StatisticsLayer -> .bktst3
	// TradeManagementLayer -> .bktst4
	static std::string GetFileExtention(LayerType layerType)
	{
		switch (layerType)
		{
		case LayerType::ScoringLayer:
			return "bktst1";
		case LayerType::FilterLayer:
			return "bktst2";
		case LayerType::StatisticsLayer:
			return "bktst3";
		case LayerType::TradeManagementLayer:
			return "bktst4";
		default:
			return "bktst";
		}
	}
};