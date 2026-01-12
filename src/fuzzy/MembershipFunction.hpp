#pragma once

#include <algorithm>
#include <string>
#include <vector>

enum class MFType { TRIANGLE, TRAPEZOID, Z_SHAPE, S_SHAPE };

struct MembershipFunction {
  std::string         label;
  MFType              type;
  std::vector<double> p;

  double evaluate(double x) const {
    switch (type) {
    case MFType::TRIANGLE: {
      double a = p[0], b = p[1], c = p[2];
      if (x <= a || x >= c)
        return 0.0;
      if (x == b)
        return 1.0;
      return (x < b) ? (x - a) / (b - a) : (c - x) / (c - b);
    }

    case MFType::TRAPEZOID: {
      double a = p[0], b = p[1], c = p[2], d = p[3];
      if (x <= a || x >= d)
        return 0.0;
      if (x >= b && x <= c)
        return 1.0;
      return (x < b) ? (x - a) / (b - a) : (d - x) / (d - c);
    }

    case MFType::Z_SHAPE: {
      double a = p[0], b = p[1];
      if (x <= a)
        return 1.0;
      if (x >= b)
        return 0.0;
      double t = (x - a) / (b - a);
      return 1.0 - 2.0 * t * t;
    }

    case MFType::S_SHAPE: {
      double a = p[0], b = p[1];
      if (x <= a)
        return 0.0;
      if (x >= b)
        return 1.0;
      double t = (x - a) / (b - a);
      return 2.0 * t * t;
    }
    }
    return 0.0;
  }
};
