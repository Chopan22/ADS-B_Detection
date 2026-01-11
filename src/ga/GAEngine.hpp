#pragma once
#include "ga_config.hpp"

#include <cstddef>
#include "Chromosome.hpp"
#include "Population.hpp"

namespace ga {

class GAEngine {
public:
    GAEngine(size_t populationSize,
             size_t generations,
             double crossoverProb,
             double mutationProb,
             size_t tournamentSize);

    void run();

    const Chromosome& bestChromosome() const { return best_; }
    double bestFitness() const { return bestFitness_; }

private:
    size_t populationSize_;
    size_t generations_;
    double crossoverProb_;
    double mutationProb_;
    size_t tournamentSize_;

    Population population_;
    Chromosome best_;
    double bestFitness_;

    void evaluatePopulation();
};
}
