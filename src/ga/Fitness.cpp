#include "Fitness.hpp"

#include "../fuzzy/FuzzyInferenceSystem.hpp"

#include <cmath>
#include <stdexcept>

// Only include fuzzy headers in production mode
#ifndef GA_TEST_MODE
#include "../fuzzy/AdsbFuzzyVariable.hpp"
#include "../fuzzy/RuleBase.hpp"
#endif

namespace ga {

Fitness::Fitness(const std::vector<std::map<std::string, double>>& inputs,
                 const std::vector<double>&                        expectedOutputs)
    : testInputs_(inputs), expectedOutputs_(expectedOutputs) {
  if (inputs.size() != expectedOutputs.size())
    throw std::runtime_error("Inputs and expected outputs size mismatch");
}

#ifdef GA_TEST_MODE

double Fitness::evaluate(const Chromosome& c) {
  const size_t numTestCases = 10;
  double       totalError = 0.0;

  for (size_t i = 0; i < numTestCases; ++i) {
    double simulatedInput = static_cast<double>(i) / numTestCases;

    double fuzzyOutput = 0.0;
    double totalWeight = 0.0;

    for (size_t g = 0; g < c.genes.size(); ++g) {
      double center = c.genes[g] / 10.0;
      double distance = std::abs(simulatedInput - center);

      double membership = std::exp(-distance * distance / 0.1);

      fuzzyOutput += membership * c.genes[g];
      totalWeight += membership;
    }

    if (totalWeight > 0.0) {
      fuzzyOutput /= totalWeight;
    }

    double expectedOutput = 7.0;
    double error = fuzzyOutput - expectedOutput;
    totalError += error * error;
  }

  double mse = totalError / numTestCases;

  return 100.0 / (1.0 + mse);
}

#else

double Fitness::evaluate(const Chromosome& chromo) {
  size_t idx = 0;
  auto   nextGenes = [&](size_t count) -> std::vector<double> {
    std::vector<double> v(chromo.genes.begin() + idx, chromo.genes.begin() + idx + count);
    idx += count;
    return v;
  };

  auto speedVar = fuzzy::createSpeedChangeVariable(nextGenes(13));
  auto headingVar = fuzzy::createHeadingChangeVariable(nextGenes(13));
  auto vertVar = fuzzy::createVerticalRateChangeVariable(nextGenes(13));
  auto altVar = fuzzy::createAltitudeChangeVariable(nextGenes(13));
  auto timeVar = fuzzy::createTimeGapVariable(nextGenes(7));
  auto anomalyVar = fuzzy::createAnomalyLevelVariable(nextGenes(7));

  fuzzy::FuzzyInferenceSystem fis;
  fis.addInputVariable(speedVar);
  fis.addInputVariable(headingVar);
  fis.addInputVariable(vertVar);
  fis.addInputVariable(altVar);
  fis.addInputVariable(timeVar);
  fis.setOutputVariable(anomalyVar);

  for (const auto& rule : fuzzy::createAdsbRuleBase())
    fis.addRule(rule);

  double weightedMse = 0.0;
  double totalWeight = 0.0;

  for (size_t i = 0; i < testInputs_.size(); ++i) {
    double out = fis.evaluate(testInputs_[i]);
    double target = expectedOutputs_[i];
    double err = out - target;

    double weight = 1.0;
    if (target >= 0.8)
      weight = 10.0;
    else if (target >= 0.4)
      weight = 5.0;
    else if (target > 0.0)
      weight = 2.0;

    weightedMse += weight * (err * err);
    totalWeight += weight;
  }

  weightedMse /= totalWeight;

  return 1.0 / (1.0 + weightedMse);
}
#endif
} // namespace ga
