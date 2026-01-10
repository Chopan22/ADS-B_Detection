#pragma once

#include "FuzzyVariable.hpp"
#include "MembershipFunction.hpp"

namespace fuzzy {

inline FuzzyVariable createSpeedChangeVariable() {
    FuzzyVariable var;
    var.name = "SpeedChange";
    var.min = -10.0;
    var.max = 10.0;

    var.mfs = {
        {"Negative_Large", MFType::Z_SHAPE, {-6.0, -3.0}},
        {"Negative",       MFType::TRIANGLE, {-6.0, -3.0, 0.0}},
        {"Zero",           MFType::TRIANGLE, {-1.0,  0.0, 1.0}},
        {"Positive",       MFType::TRIANGLE, { 0.0,  3.0, 6.0}},
        {"Positive_Large", MFType::S_SHAPE, { 3.0,  6.0}}
    };

    return var;
}

inline FuzzyVariable createHeadingChangeVariable() {
    FuzzyVariable var;
    var.name = "HeadingChange";
    var.min = -180.0;
    var.max = 180.0;

    var.mfs = {
        {"Negative_Large", MFType::Z_SHAPE, {-90.0, -30.0}},
        {"Negative",       MFType::TRIANGLE, {-60.0, -20.0, 0.0}},
        {"Zero",           MFType::TRIANGLE, { -5.0,   0.0, 5.0}},
        {"Positive",       MFType::TRIANGLE, {  0.0,  20.0, 60.0}},
        {"Positive_Large", MFType::S_SHAPE, { 30.0,  90.0}}
    };

    return var;
}

inline FuzzyVariable createVerticalRateChangeVariable() {
    FuzzyVariable var;
    var.name = "VerticalRateChange";
    var.min = -20.0;
    var.max = 20.0;

    var.mfs = {
        {"Negative_Large", MFType::Z_SHAPE, {-10.0, -5.0}},
        {"Negative",       MFType::TRIANGLE, {-10.0, -5.0, 0.0}},
        {"Zero",           MFType::TRIANGLE, { -1.0,  0.0, 1.0}},
        {"Positive",       MFType::TRIANGLE, {  0.0,  5.0, 10.0}},
        {"Positive_Large", MFType::S_SHAPE, {  5.0, 10.0}}
    };

    return var;
}

inline FuzzyVariable createAltitudeChangeVariable() {
    FuzzyVariable var;
    var.name = "AltitudeChange";
    var.min = -1000.0;
    var.max = 1000.0;

    var.mfs = {
        {"Negative_Large", MFType::Z_SHAPE, {-500.0, -100.0}},
        {"Negative",       MFType::TRIANGLE, {-500.0, -100.0, 0.0}},
        {"Zero",           MFType::TRIANGLE, { -50.0,   0.0, 50.0}},
        {"Positive",       MFType::TRIANGLE, {   0.0, 100.0, 500.0}},
        {"Positive_Large", MFType::S_SHAPE, { 100.0, 500.0}}
    };

    return var;
}

inline FuzzyVariable createTimeGapVariable() {
    FuzzyVariable var;
    var.name = "TimeGap";
    var.min = 0.0;
    var.max = 60.0;

    var.mfs = {
        {"Small",  MFType::Z_SHAPE, { 1.0,  5.0}},
        {"Medium", MFType::TRIANGLE, { 3.0, 10.0, 20.0}},
        {"Large",  MFType::S_SHAPE, {15.0, 30.0}}
    };

    return var;
}

inline FuzzyVariable createAnomalyLevelVariable() {
    FuzzyVariable var;
    var.name = "AnomalyLevel";
    var.min = 0.0;
    var.max = 1.0;

    var.mfs = {
        {"Low",    MFType::Z_SHAPE, {0.2, 0.4}},
        {"Medium", MFType::TRIANGLE, {0.2, 0.5, 0.8}},
        {"High",   MFType::S_SHAPE, {0.6, 0.8}}
    };

    return var;
}
}
