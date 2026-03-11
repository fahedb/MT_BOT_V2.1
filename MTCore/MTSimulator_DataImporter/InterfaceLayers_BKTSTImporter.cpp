#include "InterfaceLayers_BKTSTImporter.h"
#include "LOGGER.H"

#include "ProccessFlags.h"

void InterfaceLayers_BKTSTImporter::Set_HisyoryTL(std::vector<ChangeNode*>* historyTl, DBMap* histMap) {
	m_historyTL = historyTl;
	m_historyDBMap = histMap;
}

void InterfaceLayers_BKTSTImporter::InitImporters()
{
	ResetImportersToNull();
	LayerType layerType = m_initializedLayerType;
	std::string bktstPath = m_bktstName;
	if(bktstPath.empty() || layerType == LayerType::NONE)
	{
		Logger::getInstance().WriteLog("InterfaceLayers_BKTSTImporter::InitImporters - BKTST path is empty or Layer type is NONE !!!.", false, false, LogType::LOG);
		return;
	}

	if (layerType == LayerType::NONE)
	{
		Logger::getInstance().WriteLog("InterfaceLayers_BKTSTImporter::InitImporters - Layer type is NONE !!!.", false, false, LogType::LOG);
		return;
	}
	else
		{
		switch (layerType)
		{
		case LayerType::ScoringLayer:
			m_scoringLayerImporter = new ScoringLayerImporter(bktstPath, m_historyTL, m_historyDBMap);
			m_scoringLayerImporter->InitMyImporter();
			m_scoringLayerImporter->Init_bktstPolicies();
			break;
		case LayerType::StatisticsLayer:
			m_statisticsLayerImporter = new StatisticsLayerDataImporter(bktstPath, m_historyTL, m_historyDBMap);
			m_statisticsLayerImporter->InitMyImporter();
			m_statisticsLayerImporter->Init_bktstPolicies();
			break;
		case LayerType::TradeManagementLayer:
			m_tradeLayerImporter = new TradeLayerDataImporter(bktstPath, m_historyTL, m_historyDBMap);
			m_tradeLayerImporter->InitMyImporter();
			m_tradeLayerImporter->Init_bktstPolicies();
			break;
		default:
			break;
		}
	}
}
bool InterfaceLayers_BKTSTImporter::ActPreCondition(LayerType layerType) const
{
	if (m_bktstName.empty())
	{
		return false;
	}
	if (m_scoringLayerImporter == nullptr && m_statisticsLayerImporter == nullptr && m_tradeLayerImporter == nullptr)
	{
		Logger::getInstance().WriteLog("InterfaceLayers_BKTSTImporter::ActPreCondition - All layer importers are null!", true, false, LogType::ERR);
		return false;
	}
	if (!m_historyDBMap || !m_historyTL)
	{
		Logger::getInstance().WriteLog("InterfaceLayers_BKTSTImporter::ActPreCondition - HistoryTL or HistoryDBMap is null!", true, false, LogType::ERR);
		return false;
	}
	switch (layerType)
	{
	case LayerType::ScoringLayer:
		if (!m_scoringLayerImporter)
		{
			Logger::getInstance().WriteLog("InterfaceLayers_BKTSTImporter::ActPreCondition - Scoring layer importer is null!", true, false, LogType::ERR);
			return false;
		}
		break;
	case LayerType::StatisticsLayer:
		if (!m_statisticsLayerImporter)
		{
			Logger::getInstance().WriteLog("InterfaceLayers_BKTSTImporter::ActPreCondition - Statistics layer importer is null!", true, false, LogType::ERR);
			return false;
		}
		break;
	case LayerType::TradeManagementLayer:
		if (!m_tradeLayerImporter)
		{
			Logger::getInstance().WriteLog("InterfaceLayers_BKTSTImporter::ActPreCondition - Trade layer importer is null!", true, false, LogType::ERR);
			return false;
		}
		break;
	case LayerType::NONE: // None type, return false
		Logger::getInstance().WriteLog("InterfaceLayers_BKTSTImporter::ActPreCondition - Layer type is NONE!", true, false, LogType::ERR);
		return false;
		
	default:
		break;
	}
	return true;
}
InterfaceLayers_BKTSTImporter::InterfaceLayers_BKTSTImporter()
{
	std::string bktstFilePath = ProccessFlags::GetSwitch("-ImportBKTST");
	
	if(bktstFilePath.empty())
	{
		Logger::getInstance().WriteLog("InterfaceLayers_BKTSTImporter::InterfaceLayers_BKTSTImporter - No BKTST file path provided in flags.", false, false, LogType::LOG);
		return;
	}

	int layerIdx = bktstFilePath.at(bktstFilePath.length() - 1) - '1'; // Layer index is the last character of the file type (bktst1 for layer 1, bktst2 for layer 2, etc.)
	
	LayerTypeList list;
	if(list.types.size() <= layerIdx || layerIdx < 0)
	{
		Logger::getInstance().WriteLog("InterfaceLayers_BKTSTImporter::InterfaceLayers_BKTSTImporter - Invalid layer index in BKTST file path: " + bktstFilePath, true, false, LogType::ERR);
		return;
	}

	m_initializedLayerType = static_cast<LayerType>(layerIdx); // Convert to LayerType enum -- NOTE : need to subtract 1 because enum starts from 0
	m_bktstName = bktstFilePath;
	m_isInitialized = true;
}

void InterfaceLayers_BKTSTImporter::ResetImportersToNull()
{
	if(m_scoringLayerImporter)
	{
		delete m_scoringLayerImporter;
		m_scoringLayerImporter = nullptr;
	}
	if (m_statisticsLayerImporter)
	{
		delete m_statisticsLayerImporter;
		m_statisticsLayerImporter = nullptr;
	}
	if (m_tradeLayerImporter)
	{
		delete m_tradeLayerImporter;
		m_tradeLayerImporter = nullptr;
	}
}

