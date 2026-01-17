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
    c.mutate(0.9);
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
  std::vector<Chromosome> offspringPopulation;
  offspringPopulation.reserve(populationSize_);

  static std::random_device              rd;
  static std::mt19937                    gen(rd());
  std::uniform_real_distribution<double> probDist(0.0, 1.0);

  while (offspringPopulation.size() < populationSize_) {
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

    offspringPopulation.push_back(offspring1);
    if (offspringPopulation.size() < populationSize_)
      offspringPopulation.push_back(offspring2);
  }

  std::vector<Chromosome> combinedPopulation;
  combinedPopulation.reserve(2 * populationSize_);

  combinedPopulation.insert(combinedPopulation.end(), chromosomes_.begin(), chromosomes_.end());
  combinedPopulation.insert(combinedPopulation.end(), offspringPopulation.begin(),
                            offspringPopulation.end());

  chromosomes_ = std::move(combinedPopulation);
  fitnessValues_.resize(chromosomes_.size());
  evaluateFitness();

  std::vector<size_t> indices(chromosomes_.size());
  std::iota(indices.begin(), indices.end(), 0);

  std::sort(indices.begin(), indices.end(),
            [this](size_t a, size_t b) { return fitnessValues_[a] > fitnessValues_[b]; });

  std::vector<Chromosome> elitePopulation;
  std::vector<double>     eliteFitness;
  elitePopulation.reserve(populationSize_);
  eliteFitness.reserve(populationSize_);

  for (size_t i = 0; i < populationSize_; ++i) {
    elitePopulation.push_back(chromosomes_[indices[i]]);
    eliteFitness.push_back(fitnessValues_[indices[i]]);
  }

  chromosomes_ = std::move(elitePopulation);
  fitnessValues_ = std::move(eliteFitness);
}

Chromosome Population::getBest() const {
  auto   maxIt = std::max_element(fitnessValues_.begin(), fitnessValues_.end());
  size_t idx = std::distance(fitnessValues_.begin(), maxIt);
  return chromosomes_[idx];
}
} // namespace ga
