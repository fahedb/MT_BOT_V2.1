#include <string>
#include "ThreadPool.h"
#include "ConfigsTableStaticProxy.h"

#pragma once

enum class PolicyType
{
    InfraConfigrationsPolicy,
    FusionEnginePolicy,
    ScoreComparerPolicy,
    PolynomComparerPolicy,
    ConfigrationsPolicy,
    OrderTracerPolicy,
    OrdersPolicy,
    TradeManagerPolicy, 
    ScoreCachingPolicy,
    PatternGodStatisticsPolicy,
    BaseComparerPolicy
};
struct PolicyTypeList
{
    std::vector<PolicyType> types = {
        PolicyType::InfraConfigrationsPolicy,
        PolicyType::FusionEnginePolicy,
        PolicyType::ScoreComparerPolicy,
        PolicyType::PolynomComparerPolicy,
        PolicyType::ConfigrationsPolicy,
        PolicyType::OrderTracerPolicy,
        PolicyType::OrdersPolicy,
        PolicyType::TradeManagerPolicy,
        PolicyType::ScoreCachingPolicy,
        PolicyType::PatternGodStatisticsPolicy,
        PolicyType::BaseComparerPolicy


    };
};

/// <summary>
/// Base class for all policies.
/// </summary>

class BasePolicy
{
public:
    virtual ~BasePolicy() = default;

	virtual std::vector<std::vector<std::string>> Get_All() = 0; // Pure virtual function to get all configurations

protected:
    std::vector<std::vector<std::string>> Get_AllConfigs(std::vector<std::pair<std::string, std::string>> allConfigs);

private:
	virtual void InitPolicy() = 0; // Pure virtual function to initialize the policy
};

// ---------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------

// <summary>
/// Class for Infra Configurations Policy.
/// Contains configurations related to the infrastructure such as HTTP server port.
/// </summary>

class InfraConfigrationsPolicy : public BasePolicy
{
public:
    InfraConfigrationsPolicy()
    {
        InitPolicy();
    }
    std::string http_server_port;
    virtual std::vector<std::vector<std::string>> Get_All() override;
private:
    void InitPolicy();
};
// ---------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------

// <summary>
/// Class for Fusion Engine Policy.
/// Contains configurations related to the fusion engine, such as the minimum number of similar results to fuse.
/// </summary>

struct FusionEnginePolicy : public BasePolicy
{
public:
    FusionEnginePolicy()
    {
        InitPolicy(); // Initialize the policy
	}
    virtual std::vector<std::vector<std::string>> Get_All() override;
    int Min_Similar_Results_To_Fusuion = 0; // Default value for maximum score possible for change nodes

private:
    void InitPolicy() override;
};
// ---------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------

// <summary>
/// Class for Score Comparer Policy.
/// Contains configurations related to score comparison, such as weights for different score types.
/// </summary>

struct ScoreComparerPolicy : public BasePolicy
{

public:
    ScoreComparerPolicy()
    {
        InitPolicy();
    }

    virtual std::vector<std::vector<std::string>> Get_All() override;

    int RSI_Wieght = 0;
    int EMA_Wieght = 0;
    int ZLSMA_Wieght = 0;
    int VOLUME_Wieght = 0;

private:
    void InitPolicy();
};

// ---------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------

// <summary>
/// Class for Polynom Comparer Policy.
/// Contains configurations related to polynomial comparison, such as weights for evaluation, RMSE, and coefficient similarity.
/// </summary>

struct PolynomComparerPolicy : public BasePolicy
{
public:
    PolynomComparerPolicy()
    {
        InitPolicy();
    }
    virtual std::vector<std::vector<std::string>> Get_All() override;

    double weight_eval = 0.5; // Weight for evaluation score
    double weight_rmse = 0.3; // Weight for RMSE score
    double weight_coeff = 0.2; // Weight for coefficient similarity score

    bool EnablePolynomScan = 0;
    int PolynomLen_ForScan = 5; // Length of the polynomial to scan

private:
    void InitPolicy();
};

// ---------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------

