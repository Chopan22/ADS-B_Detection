#pragma once

#include "../adsb/AdsbCsvParser.hpp"
#include "../adsb/AdsbState.hpp"
#include "../feature/FeatureExtractor.hpp"
#include "../feature/FeatureVector.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace adsb {

struct TrainingSample {
  std::map<std::string, double> inputs;
  double                        expectedOutput;
  size_t                        originalIndex;
};

class AdsbDataPreprocessor {
public:
  struct Config {
    // Filtering thresholds (initialized with default values)
    double maxTimeGap;          // seconds
    double maxSpeedChange;      // m/s
    double maxHeadingChange;    // degrees
    double maxVertRateChange;   // m/s
    double maxAltitudeChange;   // meters

    // Ranges
    double speedChangeRange;
    double headingChangeRange;
    double vertRateChangeRange;
    double altitudeChangeRange;
    double timeGapMax;

    // Constructor to initialize default values
    Config()
      : maxTimeGap(60.0), maxSpeedChange(50.0), maxHeadingChange(180.0),
        maxVertRateChange(50.0), maxAltitudeChange(2000.0),
        speedChangeRange(10.0), headingChangeRange(180.0),
        vertRateChangeRange(20.0), altitudeChangeRange(1000.0),
        timeGapMax(60.0) {}
  };

  explicit AdsbDataPreprocessor(const Config& config = Config()) : config_(config) {}

  std::pair<std::vector<std::map<std::string, double>>, std::vector<double>>
  process(const std::string& csvPath) {

    std::cout << "Loading ADS-B data from: " << csvPath << "\n";
    auto states = AdsbCsvParser::load(csvPath);
    std::cout << "Loaded " << states.size() << " ADS-B states\n";

    std::cout << "Extracting features...\n";
    auto features = FeatureExtractor::extract(states);
    std::cout << "Extracted " << features.size() << " feature vectors\n";

    std::cout << "Converting to training samples...\n";
    auto samples = convertToSamples(features);
    std::cout << "Created " << samples.size() << " samples\n";

    std::cout << "Filtering outliers...\n";
    auto filtered = filterOutliers(samples);
    std::cout << "Retained " << filtered.size() << " samples after filtering\n";

    std::cout << "Generating labels using expert rules...\n";
    auto labeled = applyExpertRules(filtered);

    std::vector<std::map<std::string, double>> inputs;
    std::vector<double>                        outputs;
    inputs.reserve(labeled.size());
    outputs.reserve(labeled.size());

    for (const auto& sample : labeled) {
      inputs.push_back(sample.inputs);
      outputs.push_back(sample.expectedOutput);
    }

    printStatistics(inputs, outputs);

    return {inputs, outputs};
  }

private:
  Config config_;

  std::vector<TrainingSample> convertToSamples(const std::vector<FeatureVector>& features) {
    std::vector<TrainingSample> samples;
    samples.reserve(features.size());

    for (size_t i = 0; i < features.size(); ++i) {
      const auto& fv = features[i];

      TrainingSample sample;
      sample.originalIndex = i;

      sample.inputs["SpeedChange"] = normalizeSpeedChange(fv.d_speed);
      sample.inputs["HeadingChange"] = normalizeHeadingChange(fv.d_heading);
      sample.inputs["VerticalRateChange"] = normalizeVerticalRate(fv.d_vert_rate);
      sample.inputs["AltitudeChange"] = normalizeAltitudeChange(fv.d_altitude);
      sample.inputs["TimeGap"] = normalizeTimeGap(fv.dt);

      samples.push_back(sample);
    }

    return samples;
  }

  std::vector<TrainingSample> filterOutliers(const std::vector<TrainingSample>& samples) {
    std::vector<TrainingSample> filtered;
    filtered.reserve(samples.size());

    for (const auto& sample : samples) {
      bool valid = true;

      if (std::abs(sample.inputs.at("SpeedChange")) > config_.speedChangeRange) {
        valid = false;
      }
      if (std::abs(sample.inputs.at("HeadingChange")) > config_.headingChangeRange) {
        valid = false;
      }
      if (std::abs(sample.inputs.at("VerticalRateChange")) > config_.vertRateChangeRange) {
        valid = false;
      }
      if (std::abs(sample.inputs.at("AltitudeChange")) > config_.altitudeChangeRange) {
        valid = false;
      }
      if (sample.inputs.at("TimeGap") > config_.maxTimeGap) {
        valid = false;
      }

      for (const auto& [key, value] : sample.inputs) {
        if (std::isnan(value) || std::isinf(value)) {
          valid = false;
          break;
        }
      }

      if (valid) {
        filtered.push_back(sample);
      }
    }
    return filtered;
  }

