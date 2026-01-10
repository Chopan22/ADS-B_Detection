#pragma once

#include <string>
#include <vector>
#include <algorithm>

enum class MFType {
    TRIANGLE,
    TRAPEZOID
};

struct MembershipFunction {
    std::string label;
    MFType type;
    std::vector<double> p;  

    double evaluate(double x) const {
        if (type == MFType::TRIANGLE) {
            double a = p[0], b = p[1], c = p[2];
            if (x <= a || x >= c) return 0.0;
            if (x == b) return 1.0;
            return (x < b) ? (x - a) / (b - a) : (c - x) / (c - b);
        } 
        else {
            double a = p[0], b = p[1], c = p[2], d = p[3];
            if (x <= a || x >= d) return 0.0;
            if (x >= b && x <= c) return 1.0;
            return (x < b) ? (x - a) / (b - a) : (d - x) / (d - c);
        }
    }
};

