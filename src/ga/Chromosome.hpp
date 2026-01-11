#pragma once

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <iostream>

namespace ga {

constexpr int TOTAL_GENES = (4+9) + (4+9) + (4+9) + (4+9) + (4+3) + (4+3); 
// SpeedChange, HeadingChange, VerticalRateChange, AltitudeChange, TimeGap, AnomalyLevel

struct GeneBounds {
    double min;
    double max;
};

class Chromosome {
public:
    std::vector<double> genes;
    std::vector<GeneBounds> bounds;

    static const std::vector<double> DEFAULT_GENES;

    Chromosome();

    void repair();

    void print() const;
};
}

