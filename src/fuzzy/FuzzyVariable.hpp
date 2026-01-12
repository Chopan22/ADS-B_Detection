#pragma once

#include "MembershipFunction.hpp"

#include <stdexcept>
#include <string>
#include <vector>

struct FuzzyVariable {
  std::string                     name;
  double                          min;
  double                          max;
  std::vector<MembershipFunction> mfs;

  std::vector<double> fuzzify(double x) const {
    std::vector<double> mu;
    mu.reserve(mfs.size());
    for (const auto& mf : mfs) {
      mu.push_back(mf.evaluate(x));
    }
    return mu;
  }

  double membership(const std::string& label, double x) const {
    for (const auto& mf : mfs) {
      if (mf.label == label) {
        return mf.evaluate(x);
      }
    }
    throw std::runtime_error("Membership function not found: " + label + " in variable " + name);
  }
};
