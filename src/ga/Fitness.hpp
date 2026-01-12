#pragma once
#include "../fuzzy/FuzzyInferenceSystem.hpp"
#include "Chromosome.hpp"
#include "ga_config.hpp"

#include <map>
#include <vector>

namespace ga {

class Fitness {
public:
  Fitness(const std::vector<std::map<std::string, double>>& inputs,
          const std::vector<double>&                        expectedOutputs);

  double evaluate(const Chromosome& chromo);

private:
  std::vector<std::map<std::string, double>> testInputs_;
  std::vector<double>                        expectedOutputs_;
};
} // namespace ga
