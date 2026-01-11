#include "Chromosome.hpp"

namespace ga {

const std::vector<double> Chromosome::DEFAULT_GENES = {
    -6.0, -3.0,           //SpeedChange Negative_Large
    -6.0, -3.0, 0.0,      //SpeedChange Negative
    -1.0, 0.0, 1.0,       //SpeedChange Zero
    0.0, 3.0, 6.0,        //SpeedChange Positive
    3.0, 6.0,             //SpeedChange Positive_Large

    -90.0, -30.0,         //HeadingChange Negative_Large
    -60.0, -20.0, 0.0,    //HeadingChange Negative
    -5.0, 0.0, 5.0,       //HeadingChange Zero
    0.0, 20.0, 60.0,      //HeadingChange Positive
    30.0, 90.0,           //HeadingChange Positive_Large

    -10.0, -5.0,          //VerticalRataChange Negative_Large
    -10.0, -5.0, 0.0,     //VerticalRataChange Negative
    -1.0, 0.0, 1.0,       //VerticalRataChange Zero
    0.0, 5.0, 10.0,       //VerticalRataChange Positive
    5.0, 10.0,            //VerticalRataChange Positive_Large

    -500.0, -100.0,       //AltitudeChange Negative_Large
    -500.0, -100.0, 0.0,  //AltitudeChange Negative
    -50.0, 0.0, 50.0,     //AltitudeChange Zero
    0.0, 100.0, 500.0,    //AltitudeChange Positive
    100.0, 500.0,         //AltitudeChange Positive_Large

    1.0, 5.0,             //TimeGap Small
    3.0, 10.0, 20.0,      //TimeGap Medium
    15.0, 30.0, 60.0,     //TimeGap Large

    0.2, 0.4,             //AnomalyLevel Low
    0.2, 0.5, 0.8,        //AnomalyLevel Medium
    0.6, 0.8              //AnomalyLevel High
};

Chromosome::Chromosome() {
    if (DEFAULT_GENES.size() != TOTAL_GENES)
        throw std::runtime_error("DEFAULT_GENES size does not match TOTAL_GENES");

    genes = DEFAULT_GENES;
    bounds.resize(genes.size());

    // Initialize bounds relative to neighboring points
    for (size_t i = 0; i < genes.size(); ++i) {
        double left = (i == 0) ? genes[i] - 1.0 : genes[i-1];
        double right = (i == genes.size()-1) ? genes[i] + 1.0 : genes[i+1];

        bounds[i].min = std::max(genes[i] - 1.0, left);
        bounds[i].max = std::min(genes[i] + 1.0, right);
    }

    repair();
}

void Chromosome::repair() {
    // Ensure genes are within bounds
    for (size_t i = 0; i < genes.size(); ++i) {
        if (genes[i] < bounds[i].min) genes[i] = bounds[i].min;
        if (genes[i] > bounds[i].max) genes[i] = bounds[i].max;
    }

    // Ensure ordering within each MF (left < peak < right)
    // Example: each MF has 3 points
    for (size_t i = 0; i < genes.size();) {
        size_t mf_size = 3; // default for triangular
        if (i + mf_size <= genes.size()) {
            if (genes[i] > genes[i+1]) genes[i+1] = genes[i]; // left < peak
            if (genes[i+1] > genes[i+2]) genes[i+2] = genes[i+1]; // peak < right
        }
        i += mf_size;
    }
}

void Chromosome::print() const {
    std::cout << "Chromosome: ";
    for (double g : genes)
        std::cout << g << " ";
    std::cout << "\n";
}

} // namespace ga

