#pragma once
#include "ga_config.hpp"

#include <vector>
#include <stdexcept>

struct GeneBounds {
    double min;
    double max;
};

struct VariableScope {
    double min;
    double max;
};

class Chromosome {
public:
    std::vector<double> genes;
    std::vector<GeneBounds> bounds;

    Chromosome();

    void updateBounds();
    voidstd::pair<Chromosome, Chromosome> crossoverTwo(const Chromosome& other, std::mt19937& rng) const; repair();

    #ifdef GA_TEST_MODE
    constexpr size_t TOTAL_GENES = 6;
    #else
    constexpr size_t TOTAL_GENES = 66;
    #endif
    static const std::vector<double> DEFAULT_GENES;
};
