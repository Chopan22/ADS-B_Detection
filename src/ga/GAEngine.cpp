#include "GAEngine.hpp"
#include "Selection.hpp" 
#include "Fitness.hpp"

#include <limits>
#include <iostream>
#include <random>

namespace ga {

GAEngine::GAEngine(size_t populationSize,
                   size_t generations,
                   double crossoverProb,
                   double mutationProb,
                   size_t tournamentSize)
    : populationSize_(populationSize),
      generations_(generations),
      crossoverProb_(crossoverProb),
      mutationProb_(mutationProb),
      tournamentSize_(tournamentSize),
      population_(populationSize),
      bestFitness_(-std::numeric_limits<double>::infinity())
{
}

void GAEngine::evaluatePopulation() {
    for (auto& c : population_.getChromosomes()) {
        c.fitness = fitnessFunction(c.genes);
        if (c.fitness > bestFitness_) {
            bestFitness_ = c.fitness;
            best_ = c;
        }
    }
}

void GAEngine::run() {
    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<> dist(0.0, 1.0);

    for (size_t g = 0; g < generations_; ++g) {
        evaluatePopulation();

        Population newPop(populationSize_);

        for (size_t i = 0; i < populationSize_ / 2; ++i) {
            // Tournament selection
            Chromosome parent1 = Selection::tournament(population_, tournamentSize_, gen);
            Chromosome parent2 = Selection::tournament(population_, tournamentSize_, gen);

            Chromosome offspring1 = parent1;
            Chromosome offspring2 = parent2;

            // Crossover
            if (dist(gen) < crossoverProb_) {
                Chromosome::crossover(parent1, parent2, offspring1, offspring2);
            }

            // Mutation
            if (dist(gen) < mutationProb_) offspring1.mutate(gen);
            if (dist(gen) < mutationProb_) offspring2.mutate(gen);

            newPop.addChromosome(offspring1);
            newPop.addChromosome(offspring2);
        }

        population_ = std::move(newPop);

        if (g % 10 == 0 || g == generations_ - 1) {
            std::cout << "Generation " << g
                      << " Best Fitness: " << bestFitness_ << "\n";
        }
    }
}
}
