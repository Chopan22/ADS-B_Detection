#pragma once

#include <vector>
#include "FuzzyRule.hpp"

namespace fuzzy {

inline std::vector<FuzzyRule> normalBehaviorRules() {
    return {

        FuzzyRule({
            {"SpeedChange", "Zero"},
            {"HeadingChange", "Zero"},
            {"VerticalRateChange", "Zero"},
            {"TimeGap", "Small"}
        }, {"AnomalyLevel", "Low"}),

        FuzzyRule({
            {"SpeedChange", "Negative"},
            {"HeadingChange", "Zero"},
            {"TimeGap", "Small"}
        }, {"AnomalyLevel", "Low"}),

        FuzzyRule({
            {"SpeedChange", "Positive"},
            {"VerticalRateChange", "Positive"},
            {"TimeGap", "Small"}
        }, {"AnomalyLevel", "Low"})
    };
}

inline std::vector<FuzzyRule> strongAnomalyRules() {
    return {

        FuzzyRule({
            {"SpeedChange", "Positive_Large"},
            {"HeadingChange", "Positive_Large"},
            {"TimeGap", "Small"}
        }, {"AnomalyLevel", "High"}),

        FuzzyRule({
            {"HeadingChange", "Positive"},
            {"VerticalRateChange", "Negative_Large"},
            {"TimeGap", "Small"}
        }, {"AnomalyLevel", "High"}),

        FuzzyRule({
            {"AltitudeChange", "Positive_Large"},
            {"VerticalRateChange", "Zero"},
            {"TimeGap", "Small"}
        }, {"AnomalyLevel", "High"})
    };
}

inline std::vector<FuzzyRule> timeGapRules() {
    return {

        FuzzyRule({
            {"TimeGap", "Large"},
            {"SpeedChange", "Positive_Large"}
        }, {"AnomalyLevel", "Medium"}),

        FuzzyRule({
            {"TimeGap", "Large"},
            {"HeadingChange", "Positive_Large"}
        }, {"AnomalyLevel", "Low"}),

        FuzzyRule({
            {"TimeGap", "Large"},
            {"SpeedChange", "Zero"}
        }, {"AnomalyLevel", "Low"})
    };
}

inline std::vector<FuzzyRule> compoundAnomalyRules() {
    return {

        FuzzyRule({
            {"SpeedChange", "Positive"},
            {"HeadingChange", "Negative"},
            {"VerticalRateChange", "Positive_Large"},
            {"TimeGap", "Small"}
        }, {"AnomalyLevel", "High"}),

        FuzzyRule({
            {"SpeedChange", "Negative"},
            {"AltitudeChange", "Positive_Large"},
            {"TimeGap", "Small"}
        }, {"AnomalyLevel", "High"})
    };
}

inline std::vector<FuzzyRule> createAdsbRuleBase() {
    std::vector<FuzzyRule> rules;

    auto append = [&](const std::vector<FuzzyRule>& r) {
        rules.insert(rules.end(), r.begin(), r.end());
    };

    append(normalBehaviorRules());
    append(strongAnomalyRules());
    append(timeGapRules());
    append(compoundAnomalyRules());

    return rules;
}
} 
