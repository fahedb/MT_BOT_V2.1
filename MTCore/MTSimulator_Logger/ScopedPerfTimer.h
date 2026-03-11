#pragma once
#include <chrono>
#include <iostream>
#include <string>
#include "LOGGER.H"

class ScopedPerfTimer
{
public:
    using Clock = std::chrono::high_resolution_clock;

    explicit ScopedPerfTimer(const std::string& name)
        : m_name(name), m_start(Clock::now())
    {
    }

    ~ScopedPerfTimer()
    {
        const auto end = Clock::now();
        auto const elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start).count();
		auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_start).count();
		if (elapsedUs > 1000) // Log only if the elapsed time is greater than 1 ms to avoid cluttering logs with very fast operations
			Logger::getInstance().WriteLog("[PERF] " + m_name + " took " + std::to_string(elapsedMs) + " ms", false, true, LogType::LOG);
        else
			Logger::getInstance().WriteLog("[PERF] " + m_name + " took " + std::to_string(elapsedUs) + " us", false, true, LogType::LOG);
    }

private:
    std::string m_name;
    Clock::time_point m_start;
};
