#pragma once

#include <vector>
#include "Chromosome.hpp"
#include "../fuzzy/FuzzyInferenceSystem.hpp"

namespace ga {

class Fitness {
public:
    Fitness(const std::vector<std::map<std::string, double>>& inputs,
            const std::vector<double>& expectedOutputs);

    double evaluate(const Chromosome& chromo);

private:
    std::vector<std::map<std::string, double>> testInputs_;
    std::vector<double> expectedOutputs_;

    fuzzy::FuzzyInferenceSystem baseFIS_;
};
}
