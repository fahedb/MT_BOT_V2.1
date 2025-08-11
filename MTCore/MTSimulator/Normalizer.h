#include "ComparersEnums.h"
#include "IndicatorStructres.h"
#pragma once

template<typename T>
struct Normalizer { 
    Normalizer(T minV, T maxV) : minVal(minV), maxVal(maxV) {}

    virtual ~Normalizer() = default;
    virtual T normalize(const T& r) const = 0;

    T minVal;    // ערך מינימום מחושב מ-ScoresResult (למשל perItemMin)
    T maxVal;    // ערך מקסימום מחושב מ-ScoresResult (למשל perItemMax)
    T rangeVal;
};

struct ScoresResultRangeNormalizer final : Normalizer<ScoresResult> {

    explicit ScoresResultRangeNormalizer(ScoresResult minV, ScoresResult maxV) : Normalizer(minV, maxV) { rangeVal = maxVal - minVal; }

    ScoresResult normalize(const ScoresResult& r) const override {
        // Normalize by min and max
        ScoresResult tmp = r - minVal;
        return tmp / rangeVal;

    }
};