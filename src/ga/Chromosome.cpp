#include "Chromosome.hpp"

#include <stdexcept>
#include <algorithm>

constexpr size_t SPEEDCHANGE_GENES = 13;
constexpr size_t HEADING_GENES     = 13;
constexpr size_t VERRATE_GENES     = 13;
constexpr size_t ALTITUDE_GENES    = 13;
constexpr size_t TIMEGAP_GENES     = 7;
constexpr size_t ANOMALY_GENES     = 7;

constexpr size_t TOTAL_GENES = SPEEDCHANGE_GENES + HEADING_GENES +
                               VERRATE_GENES + ALTITUDE_GENES +
                               TIMEGAP_GENES + ANOMALY_GENES;

constexpr VariableScope VARIABLE_SCOPES[] = {
    {-10.0, 10.0},     // SpeedChange
    {-180.0, 180.0},   // HeadingChange
    {-20.0, 20.0},     // VerticalRateChange
    {-1000.0, 1000.0}, // AltitudeChange
    {0.0, 60.0},       // TimeGap
    {0.0, 1.0}         // AnomalyLevel
};

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

namespace ga {
Chromosome::Chromosome() {
    if (DEFAULT_GENES.size() != TOTAL_GENES)
        throw std::runtime_error("DEFAULT_GENES size does not match TOTAL_GENES");

    genes = DEFAULT_GENES;
    bounds.resize(genes.size());
    updateBounds();
    repair();
}

void Chromosome::updateBounds() {
    auto getVarIndexAndOffset = [&](size_t i, size_t &offset) -> size_t {
        size_t start = 0;
        size_t var_idx = 0;
        size_t sizes[] = {SPEEDCHANGE_GENES, HEADING_GENES, VERRATE_GENES, ALTITUDE_GENES, TIMEGAP_GENES, ANOMALY_GENES};
        for (; var_idx < 6; ++var_idx) {
            if (i < start + sizes[var_idx]) {
                offset = i - start;
                return var_idx;
            }
            start += sizes[var_idx];
        }
        offset = 0;
        return 5;
    };

    for (size_t i = 0; i < genes.size(); ++i) {
        size_t offset;
        size_t var_idx = getVarIndexAndOffset(i, offset);

        size_t var_size;
        switch (var_idx) {
            case 0: var_size = SPEEDCHANGE_GENES; break;
            case 1: var_size = HEADING_GENES;    break;
            case 2: var_size = VERRATE_GENES;    break;
            case 3: var_size = ALTITUDE_GENES;   break;
            case 4: var_size = TIMEGAP_GENES;    break;
            case 5: var_size = ANOMALY_GENES;    break;
        }

        double var_min = VARIABLE_SCOPES[var_idx].min;
        double var_max = VARIABLE_SCOPES[var_idx].max;

        if (offset == 0) {
            bounds[i].min = var_min;
            bounds[i].max = genes[i+1];
        }
        else if (offset == 1) {
            bounds[i].min = genes[i-1];
            bounds[i].max = (i+2 < genes.size()) ? genes[i+2] : var_max;
        }
        else if (offset == var_size-1) {
            bounds[i].min = genes[i-1];
            bounds[i].max = var_max;
        }
        else if (offset == var_size-2) {
            bounds[i].min = genes[i-2];
            bounds[i].max = genes[i+1];
        }
        else { 
            bounds[i].min = genes[i-2];
            bounds[i].max = genes[i+2];
        }
    }
}

void Chromosome::repair() {
    for (size_t i = 0; i < genes.size(); ++i) {
        if (genes[i] < bounds[i].min) genes[i] = bounds[i].min;
        if (genes[i] > bounds[i].max) genes[i] = bounds[i].max;
    }
}

std::pair<Chromosome, Chromosome> Chromosome::crossoverTwo(const Chromosome& other, std::mt19937& rng) const {
    Chromosome child1 = *this;
    Chromosome child2 = other;

    constexpr size_t varSizes[] = {
        SPEEDCHANGE_GENES, HEADING_GENES, VERRATE_GENES,
        ALTITUDE_GENES, TIMEGAP_GENES, ANOMALY_GENES
    };
    constexpr size_t NUM_VARS = 6;

    std::uniform_real_distribution<double> dist(0.0, 1.0);

    size_t startIdx = 0;
    for (size_t v = 0; v < NUM_VARS; ++v) {
        size_t varSize = varSizes[v];

        if (dist(rng) < 0.5) {
            for (size_t i = 0; i < varSize; ++i) {
                std::swap(child1.genes[startIdx + i], child2.genes[startIdx + i]);
            }
        }

        startIdx += varSize;
    }

    child1.updateBounds();
    child2.updateBounds();
    child1.repair();
    child2.repair();

    return {child1, child2};
}
}
