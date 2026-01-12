#include "Population.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <stdexcept>

namespace ga {

Population::Population(size_t populationSize, Fitness& fitnessEvaluator, double crossoverProb,
                       double mutationProb, size_t tournamentSize)
    : populationSize_(populationSize), fitness_(fitnessEvaluator), crossoverProb_(crossoverProb),
      mutationProb_(mutationProb), tournamentSize_(tournamentSize) {
  if (populationSize_ == 0)
    throw std::runtime_error("Population size must be > 0");
}

void Population::initialize() {
  chromosomes_.resize(populationSize_);
  fitnessValues_.resize(populationSize_);

  for (auto& c : chromosomes_) {
    c = Chromosome();
    c.mutate(1.0);
  }

  evaluateFitness();
}

void Population::debugPrint() const {
#ifdef GA_TEST_MODE
  for (size_t i = 0; i < chromosomes_.size(); ++i) {
    std::cout << "Ind " << i << " | Fitness: " << fitnessValues_[i] << " | ";
    for (double g : chromosomes_[i].genes)
      std::cout << g << " ";
    std::cout << "\n";
  }
#endif
}

void Population::evaluateFitness() {
  assert(!chromosomes_.empty());
  assert(chromosomes_.size() == fitnessValues_.size());

  for (size_t i = 0; i < chromosomes_.size(); ++i) {
    fitnessValues_[i] = fitness_.evaluate(chromosomes_[i]);

    assert(fitnessValues_[i] >= 0.0);
  }
}

Chromosome Population::tournamentSelect() {
  static std::random_device             rd;
  static std::mt19937                   gen(rd());
  std::uniform_int_distribution<size_t> dis(0, populationSize_ - 1);

  size_t bestIdx = dis(gen);
  double bestFit = fitnessValues_[bestIdx];

  for (size_t i = 1; i < tournamentSize_; ++i) {
    size_t idx = dis(gen);
    if (fitnessValues_[idx] > bestFit) {
      bestIdx = idx;
      bestFit = fitnessValues_[idx];
    }
  }

  return chromosomes_[bestIdx];
}

void Population::evolve() {
  std::vector<Chromosome> newPopulation;
  newPopulation.reserve(populationSize_);

  static std::random_device              rd;
  static std::mt19937                    gen(rd());
  std::uniform_real_distribution<double> probDist(0.0, 1.0);

  while (newPopulation.size() < populationSize_) {
    // Selection
    Chromosome parent1 = tournamentSelect();
    Chromosome parent2 = tournamentSelect();

    Chromosome offspring1 = parent1;
    Chromosome offspring2 = parent2;

    // Crossover
    if (probDist(gen) < crossoverProb_) {
      Chromosome::crossover(parent1, parent2, offspring1, offspring2);
    }

    // Mutation
    if (probDist(gen) < mutationProb_)
      offspring1.mutate();
    if (probDist(gen) < mutationProb_)
      offspring2.mutate();

    newPopulation.push_back(offspring1);
    if (newPopulation.size() < populationSize_)
      newPopulation.push_back(offspring2);
  }

  chromosomes_ = std::move(newPopulation);
  evaluateFitness();
}

Chromosome Population::getBest() const {
  auto   maxIt = std::max_element(fitnessValues_.begin(), fitnessValues_.end());
  size_t idx = std::distance(fitnessValues_.begin(), maxIt);
  return chromosomes_[idx];
}
} // namespace ga
