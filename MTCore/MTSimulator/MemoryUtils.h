#include "SelectionNodes.h"
#include "LOGGER.H"

enum class MemoryActionType
{
    DEEP_COPY,
	MOVE,
    NONE
};

#pragma once
class MemoryUtils
{
public:
    template<typename T>
    static std::vector<std::vector<T*>>* SplitIntoChunks(std::vector<T*>& src, int chunkSize)
    {
        std::vector<std::vector<T*>>* dest = new std::vector<std::vector<T*>>;
        int totalSize = static_cast<int>(src.size());

        for (int i = 0; i < totalSize; i += chunkSize)
        {
            int end = min(i + chunkSize, totalSize);
            std::vector<T*> chunk;
            chunk.reserve(end - i);

            for (int j = i; j < end; ++j)
            {
                chunk.push_back(src[j]);
            }

            dest->push_back(std::move(chunk));
        }
        return dest;
    }

    template<typename T>
    static void DeleteObjects_ByVec(std::vector<T*>& toDelete, bool toLog =true)
    {
        if (toLog) Logger::getInstance().WriteLog("Thread Pool Worker Start Cleaning " + std::to_string(toDelete.size()) + " Objects", false, false, LogType::LOG);
        auto start = std::chrono::steady_clock::now();

        int mySize = toDelete.size();
        for (int t = 0; t < mySize; t++)
        {
            delete toDelete[t] ;
        }
        toDelete.clear();

        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        if (toLog) Logger::getInstance().WriteLog("Thread Pool Worker Done Cleaning, Execute time: " + std::to_string(elapsed_seconds.count()), false, false, LogType::LOG);
    }

    template<typename T>
    static void DeleteObjects_ByVec(std::vector<T*>* toDelete,bool toLog = true) // For vector pointer usage
    {
        if (toDelete)
            MemoryUtils::DeleteObjects_ByVec(*toDelete, toLog);
    }

};

