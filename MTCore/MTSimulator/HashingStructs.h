#include "robin_hood.h"
#include "httplib.h"
#include "PolynomTimeLineCache.h"

#pragma once
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