struct BaseComparerPolicy : public BasePolicy
{
public:
    BaseComparerPolicy()
    {
        InitPolicy();
    }
    virtual std::vector<std::vector<std::string>> Get_All() override
    {
        std::vector<std::pair<std::string, std::string>> allConfigs = { // Init all relevant configs for the policy
            std::make_pair(CONFIGS_MIN_SIMILAR_RESULTS_TO_FUSION),
            std::make_pair(CONFIG_POLYNOM_FUSION_WEIGHT),
            std::make_pair(CONFIG_SCORE_FUSION_WEIGHT)
        };
        return Get_AllConfigs(allConfigs);
    }
    int Min_Similar_Results_To_Fusion = 2; // Default value for minimum similar results to fusion

    double PolynomComparerWeight = 0.7; // Weight for polynomial fusion
    double ScoreComparerWeight = 0.3; // Weight for score fusion

private:
    void InitPolicy();
};

// ---------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------

/// <summary>
/// Defines a policy for tracing orders, inheriting from BasePolicy.
/// </summary>

struct OrderTracerPolicy : public BasePolicy
{
public:
    OrderTracerPolicy()
    {
        InitPolicy();
    }
    virtual std::vector<std::vector<std::string>> Get_All() override;

    bool EnableTradeTracing = false;
    bool EnableDynamicTimeLine_TradeTracing = false;
    int TraceCurrTimeLineLen = 0;

private:
    void InitPolicy();
};


// ---------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------

struct OrdersPolicy : public BasePolicy
{
public:
    OrdersPolicy()
    {
        InitPolicy();
    }
    virtual std::vector<std::vector<std::string>> Get_All() override;
    int SL_start = 0;
    int TP_start = 0;

    int SL_diff = 0;
    int TP_diff = 0;

	bool Dynamic_SL_TP = false; // Dynamic Stop Loss and Take Profit

private:
    void InitPolicy();
};

// ---------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------

/// <summary>
/// Score caching policy
/// </summary>

struct ScoreCachingPolicy : public BasePolicy
{
public:
    ScoreCachingPolicy()
    {
        InitPolicy();
    }

    virtual std::vector<std::vector<std::string>> Get_All() override;

    bool EnableScoreCaching = true;
    long ScoreResultsCacheSizePerThread = 100000; // Default size for each thread's cache
    int maxScorePossibleForChangeNodes = 0;
    int Max_Orders_InRow = 0;
    int Sleep_Minuts_after_Max_Orders_InRow = 0;
    int CleanEveryXScans = 10; // Default to clean cache every 10 scans

private:
    void InitPolicy();
};

struct TradeManagerPolicy : public BasePolicy
{
public:
    TradeManagerPolicy()
    {
        InitPolicy();
    }
    
    virtual std::vector<std::vector<std::string>> Get_All() override;

    double Pattern_Min_Patterns_ToTake_Action = 0.0;
    double Pattern_Max_Patterns_To_Find = 0.0;
    double Pattern_TopScore_ThresholdRatio = 0.0;
    double Pattern_Min_Percent_To_ExecuteOrder = 0.0;
    double Pattern_Diff_Percent_To_ExecuteOrder_Alternative = 0.0;
    double Pattern_Max_Noise_To_ExecuteOrder_Alternative = 0.0;
    int Pattern_Activate_Alternative_ExecuteOrder = 0;
    double Pattern_Min_PipsChange_ToCount = 0.0;
    double Min_MeanPips_ToExecute_Order = 0.0;

private:
    void InitPolicy();
};

class PatternGodStatisticsPolicy : public BasePolicy
{
public:
    PatternGodStatisticsPolicy()
    {
        InitPolicy();
    }
    virtual std::vector<std::vector<std::string>> Get_All() override;
    int Score_Future_Vision_Candles = 0;
    int SavedDirectionStaticsLen = 0;


private:
    void InitPolicy();
};

// ---------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------



// <summary>
/// Class for Configurations Policy.
/// Contains general policy configs
// </summary>

class ConfigrationsPolicy : public BasePolicy
{
public:
    ConfigrationsPolicy()
    {
        InitPolicy(); // Initialize the policy
	}
    virtual std::vector<std::vector<std::string>> Get_All() override;

    double Score_Min_TotalScore_ToAct_PerCandle = 0;
    int Pattern_CurrentTimeLineLen = 0;
    int EMA_len_for_Pattern_Export = 0;
    int TimeBetweenScans = 0;
    bool Export_Patterns_PNG_To_Telegram = false;
    bool Enable_Patterns_PNG_Export = false;
    int ThreadsCount = 0; // Default to number of hardware threads minus one

    
private:
    void InitPolicy() override;
};