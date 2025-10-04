#include "DirectionStats.h"
#include "Logger.h"

DirectionStats::DirectionStats(DirectionStats&& other) noexcept
	: m_patterns_wScores(std::move(other.m_patterns_wScores)),
	m_currPattern(other.m_currPattern ? new TimeLine(std::move(*other.m_currPattern)) : nullptr),
	m_tradeRes(other.m_tradeRes),
	m_active(other.m_active),
	m_timePoint(std::move(other.m_timePoint)),
	m_pair(other.m_pair),
	general(std::move(other.general)),
	first(std::move(other.first)),
	highest(std::move(other.highest)),
	stepsMean_General(std::move(other.stepsMean_General)), // Move constructor
	stepsMean_PurePositive(std::move(other.stepsMean_PurePositive)), // Move constructor
	stepsMean_PureNegative(std::move(other.stepsMean_PureNegative)) // Move constructor
{
	other.m_currPattern = nullptr; // Nullify the moved pointer
}

DirectionStats::DirectionStats(const DirectionStats& other)
	: m_patterns_wScores(other.m_patterns_wScores),
	m_currPattern(other.m_currPattern ? new TimeLine(*other.m_currPattern) : nullptr),
	m_tradeRes(other.m_tradeRes),
	m_active(other.m_active),
	m_timePoint(other.m_timePoint),
	m_pair(other.m_pair),
	general(other.general),
	first(other.first),
	highest(other.highest),
	stepsMean_General(other.stepsMean_General),  // Copy constructor
	stepsMean_PurePositive(other.stepsMean_PurePositive), // Copy constructor
	stepsMean_PureNegative(other.stepsMean_PureNegative)  // Copy constructor
{
}

DirectionStats::DirectionStats()
{
	m_tradeRes = false;
	m_currPattern = nullptr;
	m_pair = CoinsPair::NONE;
	m_active = false;
}

void DirectionStats::addStep_ForCandleIdx(int pipChange, int candleIdx) {
	stepsMean_General.addStep(pipChange, candleIdx);

	if (pipChange > 0) {
		stepsMean_PurePositive.addStep(pipChange, candleIdx);
	}
	else if (pipChange < 0) {
		stepsMean_PureNegative.addStep(pipChange, candleIdx);

	}
}

void DirectionStats::Add_Patterns(std::vector<TimeLineScored>* patterns,bool moveMemory)
{
	if (patterns == nullptr || patterns->empty())
	{
		Logger::getInstance().WriteLog("Add_Patterns: patterns is null or empty, nothing to add.", false, true, LogType::ERR);
		return;
	}

	for (int i = 0; i < patterns->size(); i++)
	{
		if (!moveMemory)
			m_patterns_wScores.push_back(patterns->at(i));
		else
			m_patterns_wScores.emplace_back(std::move(patterns->at(i)));
	}
}

void DirectionStats::Add_CurrPattern(const TimeLine& currPatter, bool moveMemory)
{
	if (moveMemory)
		m_currPattern = new TimeLine(std::move(currPatter));
	else
		m_currPattern = new TimeLine(currPatter);
}

VotedDirectionStats& DirectionStats::GetGroupStatsByName(const std::string& son)
{
	StatsType dir = StatusType_Utils::GetStatsTypeEnum(son);
	return GetGroupStats(dir); // Ensure general stats are initialized
}

StepsMean& DirectionStats::GetStepMeans(CommonDirection dir)
{
	if (dir == CommonDirection::UP) return stepsMean_PurePositive;
	if (dir == CommonDirection::DOWN) return stepsMean_PureNegative;
	return stepsMean_General; // Default to General if not UP or DOWN
}

VotedDirectionStats& DirectionStats::GetGroupStats(StatsType type)
{
	switch (type)
	{
	case StatsType::General:
		return general;
		break;
	case StatsType::First:
		return first;
		break;
	case StatsType::Highest:
		return highest;
		break;
	default:
		return general;
		break;
	}
}