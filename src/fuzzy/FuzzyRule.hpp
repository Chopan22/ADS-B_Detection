#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace fuzzy {

struct Antecedent {
  std::string variable;
  std::string term;
};

struct Consequent {
  std::string variable;
  std::string term;
};

class FuzzyRule {
public:
  FuzzyRule(std::vector<Antecedent> ants, Consequent cons)
      : antecedents(std::move(ants)), consequent(std::move(cons)) {}

  const std::vector<Antecedent>& getAntecedents() const { return antecedents; }

  const Consequent& getConsequent() const { return consequent; }

private:
  std::vector<Antecedent> antecedents;
  Consequent              consequent;
};
} // namespace fuzzy
