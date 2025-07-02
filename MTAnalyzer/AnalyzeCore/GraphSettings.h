#include "GraphTypeEnum.h"

#pragma once
struct GraphPolicy
{
	PatternCompare_GraphType m_graphType = PatternCompare_GraphType::NONE;
};

class GraphSettings
{
public:
    // Deleted copy constructor and assignment operator to prevent copies
    GraphSettings(const GraphSettings&) = delete;
    GraphSettings& operator=(const GraphSettings&) = delete;
    // Static method to get the instance of the Singleton
    static GraphSettings* getInstance() {
        static GraphSettings instance; // Guaranteed to be destroyed and instantiated on first use
        return &instance;
    }

public:
    GraphPolicy* Get_Policy() { return &m_policy; }

private:
    GraphPolicy m_policy;

    // Private constructor to prevent instantiation
    GraphSettings() {
    }
    ~GraphSettings()
    {
    }
};

