#pragma once
#include "Chromosome.hpp"
#include "Fitness.hpp"
#include "ga_config.hpp"

#include <memory>
#include <vector>

namespace ga {

class Population {
public:
  Population(size_t populationSize, Fitness& fitnessEvaluator, double crossoverProb = 0.8,
             double mutationProb = 0.2, size_t tournamentSize = 3);

  void initialize();
  void evolve();
  void debugPrint() const;

  Chromosome                     getBest() const;
  const std::vector<Chromosome>& getChromosomes() const { return chromosomes_; }

private:
  size_t   populationSize_;
  Fitness& fitness_;
  double   crossoverProb_;
  double   mutationProb_;
  size_t   tournamentSize_;

  std::vector<Chromosome> chromosomes_;
  std::vector<double>     fitnessValues_;

  void       evaluateFitness();
  Chromosome tournamentSelect();
};
} // namespace ga
