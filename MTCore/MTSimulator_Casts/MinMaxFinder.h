#pragma once
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
    virtual T Find_Min() const {
        return *std::min_element(m_data.begin(), m_data.end());
    }
    virtual T Find_Max() const {
        return *std::max_element(m_data.begin(), m_data.end());
    }


	std::vector<T> m_data;
};

