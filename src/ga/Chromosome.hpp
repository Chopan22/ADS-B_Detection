#pragma once

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

    static constexpr size_t TOTAL_GENES = 66;
    static const std::vector<double> DEFAULT_GENES;
};
