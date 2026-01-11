#include "GAEngine.hpp"
#include "Selection.hpp" 
#include "Fitness.hpp"

#include <limits>
#include <iostream>
#include <random>
#include <stdexcept>

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
      fitness_(nullptr),
      population_(nullptr),
      bestFitness_(-std::numeric_limits<double>::infinity())
{
}

void GAEngine::setFitnessEvaluator(Fitness* fitness) {
    fitness_ = fitness;
    if (fitness_) {
        population_ = std::make_unique<Population>(
            populationSize_, *fitness_, crossoverProb_, mutationProb_, tournamentSize_
        );
    }
}

void GAEngine::evaluatePopulation() {
    if (!population_) {
        throw std::runtime_error("Population not initialized. Call setFitnessEvaluator first.");
    }
    
    const auto& chromosomes = population_->getChromosomes();
    
    for (size_t i = 0; i < chromosomes.size(); ++i) {
        double fitness = fitness_->evaluate(chromosomes[i]);
        if (fitness > bestFitness_) {
            bestFitness_ = fitness;
            best_ = chromosomes[i];
        }
    }
}

void GAEngine::run() {
    if (!fitness_ || !population_) {
        throw std::runtime_error("Fitness evaluator not set. Call setFitnessEvaluator first.");
    }

    std::cout << "Initializing population...\n";
    population_->initialize();
    
    std::cout << "Starting GA evolution...\n";
    
    for (size_t generation = 0; generation < generations_; ++generation) {
        evaluatePopulation();

#ifdef GA_TEST_MODE
        std::cout << "\n=== Generation " << generation << " ===\n";
        population_->debugPrint();
#endif

        if (generation < generations_ - 1) {
            population_->evolve();
        }

        if (generation % 10 == 0 || generation == generations_ - 1) {
            std::cout << "Generation " << generation
                      << " | Best Fitness: " << bestFitness_ << "\n";
        }
    }
    
    std::cout << "\nGA Complete!\n";
    std::cout << "Final Best Fitness: " << bestFitness_ << "\n";
}
}
