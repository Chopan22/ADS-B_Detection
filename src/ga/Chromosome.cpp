#include "Chromosome.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <stdexcept>

namespace ga {

constexpr size_t SPEEDCHANGE_GENES = 13;
constexpr size_t HEADING_GENES = 13;
constexpr size_t VERRATE_GENES = 13;
constexpr size_t ALTITUDE_GENES = 13;
constexpr size_t TIMEGAP_GENES = 7;
constexpr size_t ANOMALY_GENES = 7;

constexpr size_t TOTAL_GENES_FULL = SPEEDCHANGE_GENES + HEADING_GENES + VERRATE_GENES +
                                    ALTITUDE_GENES + TIMEGAP_GENES + ANOMALY_GENES;

constexpr VariableScope VARIABLE_SCOPES[] = {
    {-10.0, 10.0},     // SpeedChange
    {-180.0, 180.0},   // HeadingChange
    {-20.0, 20.0},     // VerticalRateChange
    {-1000.0, 1000.0}, // AltitudeChange
    {0.0, 60.0},       // TimeGap
    {0.0, 1.0}         // AnomalyLevel
};

#ifdef GA_TEST_MODE
const std::vector<double> Chromosome::DEFAULT_GENES = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
#else
const std::vector<double> Chromosome::DEFAULT_GENES = {
    -6.0,   -3.0,        // SpeedChange Negative_Large
    -6.0,   -3.0,   0.0, // SpeedChange Negative
    -1.0,   0.0,    1.0, // SpeedChange Zero
    0.0,    3.0,    6.0, // SpeedChange Positive
    3.0,    6.0,         // SpeedChange Positive_Large

    -90.0,  -30.0,        // HeadingChange Negative_Large
    -60.0,  -20.0,  0.0,  // HeadingChange Negative
    -5.0,   0.0,    5.0,  // HeadingChange Zero
    0.0,    20.0,   60.0, // HeadingChange Positive
    30.0,   90.0,         // HeadingChange Positive_Large

    -10.0,  -5.0,         // VerticalRataChange Negative_Large
    -10.0,  -5.0,   0.0,  // VerticalRataChange Negative
    -1.0,   0.0,    1.0,  // VerticalRataChange Zero
    0.0,    5.0,    10.0, // VerticalRataChange Positive
    5.0,    10.0,         // VerticalRataChange Positive_Large

    -500.0, -100.0,        // AltitudeChange Negative_Large
    -500.0, -100.0, 0.0,   // AltitudeChange Negative
    -50.0,  0.0,    50.0,  // AltitudeChange Zero
    0.0,    100.0,  500.0, // AltitudeChange Positive
    100.0,  500.0,         // AltitudeChange Positive_Large

    1.0,    5.0,          // TimeGap Small
    3.0,    10.0,   20.0, // TimeGap Medium
    15.0,   30.0,         // TimeGap Large

    0.2,    0.4,         // AnomalyLevel Low
    0.2,    0.5,    0.8, // AnomalyLevel Medium
    0.6,    0.8          // AnomalyLevel High
};
#endif

Chromosome::Chromosome() {
#ifdef GA_TEST_MODE
  if (DEFAULT_GENES.size() != 6)
    throw std::runtime_error("DEFAULT_GENES size does not match expected test size");
#else
  if (DEFAULT_GENES.size() != TOTAL_GENES_FULL)
    throw std::runtime_error("DEFAULT_GENES size does not match TOTAL_GENES");
#endif

  genes = DEFAULT_GENES;
  bounds.resize(genes.size());
  updateBounds();
  repair();
}

void Chromosome::updateBounds() {
#ifdef GA_TEST_MODE
  for (size_t i = 0; i < genes.size(); ++i) {
    bounds[i].min = 0.0;
    bounds[i].max = 10.0;
  }
#else
  auto getVarIndexAndOffset = [&](size_t i, size_t& offset) -> size_t {
    size_t start = 0;
    size_t var_idx = 0;
    size_t sizes[] = {SPEEDCHANGE_GENES, HEADING_GENES, VERRATE_GENES,
                      ALTITUDE_GENES,    TIMEGAP_GENES, ANOMALY_GENES};
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
    case 0:
      var_size = SPEEDCHANGE_GENES;
      break;
    case 1:
      var_size = HEADING_GENES;
      break;
    case 2:
      var_size = VERRATE_GENES;
      break;
    case 3:
      var_size = ALTITUDE_GENES;
      break;
    case 4:
      var_size = TIMEGAP_GENES;
      break;
    case 5:
      var_size = ANOMALY_GENES;
      break;
    default:
      var_size = 1;
      break;
    }

    double var_min = VARIABLE_SCOPES[var_idx].min;
    double var_max = VARIABLE_SCOPES[var_idx].max;

    if (var_idx >= 0 && var_idx <= 3) {
      switch (offset) {
      case 0:
        bounds[i].min = var_min;
        bounds[i].max = genes[i + 2];
        break;
      case 1:
        bounds[i].min = genes[i + 1];
        bounds[i].max = genes[i + 2];
        break;
      case 2:
        bounds[i].min = genes[i - 2];
        bounds[i].max = genes[i - 1];
        break;
      case 3:
        bounds[i].min = genes[i - 2];
        bounds[i].max = genes[i + 2];
        break;
      case 4:
        bounds[i].min = genes[i + 1];
        bounds[i].max = genes[i + 2];
        break;
      case 5:
        bounds[i].min = genes[i - 2];
        bounds[i].max = genes[i - 1];
        break;
      case 6:
        bounds[i].min = genes[i - 2];
        bounds[i].max = genes[i + 2];
        break;
      case 7:
        bounds[i].min = genes[i + 1];
        bounds[i].max = genes[i + 2];
        break;
      case 8:
        bounds[i].min = genes[i - 2];
        bounds[i].max = genes[i - 1];
        break;
      case 9:
        bounds[i].min = genes[i - 2];
        bounds[i].max = genes[i + 2];
        break;
      case 10:
        bounds[i].min = genes[i + 1];
        bounds[i].max = genes[i + 2];
        break;
      case 11:
        bounds[i].min = genes[i - 2];
        bounds[i].max = genes[i - 1];
        break;
      case 12:
        bounds[i].min = genes[i - 2];
        bounds[i].max = var_max;
        break;
      }
    } else {
      switch (offset) {
      case 0:
        bounds[i].min = var_min;
        bounds[i].max = genes[i + 2];
        break;
      case 1:
        bounds[i].min = genes[i + 1];
        bounds[i].max = genes[i + 2];
        break;
      case 2:
        bounds[i].min = genes[i - 2];
        bounds[i].max = genes[i - 1];
        break;
      case 3:
        bounds[i].min = genes[i - 2];
        bounds[i].max = genes[i + 2];
        break;
      case 4:
        bounds[i].min = genes[i + 1];
        bounds[i].max = genes[i + 2];
        break;
      case 5:
        bounds[i].min = genes[i - 2];
        bounds[i].max = genes[i - 1];
        break;
      case 6:
        bounds[i].min = genes[i - 2];
        bounds[i].max = var_max;
        break;
      }
    }
    assert(bounds[i].min >= var_min);
    assert(bounds[i].max <= var_max);
  }
#endif
}

