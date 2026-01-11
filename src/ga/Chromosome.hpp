#pragma once
#include "ga_config.hpp"

#include <vector>
#include <stdexcept>
#include <random>

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
    void repair();
    void mutate(double mutationRate = 0.1);
    
    std::pair<Chromosome, Chromosome> crossoverTwo(const Chromosome& other, std::mt19937& rng) const;
    
    static void crossover(const Chromosome& parent1, const Chromosome& parent2, Chromosome& offspring1, Chromosome& offspring2);

    static const std::vector<double> DEFAULT_GENES;
};

#ifdef GA_TEST_MODE
static constexpr size_t TOTAL_GENES = 6;
#else
static constexpr size_t TOTAL_GENES = 66;
#endif
