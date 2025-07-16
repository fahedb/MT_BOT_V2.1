#include "TimeLine.h"
#include "CONFIGRATIONS.H"
#include "Polynom.h"

#pragma once

/// <summary>
/// Class to save current timeline 
/// And long current timeLine (last X bars (by a config) )
/// </summary>

class PolynomCache
{

public:

    PolynomCache()
    {
    }

    PolynomCache(const PolynomCache&) = delete;
    PolynomCache& operator=(const PolynomCache&) = delete;

    // Static method to get the instance of the Singleton
    static PolynomCache* getInstance() {
        static PolynomCache instance; // Guaranteed to be destroyed and instantiated on first use
        return &instance;
    }
private:
};