  std::vector<TrainingSample> applyExpertRules(std::vector<TrainingSample> samples) {
    for (auto& sample : samples) {
      double speed = sample.inputs.at("SpeedChange");
      double heading = sample.inputs.at("HeadingChange");
      double vertRate = sample.inputs.at("VerticalRateChange");
      double altitude = sample.inputs.at("AltitudeChange");
      double timeGap = sample.inputs.at("TimeGap");

      double anomalyLevel = 0.0;

      // 1. Rule: Extreme Physics / Boundary Violation (Score 0.9 - 1.0)
      // Close to or exceeding the defined maximum capability of the sensor/model.
      if (std::abs(speed) > 8.0 || std::abs(vertRate) > 15.0 || std::abs(altitude) > 800.0) {
          anomalyLevel = 1.0; 
      }

      // 2. Rule: Impossible Rotation (Score 0.9)
      // Turning > 90 degrees in a single update is physically impossible for a jet.
      else if (std::abs(heading) > 90.0) {
          anomalyLevel = 0.9;
      }

      // 3. Rule: Compound Aggressive Maneuver (Score 0.7 - 0.8)
      // Significant speed and heading changes happening simultaneously.
      else if (std::abs(speed) > 5.0 && std::abs(heading) > 45.0) {
          anomalyLevel = 0.8;
      }

      // 4. Rule: Performance Edge / Rapid Transition (Score 0.5)
      // Halfway to the limit. Unlikely for commercial flight but possible.
      else if (std::abs(speed) > 4.0 || std::abs(vertRate) > 8.0 || std::abs(heading) > 30.0) {
          anomalyLevel = 0.5;
      }

      // 5. Rule: Normal Operations / Coordinated Turns (Score 0.2)
      // Small deviations within expected flight envelopes.
      else if (std::abs(speed) > 1.0 || std::abs(heading) > 10.0 || std::abs(vertRate) > 2.0) {
          anomalyLevel = 0.2;
      }

      // 6. Rule: Uncertainty due to Time Gap
      // High deltas are expected if we haven't heard from the plane in 30+ seconds.
      else if (timeGap > 30.0) {
          anomalyLevel = 0.1; 
      }

      // Default: Smooth, Stable Flight
      else {
          anomalyLevel = 0.0;
      }

      sample.expectedOutput = std::clamp(anomalyLevel, 0.0, 1.0);
    }

    return samples;
  }

  double normalizeSpeedChange(double raw) {
    return std::clamp(raw, -config_.speedChangeRange, config_.speedChangeRange);
  }

  double normalizeHeadingChange(double raw) {
    return std::clamp(raw, -config_.headingChangeRange, config_.headingChangeRange);
  }

  double normalizeVerticalRate(double raw) {
    return std::clamp(raw, -config_.vertRateChangeRange, config_.vertRateChangeRange);
  }

  double normalizeAltitudeChange(double raw) {
    return std::clamp(raw, -config_.altitudeChangeRange, config_.altitudeChangeRange);
  }

  double normalizeTimeGap(double raw) { return std::clamp(raw, 0.0, config_.timeGapMax); }

  void printStatistics(const std::vector<std::map<std::string, double>>& inputs,
                       const std::vector<double>&                        outputs) {
    std::cout << "\n=== Dataset Statistics ===\n";
    std::cout << "Total samples: " << inputs.size() << "\n\n";

    int low = 0, medium = 0, high = 0;
    for (double out : outputs) {
      if (out < 0.4)
        low++;
      else if (out < 0.7)
        medium++;
      else
        high++;
    }

    std::cout << "Anomaly distribution:\n";
    std::cout << "  Low (< 0.4):      " << low << " (" << (100.0 * low / outputs.size()) << "%)\n";
    std::cout << "  Medium (0.4-0.7): " << medium << " (" << (100.0 * medium / outputs.size())
              << "%)\n";
    std::cout << "  High (> 0.7):     " << high << " (" << (100.0 * high / outputs.size())
              << "%)\n";

    std::cout << "\nFeature ranges:\n";
    for (const auto& [key, _] : inputs[0]) {
      double minVal = std::numeric_limits<double>::max();
      double maxVal = std::numeric_limits<double>::lowest();
      double sum = 0.0;

      for (const auto& input : inputs) {
        double val = input.at(key);
        minVal = std::min(minVal, val);
        maxVal = std::max(maxVal, val);
        sum += val;
      }

      double mean = sum / inputs.size();
      std::cout << "  " << key << ": [" << minVal << ", " << maxVal << "] mean=" << mean << "\n";
    }
    std::cout << "\n";
  }
};
} // namespace adsb
