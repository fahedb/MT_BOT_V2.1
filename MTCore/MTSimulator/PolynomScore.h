#include "robin_hood.h"
#include "INDICATORSTRUCTRES.H"

#pragma once

struct TimeLinePolynomScore {
    ScoresResult vScore; // Polynom scores for values
	ScoresResult sScore; // Polynom scores for slopes

    ScoresResult* GetScoreContainer(IndicatorValueType type) {
        if (type == IndicatorValueType::Value) {
            return &vScore; // Return value scores
        } else if (type == IndicatorValueType::Slope) {
            return &sScore; // Return slope scores
        }
        return nullptr; // Default to value scores if type is NONE
	}
};