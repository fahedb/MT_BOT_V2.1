#include "InterfaceLayers_BKTSTExporter.h"
#include "ProccessFlags.h"


template<typename T>
T* InterfaceLayers_BKTSTExporter::GetLayerExporter(LayerType layerT)
{
    switch (layerT)
    {
    case LayerType::ScoringLayer:
        return m_scoringLayerExporter;
        break;
    case LayerType::StatisticsLayer:
		return m_statisticsLayerExporter;
        break;
    case LayerType::TradeManagementLayer:
		return m_tradeLayerExporter;
        break;
    case LayerType::NONE:
        return nullptr;
        break;
    default:
        break;
    }

}


InterfaceLayers_BKTSTExporter::InterfaceLayers_BKTSTExporter()
{
    std::string bktstName = ProccessFlags::GetSwitch("-ExportbktstName");
    if (!bktstName.empty())
    {
        InitExporters(bktstName);
        m_bktstName = bktstName;
    }
}

void InterfaceLayers_BKTSTExporter::InitExporters(std::string bktstName)
{
    m_scoringLayerExporter = new ScoringLayerDataExporter("BKTST_ScoringLayer_" + bktstName);
    m_statisticsLayerExporter = new StatisticsLayerDataExporter("BKTST_StatisticsLayer_" + bktstName);
    m_tradeLayerExporter = new TradeLayerDataExporter("BKTST_TradeLayer_" + bktstName);
}

bool InterfaceLayers_BKTSTExporter::ActPreCondition() const
{
    if (m_bktstName.empty())
        return false;
    if(m_scoringLayerExporter == nullptr || m_statisticsLayerExporter == nullptr || m_tradeLayerExporter == nullptr)
		return false;
	return true;
}

void InterfaceLayers_BKTSTExporter::ResetExporters(std::string newbktstName)
{
    delete m_scoringLayerExporter;
    m_scoringLayerExporter = nullptr;
    delete m_statisticsLayerExporter;
    m_statisticsLayerExporter = nullptr;

	InitExporters(newbktstName); // Re-initialize exporters
}



void InterfaceLayers_BKTSTExporter::SendPolicyData(BasePolicy* policy, LayerType layerT)
{
    if(!policy)
		return;

    if(!ActPreCondition())
        return;
    switch (layerT)
    {
    case LayerType::ScoringLayer:
        m_scoringLayerExporter->AddPolicyData(policy);
        break;
    case LayerType::StatisticsLayer:
        m_statisticsLayerExporter->AddPolicyData(policy);
        break;
    case LayerType::TradeManagementLayer:
        m_tradeLayerExporter->AddPolicyData(policy);
        break;
    default:
        break;
	}
}

// Send policy data to all exporters
void InterfaceLayers_BKTSTExporter::SendSummeryData_ScoringLayer(std::string currTP, std::vector<std::pair<ChangeNode*, ScoresResult>> data)
{
    if (!ActPreCondition())
        return;
    m_scoringLayerExporter->AddScoringDataSummery(currTP, data);
}


void InterfaceLayers_BKTSTExporter::SendStatisticsData_StatisticsLayer(DirectionStats* stats)
{
    if (!ActPreCondition())
        return;
    m_statisticsLayerExporter->AddStatisticsData(stats);
}

void InterfaceLayers_BKTSTExporter::SendTradeData_TradeLayer(OrderEntity* order)
{
    if (!ActPreCondition())
        return;
    m_tradeLayerExporter->AddTradeData(order);
}

void InterfaceLayers_BKTSTExporter::Sumbmit_All_Exporters_Data()
{
    if(!ActPreCondition())
		return;

    if (m_scoringLayerExporter)
        m_scoringLayerExporter->SubmitBackTestDataFile();
    if (m_statisticsLayerExporter)
        m_statisticsLayerExporter->SubmitBackTestDataFile();
    if (m_tradeLayerExporter)
        m_tradeLayerExporter->SubmitBackTestDataFile();
}