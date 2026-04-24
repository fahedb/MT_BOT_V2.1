#pragma once
#include "IndicatorStructres.h"
#include "LOGGER.H" 

class IndicatorStorage
{
private:
    std::vector<EMA_Ind> emaPool;
    std::vector<RSI_Ind> rsiPool;
	std::vector<ZLSMA_Ind> zlsmaPool;

    int m_changeNodesCount;
	int m_indLens;
public:
    IndicatorStorage(int changeNodesCount)
    {
		int indLens = IndicatorLenList().lengths.size();

        emaPool.resize(changeNodesCount * indLens);
        rsiPool.resize(changeNodesCount * indLens);
        zlsmaPool.resize(changeNodesCount * indLens);
        m_changeNodesCount = changeNodesCount;
		m_indLens = indLens;
    }

    BaseIndicator* Get_IndicatorPointer(IndicatorType type, IndicatorLen len, int ChangeNodeIndex)
    {
        int index = ChangeNodeIndex * m_indLens + static_cast<int>(len);
        switch (type)
        {
        case IndicatorType::EMA:
            if(index < 0 || index >= emaPool.size()) {
                Logger::getInstance().WriteLog("IndicatorStorage::Get_IndicatorPointer : Index out of bounds for EMA pool.", false, true, LogType::ERR);
                return nullptr;
			}
            return &emaPool.at(index);
        case IndicatorType::RSI:
            if (index < 0 || index >= rsiPool.size()) {
                Logger::getInstance().WriteLog("IndicatorStorage::Get_IndicatorPointer : Index out of bounds for RSI pool.", false, true, LogType::ERR);
                return nullptr;
            }
            return &rsiPool.at(index);
        case IndicatorType::ZLSMA:
            if (index < 0 || index >= zlsmaPool.size()) {
                Logger::getInstance().WriteLog("IndicatorStorage::Get_IndicatorPointer : Index out of bounds for ZLSMA pool.", false, true, LogType::ERR);
                return nullptr;
			}
            return &zlsmaPool.at(index);
        default:
            return nullptr;
		}
    }
};