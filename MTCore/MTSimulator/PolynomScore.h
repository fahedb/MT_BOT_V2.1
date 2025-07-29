#include "robin_hood.h"
#include "INDICATORSTRUCTRES.H"

#pragma once

class PolynomScore {
private:

    robin_hood::unordered_map<IndicatorLen, double> scores;
    double totalScore = 0.0;
public:
    // defult constructor
    PolynomScore() : totalScore(0.0) {
        // Initialize scores for each length
        IndicatorLenList indLengths;
        for (const auto& len : indLengths.lengths) {
            scores[len] = 0.0; // Initialize each length's score to 0
        }
    }


    double GetScore(IndicatorLen len) const {
        auto it = scores.find(len);
        if (it != scores.end()) {
            return it->second; // Return the score for the given length
        }
        return 0.0; // Return 0 if the length is not found
    }

    void AddScore(IndicatorLen len, double score) { scores[len] += score; }// Set the score for the given length 
    double GetTotalScore() const { return totalScore; } // Return the total score

    // Calc the total score based on individual scores
    void CalculateTotalScore() {
        // Iterate through all scores and sum them up
        for (const auto& pair : scores) {
            totalScore += pair.second;
        }
    }

    // + operator to combine two PolynomScore objects
    PolynomScore operator+(const PolynomScore& other) const {
        PolynomScore result;
        for (const auto& pair : scores) {
            result.scores[pair.first] = pair.second + other.scores.at(pair.first);
        }
        result.totalScore = totalScore + other.totalScore;
        return result;
    }
    // += operator to add another PolynomScore to this one
    PolynomScore& operator+=(const PolynomScore& other) {
        for (const auto& pair : other.scores) {
            scores[pair.first] += pair.second;
        }
        totalScore += other.totalScore;
        return *this;
    }
};

struct TimeLinePolynomScore {
	PolynomScore vScore; // Polynom scores for values
	PolynomScore sScore; // Polynom scores for slopes

    PolynomScore* GetScoreContainer(IndicatorValueType type) {
        if (type == IndicatorValueType::Value) {
            return &vScore; // Return value scores
        } else if (type == IndicatorValueType::Slope) {
            return &sScore; // Return slope scores
        }
        return nullptr; // Default to value scores if type is NONE
	}
};