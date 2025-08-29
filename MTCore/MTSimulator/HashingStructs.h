#pragma once
#include <string>
#include "robin_hood.h"

// Hash function for std::pair<ssize_t,ssize_t>
namespace std {
    template<>
    struct hash<std::pair<ssize_t, ssize_t>> {
        size_t operator()(const std::pair<ssize_t, ssize_t>& p) const noexcept {
            uint64_t h1 = static_cast<uint64_t>(p.first);
            uint64_t h2 = static_cast<uint64_t>(p.second);

            h1 += 0x9e3779b97f4a7c15;
            h1 = (h1 ^ (h1 >> 30)) * 0xbf58476d1ce4e5b9;
            h1 = (h1 ^ (h1 >> 27)) * 0x94d049bb133111eb;
            h1 = h1 ^ (h1 >> 31);

            h2 += 0x9e3779b97f4a7c15;
            h2 = (h2 ^ (h2 >> 30)) * 0xbf58476d1ce4e5b9;
            h2 = (h2 ^ (h2 >> 27)) * 0x94d049bb133111eb;
            h2 = h2 ^ (h2 >> 31);

            return h1 ^ (h2 << 1);
        }
    };
}

// Hash functions for PolynomTimeLineKey
struct PolynomTimeLineKey
{
    // Default constructor
    PolynomTimeLineKey() : startTimePoint(0), endTimePoint(0) {}
    // Constructor with parameters
    PolynomTimeLineKey(long long start, long long end) : startTimePoint(start), endTimePoint(end) {}

    long long startTimePoint; // Start time point of the timeline
    long long endTimePoint; // End time point of the timeline

    bool operator==(const PolynomTimeLineKey& other) const {
        return startTimePoint == other.startTimePoint &&
            endTimePoint == other.endTimePoint;
    }
};

// Hash function for PolynomTimeLineKey
namespace robin_hood {
    template<>
    struct hash<PolynomTimeLineKey> {
        size_t operator()(const PolynomTimeLineKey& key) const {
            // Simple but effective hash for timestamps
            // Uses prime numbers to avoid collisions
            return key.startTimePoint * 31 + key.endTimePoint * 17;
        }
    };

    // Hash function for pair of PolynomTimeLineKeys
    template <>
    struct hash<std::pair<PolynomTimeLineKey, PolynomTimeLineKey>> {
        size_t operator()(const std::pair<PolynomTimeLineKey, PolynomTimeLineKey>& p) const {
            // Simple but effective hash for timestamp pairs
            // Combines both keys with different prime multipliers
            size_t h1 = p.first.startTimePoint * 31 + p.first.endTimePoint * 17;
            size_t h2 = p.second.startTimePoint * 23 + p.second.endTimePoint * 19;
            return h1 * 37 + h2 * 41;
        }
    };
}