void Chromosome::repair() {
  assert(!genes.empty());

  for (size_t i = 0; i < genes.size(); ++i) {
    assert(i < bounds.size());

    if (genes[i] < bounds[i].min)
      genes[i] = bounds[i].min;
    if (genes[i] > bounds[i].max)
      genes[i] = bounds[i].max;

    assert(bounds[i].min <= genes[i]);
    assert(genes[i] <= bounds[i].max);
  }
}

void Chromosome::mutate(double mutationRate) {
  static std::random_device              rd;
  static std::mt19937                    gen(rd());
  std::uniform_real_distribution<double> prob(0.0, 1.0);

  for (size_t i = 0; i < genes.size(); ++i) {
    if (prob(gen) < mutationRate) {
      double                                 range = bounds[i].max - bounds[i].min;
      std::uniform_real_distribution<double> geneDist(bounds[i].min, bounds[i].max);
      genes[i] = geneDist(gen);
    }
  }
}

std::pair<Chromosome, Chromosome> Chromosome::crossoverTwo(const Chromosome& other,
                                                           std::mt19937&     rng) const {
  Chromosome child1 = *this;
  Chromosome child2 = other;

#ifdef GA_TEST_MODE
  std::uniform_int_distribution<size_t> dist(1, genes.size() - 1);
  size_t                                crossPoint = dist(rng);

  for (size_t i = crossPoint; i < genes.size(); ++i) {
    std::swap(child1.genes[i], child2.genes[i]);
  }
#else
  constexpr size_t varSizes[] = {SPEEDCHANGE_GENES, HEADING_GENES, VERRATE_GENES,
                                 ALTITUDE_GENES,    TIMEGAP_GENES, ANOMALY_GENES};
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
#endif

  child1.updateBounds();
  child2.updateBounds();
  child1.repair();
  child2.repair();

  return {child1, child2};
}

void Chromosome::crossover(const Chromosome& parent1, const Chromosome& parent2,
                           Chromosome& offspring1, Chromosome& offspring2) {
  static std::random_device rd;
  static std::mt19937       gen(rd());

  auto children = parent1.crossoverTwo(parent2, gen);
  offspring1 = children.first;
  offspring2 = children.second;
}
} // namespace ga
