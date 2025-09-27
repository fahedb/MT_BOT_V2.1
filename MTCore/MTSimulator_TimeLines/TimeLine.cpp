#include "TimeLine.h"
#include "LOGGER.H"

TimeLine::TimeLine(TimeLine&& other) noexcept
{
    m_TL = other.m_TL;
    m_DB_Map = other.m_DB_Map;
    m_selfOwnership = other.m_selfOwnership;

    other.m_TL = nullptr;
    other.m_selfOwnership = false;
}

TimeLine::TimeLine(TimeLine* other) noexcept
{
    m_TL = other->m_TL;
    m_DB_Map = other->m_DB_Map;
    m_selfOwnership = other->m_selfOwnership;
    other->m_TL = nullptr;
    other->m_selfOwnership = false;
}

TimeLine::TimeLine(std::vector<ChangeNode*>* TL, bool ownership) {
    m_TL = TL;
    Init_TimeLineDB();
    m_selfOwnership = ownership;
}

TimeLine::TimeLine(std::vector<ChangeNode*> TL) {
    m_TL = new std::vector<ChangeNode*>(TL);
    Init_TimeLineDB();
    m_selfOwnership = true;
}
TimeLine::TimeLine(const TimeLine& other)
{
    m_selfOwnership = true;
    m_TL = new std::vector<ChangeNode*>(*other.m_TL);
    m_DB_Map = other.m_DB_Map;
    Logger::getInstance().WriteLog("TimeLine copy constructor called, this is not recommended, please use move semantics instead.", false, true, LogType::ERR);
}

TimeLine::~TimeLine()
{
    if (m_selfOwnership && m_TL != nullptr) {
        delete m_TL;
        m_TL = nullptr;
    }
}

void TimeLine::Init_TimeLineDB() {

    for (int i = 0; i < m_TL->size(); i++)
    {
        long long timeKey = m_TL->at(i)->point_DateTimeLong;
        if (!m_DB_Map.ContainsKey(timeKey)) {
            m_DB_Map.InsetIntoMap(timeKey, i);
        }
        else {
            Logger::getInstance().WriteLog("!!!!! Duplicate point_DateTime in DB: " + std::to_string(timeKey) + " !!!!!!", false, true, LogType::ERR);
        }
    }
}
