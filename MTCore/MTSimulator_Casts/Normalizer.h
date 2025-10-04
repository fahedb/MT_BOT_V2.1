#include <utility>

#pragma once

template<typename T>
struct Normalizer { 
    Normalizer(T minV, T maxV) : minVal(minV), maxVal(maxV) {}

    virtual ~Normalizer() = default;
    virtual T normalize(const T& r, double weight = 1) const = 0;

	T minVal;    // Minimum value calculated (e.g., perItemMin)
	T maxVal;    // Maximum value calculated (e.g., perItemMax)
    T rangeVal;
};


struct DoubleRangeNormalizer final : Normalizer<double>
{
    explicit DoubleRangeNormalizer(double minV, double maxV)
        : Normalizer(minV, maxV)
    {
        if (maxV < minV) std::swap(maxVal, minVal);
        // Calculate the range value
        rangeVal = maxVal - minVal;
        // If the range is zero, we cannot normalize, so we set hasValidRange to false
        if (rangeVal == 0) {
            hasValidRange = false;
        }
    }
    double normalize(const double& r, double weight = 1) const override
    {
        if (!hasValidRange) {
            // No valid range, return 0.0
            return 0.0;
        }
        // Sub the min value from the result and divide by the range value
        return ((r - minVal) / rangeVal) * weight;
    }
    bool hasValidRange = true;
};