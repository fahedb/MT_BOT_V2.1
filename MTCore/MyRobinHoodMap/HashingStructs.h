#pragma once
#include <string>
#include <iostream>
#include <stdlib.h>

// Hash function for std::pair<ssize_t,ssize_t>
namespace std {
    template<>
    struct hash<std::pair<long long, long long>> {
        size_t operator()(const std::pair<long long, long long>& p) const noexcept {
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

    static long long Cut_SecondsAndMilliSeconds(long long timePoint) // Exmaple for TimePoint: 20250328225200000 -- YYYYMMDDHHMMSSmmm 
    {
        if(timePoint < 100000000000000) // Less than 14 digits
			return timePoint; // No seconds or milliseconds to cut
        auto tp = (timePoint / 100000LL); // Remove seconds and milliseconds
        return tp;

    }
};

#define USE_ROBUST_HASH_FUNCTIONS

#ifdef USE_ROBUST_HASH_FUNCTIONS

// //////////////////////////////////////////////////////////////////////////
// Main robust hash functions /////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////


// Hash function for PolynomTimeLineKey
namespace robin_hood {

    namespace detail_hash {
        static inline uint64_t rotl(uint64_t x, unsigned r) noexcept {
            return (x << r) | (x >> (64 - r));
        }
		// SplitMix64 is a simple and fast hash function with good avalanche properties.
        static inline uint64_t mix64(uint64_t x) noexcept {
            x ^= x >> 30; x *= 0xbf58476d1ce4e5b9ULL;
            x ^= x >> 27; x *= 0x94d049bb133111ebULL;
            x ^= x >> 31;
            return x;
        }
        static inline uint64_t combine(uint64_t a, uint64_t b) noexcept {
			// Well distributed for small changes in a or b
            uint64_t h = a ^ rotl(b, 31) ^ (a + 0x9e3779b97f4a7c15ULL) * 0x9ddfea08eb382d69ULL;
            return mix64(h);
        }
    }

    template<>
    struct hash<PolynomTimeLineKey> {
        size_t operator()(const PolynomTimeLineKey& key) const noexcept {
            using namespace detail_hash;
			// Important: work in uint64_t to get well-defined overflows
			long long start = PolynomTimeLineKey::Cut_SecondsAndMilliSeconds(key.startTimePoint);
			long long end = PolynomTimeLineKey::Cut_SecondsAndMilliSeconds(key.endTimePoint);

            uint64_t a = mix64(static_cast<uint64_t>(start));
            uint64_t b = mix64(static_cast<uint64_t>(end) ^ 0x9e3779b97f4a7c15ULL);
            uint64_t h = combine(a, b);
            return static_cast<size_t>(h);
        }
    };

    template<>
    struct hash<std::pair<PolynomTimeLineKey, PolynomTimeLineKey>> {
        size_t operator()(const std::pair<PolynomTimeLineKey, PolynomTimeLineKey>& p) const noexcept {
            using namespace detail_hash;
			// Return the hash of the pair by combining the hashes of the two PolynomTimeLineKeys
            robin_hood::hash<PolynomTimeLineKey> kh;
            uint64_t h1 = static_cast<uint64_t>(kh(p.first));
            uint64_t h2 = static_cast<uint64_t>(kh(p.second));
            uint64_t h = combine(mix64(h1), mix64(h2 ^ 0x243f6a8885a308d3ULL));
            return static_cast<size_t>(h);
        }
    };

} // namespace robin_hood


#else

//////////////////////////////////////////////////////////////////////////////////////////
// Alternative simple but effective hash functions //////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////////


// Hash function for PolynomTimeLineKey
namespace robin_hood {
    template<>
    struct hash<PolynomTimeLineKey> {
        size_t operator()(const PolynomTimeLineKey& key) const {
            // Simple but effective hash for timestamps
            // Uses prime numbers to avoid collisions
			long long start = PolynomTimeLineKey::Cut_SecondsAndMilliSeconds(key.startTimePoint);
			long long end = PolynomTimeLineKey::Cut_SecondsAndMilliSeconds(key.endTimePoint);

            return start * 31ULL + end * 17ULL;
        }
    };

    // Hash function for pair of PolynomTimeLineKeys
    template <>
    struct hash<std::pair<PolynomTimeLineKey, PolynomTimeLineKey>> {
        size_t operator()(const std::pair<PolynomTimeLineKey, PolynomTimeLineKey>& p) const {
            // Simple but effective hash for timestamp pairs
            // Combines both keys with different prime multipliers
			long long firstStart = PolynomTimeLineKey::Cut_SecondsAndMilliSeconds(p.first.startTimePoint);
			long long firstEnd = PolynomTimeLineKey::Cut_SecondsAndMilliSeconds(p.first.endTimePoint);

			long long secondStart = PolynomTimeLineKey::Cut_SecondsAndMilliSeconds(p.second.startTimePoint);
			long long secondEnd = PolynomTimeLineKey::Cut_SecondsAndMilliSeconds(p.second.endTimePoint);

            size_t h1 = firstStart * 31ULL + firstEnd * 17ULL;
            size_t h2 = secondStart * 23ULL + secondEnd * 19ULL;
            return h1 * 37ULL + h2 * 41ULL;
        }
    };
}

#endif