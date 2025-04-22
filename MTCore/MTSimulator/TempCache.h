#include <map>
#include <vector>
#include <string>

#pragma once
class TempCache // Singleton class for cache data -- No need to use currently -- for future usage
{
public:
    TempCache()
    {

    }
    TempCache(const TempCache&) = delete;
    TempCache& operator=(const TempCache&) = delete;

    // Static method to get the instance of the Singleton
    static TempCache* getInstance() {
        static TempCache instance; // Guaranteed to be destroyed and instantiated on first use
        return &instance;
    }

private:
    
};

