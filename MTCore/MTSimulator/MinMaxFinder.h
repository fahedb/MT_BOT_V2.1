#pragma once
#include "INDICATORSTRUCTRES.H"
#include <vector>
#include <limits>

/// <summary>
/// Author: Fahed Elbatash
/// /// Date: 2025-08-11
/// MasterPieceSoft © 2025
/// </summary>


template<typename T>
struct MinMaxFinder {

    MinMaxFinder(std::vector<T> data) : m_data(data) {}
    virtual ~MinMaxFinder() = default;
    T Find_Min() const {
        return *std::min_element(m_data.begin(), m_data.end());
    }
    T Find_Max() const {
        return *std::max_element(m_data.begin(), m_data.end());
    }


	std::vector<T> m_data;
};


struct MinMaxFinderScoresResult : public MinMaxFinder<ScoresResult> {
    MinMaxFinderScoresResult(std::vector<ScoresResult> data) : MinMaxFinder(data) {}

    virtual ScoresResult Find_Min() const;
    virtual ScoresResult Find_Max() const;
};