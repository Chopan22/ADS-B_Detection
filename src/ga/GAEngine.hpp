#pragma once
#include "ga_config.hpp"

#include <cstddef>
#include <memory>
#include "Chromosome.hpp"
#include "Population.hpp"
#include "Fitness.hpp"

namespace ga {

class GAEngine {
public:
    GAEngine(size_t populationSize = 100,
             size_t generations = 100,
             double crossoverProb = 0.8,
             double mutationProb = 0.2,
             size_t tournamentSize = 3);

    void setFitnessEvaluator(Fitness* fitness);
    void run();

    const Chromosome& bestChromosome() const { return best_; }
    double bestFitness() const { return bestFitness_; }

private:
    size_t populationSize_;
    size_t generations_;
    double crossoverProb_;
    double mutationProb_;
    size_t tournamentSize_;

    Fitness* fitness_;
    std::unique_ptr<Population> population_;
    Chromosome best_;
    double bestFitness_;

    void evaluatePopulation();
};
}
