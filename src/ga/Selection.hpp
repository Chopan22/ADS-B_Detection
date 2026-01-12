#pragma once
#include "Chromosome.hpp"
#include "ga_config.hpp"

#include <random>
#include <vector>

namespace ga {

class Selection {
public:
  Selection(size_t tournamentSize = 3);

  Chromosome tournamentSelect(const std::vector<Chromosome>& population,
                              const std::vector<double>& fitnesses, std::mt19937& rng);

  std::vector<Chromosome> selectPopulation(const std::vector<Chromosome>& population,
                                           const std::vector<double>& fitnesses, size_t numSelected,
                                           std::mt19937& rng);

private:
  size_t tournamentSize_;
};
} // namespace ga
