#pragma once

#include "FuzzyRule.hpp"
#include "FuzzyVariable.hpp"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace fuzzy {

class FuzzyInferenceSystem {
public:
  void addInputVariable(const FuzzyVariable& var) { inputVars[var.name] = var; }

  void setOutputVariable(const FuzzyVariable& var) { outputVar = var; }

  void addRule(const FuzzyRule& rule) { rules.push_back(rule); }

  double evaluate(const std::map<std::string, double>& inputs) {
    std::map<double, double> aggregated;

    for (const auto& rule : rules) {
      double strength = 1.0;

      for (const auto& ant : rule.getAntecedents()) {
        const auto& var = inputVars.at(ant.variable);
        double      x = inputs.at(ant.variable);

        double mu = var.membership(ant.term, x);

        strength = std::min(strength, mu);
      }

      clipConsequent(rule.getConsequent(), strength, aggregated);
    }

    return defuzzify(aggregated);
  }

private:
  std::map<std::string, FuzzyVariable> inputVars;
  FuzzyVariable                        outputVar;
  std::vector<FuzzyRule>               rules;

  void clipConsequent(const Consequent& cons, double strength, std::map<double, double>& agg) {
    for (double x = 0.0; x <= 1.0; x += 0.01) {
      double mu = outputVar.membership(cons.term, x);
      mu = std::min(mu, strength);
      agg[x] = std::max(agg[x], mu);
    }
  }

  double defuzzify(const std::map<double, double>& agg) {
    double num = 0.0, den = 0.0;
    for (const auto& [x, mu] : agg) {
      num += x * mu;
      den += mu;
    }
    return (den > 0.0) ? num / den : 0.0;
  }
};
} // namespace fuzzy
