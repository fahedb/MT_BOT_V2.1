#include "FilterTypeEnum.h"
#include "GraphTypeEnum.h"

#pragma once
class MTAnalyzeApplicationPolicy
{
public:
    MTAnalyzeApplicationPolicy()
    {
        GraphType = PatternCompare_GraphType::NONE;
        FilterType = PatternsCompare_FilterType::NONE;
    }
    PatternCompare_GraphType GraphType;
    PatternsCompare_FilterType FilterType;
};

class Settings
{
public:
    // Deleted copy constructor and assignment operator to prevent copies
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
    // Static method to get the instance of the Singleton
    static Settings* getInstance() {
        static Settings instance; // Guaranteed to be destroyed and instantiated on first use
        return &instance;
    }

public:
    MTAnalyzeApplicationPolicy* Get_Policy() { return &m_policy; }

private:
    MTAnalyzeApplicationPolicy m_policy;
    
     // Private constructor to prevent instantiation
    Settings() {
    }
    ~Settings()
    {
    }
private:
};

