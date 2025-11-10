#pragma once
#include <string>
#include <vector>

enum class PolicyType
{
    InfraConfigrationsPolicy, // = 0
    ScoreComparerPolicy, // = 1
    PolynomComparerPolicy, // = 2
	ConfigrationsPolicy, // = 3
	OrderTracerPolicy, // = 4
	OrdersPolicy, // = 5
	TradeManagerPolicy, // = 6
	ScoreCachingPolicy, // = 7
	PatternGodStatisticsPolicy, // = 8
    FilterLayerPolicy, // = 10
    ScoringLayerPolicy, // =11
    ExportDataLayerPolicy
};
class PolicyTypeUtils
{
public:
    static PolicyType GetEnumFromString(const std::string& policyTypeStr)
    {
        if (policyTypeStr == "InfraConfigrationsPolicy")
            return PolicyType::InfraConfigrationsPolicy;
        else if (policyTypeStr == "ScoreComparerPolicy")
            return PolicyType::ScoreComparerPolicy;
        else if (policyTypeStr == "PolynomComparerPolicy")
            return PolicyType::PolynomComparerPolicy;
        else if (policyTypeStr == "ConfigrationsPolicy")
            return PolicyType::ConfigrationsPolicy;
        else if (policyTypeStr == "OrderTracerPolicy")
            return PolicyType::OrderTracerPolicy;
        else if (policyTypeStr == "OrdersPolicy")
            return PolicyType::OrdersPolicy;
        else if (policyTypeStr == "TradeManagerPolicy")
            return PolicyType::TradeManagerPolicy;
        else if (policyTypeStr == "ScoreCachingPolicy")
            return PolicyType::ScoreCachingPolicy;
        else if (policyTypeStr == "PatternGodStatisticsPolicy")
            return PolicyType::PatternGodStatisticsPolicy;
        else if (policyTypeStr == "FilterLayerPolicy")
			return PolicyType::FilterLayerPolicy;
		else if (policyTypeStr == "ScoringLayerPolicy")
			return PolicyType::ScoringLayerPolicy;
        else if (policyTypeStr == "ExportDataLayerPolicy")
			return PolicyType::ExportDataLayerPolicy;
        else
            return static_cast<PolicyType>(-1); // Unknown policy type
	}

    static std::string GetStringFromEnum(PolicyType policyType)
    {
        switch (policyType)
        {
        case PolicyType::InfraConfigrationsPolicy:
            return "InfraConfigrationsPolicy";
            break;
        case PolicyType::ScoreComparerPolicy:
            return "ScoreComparerPolicy";
            break;
        case PolicyType::PolynomComparerPolicy:
            return "PolynomComparerPolicy";
            break;
        case PolicyType::ConfigrationsPolicy:
            return "ConfigrationsPolicy";
            break;
        case PolicyType::OrderTracerPolicy:
            return "OrderTracerPolicy";
            break;
        case PolicyType::OrdersPolicy:
            return "OrdersPolicy";
            break;
        case PolicyType::TradeManagerPolicy:
            return "TradeManagerPolicy";
            break;
        case PolicyType::ScoreCachingPolicy:
            return "ScoreCachingPolicy";
            break;
        case PolicyType::PatternGodStatisticsPolicy:
            return "PatternGodStatisticsPolicy";
            break;
		case PolicyType::FilterLayerPolicy:
			return "FilterLayerPolicy";
			break;
		case PolicyType::ScoringLayerPolicy:
			return "ScoringLayerPolicy";
			break;
            case PolicyType::ExportDataLayerPolicy:
            return "ExportDataLayerPolicy";
			break;
        default:
            return "UnknownPolicyType";
            break;
        }
    }
};

// Hash function for PolicyType enum - needed for robin_hood::unordered_map
namespace std {
    template<>
    struct hash<PolicyType> {
        std::size_t operator()(const PolicyType& type) const {
            return static_cast<std::size_t>(type);
        }
    };
}

struct PolicyTypeList
{
    std::vector<PolicyType> types = {
        PolicyType::InfraConfigrationsPolicy,
        PolicyType::ScoreComparerPolicy,
        PolicyType::PolynomComparerPolicy,
        PolicyType::ConfigrationsPolicy,
        PolicyType::OrderTracerPolicy,
        PolicyType::OrdersPolicy,
        PolicyType::TradeManagerPolicy,
        PolicyType::ScoreCachingPolicy,
        PolicyType::PatternGodStatisticsPolicy,
		PolicyType::FilterLayerPolicy,
        PolicyType::ScoringLayerPolicy,
		PolicyType::ExportDataLayerPolicy
    };
};
