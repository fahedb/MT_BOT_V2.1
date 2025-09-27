#include "Normalizer.h"

// Author: Fahed Elbatash
// Date: 2025-08-11
// MasterPieceSoft ©

////////////////////////////////////////
///////ScoresResultRangeNormalizer//////
////////////////////////////////////////

ScoresResultRangeNormalizer::ScoresResultRangeNormalizer(const ScoresResult& minV, const ScoresResult& maxV)
    : Normalizer(minV, maxV)
{
    // Make sure to calculate the total results for min and max values
    minVal.calcTotalResult();
    maxVal.calcTotalResult();

    // Calc the range value
    rangeVal = maxVal - minVal;

    // If the range is zero, we cannot normalize, so we set hasValidRange to false
    if (rangeVal.totalScore.total == 0) {
        hasValidRange = false;
    }
}
