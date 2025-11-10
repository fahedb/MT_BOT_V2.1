#pragma once
#include "ScoringLayerDataExporter.h"
#include "FilterLayerDataExporter.h"
#include "StatisticsLayerDataExporter.h"
#include "TradeLayerDataExporter.h"


class InterfaceLayers_BKTSTExporter // Singleton class
{

public:
    // Deleted copy constructor and assignment operator to prevent copies
    InterfaceLayers_BKTSTExporter(const InterfaceLayers_BKTSTExporter&) = delete;
    InterfaceLayers_BKTSTExporter& operator=(const InterfaceLayers_BKTSTExporter&) = delete;

    // Static method to get the instance of the Singleton
    static InterfaceLayers_BKTSTExporter& getInstance() {
        static InterfaceLayers_BKTSTExporter instance; // Guaranteed to be destroyed and instantiated on first use
        return instance;
    }


	template<typename T>
    T* GetLayerExporter(LayerType layerT);
	std::string Get_BKTSTName() const { return m_bktstName; }

    void SendSummeryData_ScoringLayer(std::string currTP, std::vector<std::pair<ChangeNode*, ScoresResult>> data);
    void SendFilteredData_FilterLayer(std::string currTP, std::vector<std::pair<ChangeNode*, ScoresResult>>& data);
    void SendStatisticsData_StatisticsLayer(DirectionStats* stats);
    void SendTradeData_TradeLayer(OrderEntity* order);

	void SendPolicyData(BasePolicy* policy, LayerType layerT); // Send policy data to all exporters
    void Sumbmit_All_Exporters_Data();
	void ResetExporters(std::string newbktstName); // Reset all exporters -- Need to call InitExporters after this function to use again

private:
    InterfaceLayers_BKTSTExporter();
    void InitExporters(std::string bktstName /* the name of the backtest file operation */);
	bool ActPreCondition() const; // Check if the exporters are initialized

	ScoringLayerDataExporter* m_scoringLayerExporter = nullptr;
	FilterLayerDataExporter* m_filterLayerExporter = nullptr;
	StatisticsLayerDataExporter* m_statisticsLayerExporter = nullptr;
	TradeLayerDataExporter* m_tradeLayerExporter = nullptr;
	std::string m_bktstName;
};
