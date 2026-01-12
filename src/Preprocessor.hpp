#pragma once

#include "adsb/AdsbState.hpp"
#include "features/FeatureVector.hpp"
#include "features/FeatureExtractor.hpp"
#include "adsb/AdsbCsvParser.hpp"

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace adsb {

struct TrainingSample {
    std::map<std::string, double> inputs;
    double expectedOutput;
    size_t originalIndex;
};

class AdsbDataPreprocessor {
public:
    struct Config {
        // Filtering thresholds
        double maxTimeGap = 60.0;           // seconds
        double maxSpeedChange = 50.0;       // m/s
        double maxHeadingChange = 180.0;    // degrees
        double maxVertRateChange = 50.0;    // m/s
        double maxAltitudeChange = 2000.0;  // meters
        
        // Normalization ranges (should match fuzzy variable ranges)
        double speedChangeRange = 10.0;
        double headingChangeRange = 180.0;
        double vertRateChangeRange = 20.0;
        double altitudeChangeRange = 1000.0;
        double timeGapMax = 60.0;
        
        // Labeling strategy
        enum class LabelStrategy {
            MANUAL,              // Use provided labels
            THRESHOLD_BASED,     // Auto-label based on thresholds
            EXPERT_RULES         // Complex rule-based labeling
        } labelStrategy = LabelStrategy::EXPERT_RULES;
        
        // Expert rule thresholds for auto-labeling
        double lowAnomalyThreshold = 0.3;
        double highAnomalyThreshold = 0.7;
    };

    explicit AdsbDataPreprocessor(const Config& config = Config())
        : config_(config) {}

    std::pair<std::vector<std::map<std::string, double>>, std::vector<double>>
    process(const std::string& csvPath, 
            const std::vector<double>& manualLabels = {}) {
        
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
        
        std::cout << "Generating labels...\n";
        auto labeled = labelSamples(filtered, manualLabels);
        
        std::vector<std::map<std::string, double>> inputs;
        std::vector<double> outputs;
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
            
            // Normalize features to fuzzy variable ranges
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
            
            // Check against thresholds
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
            
            // Check for NaN values
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

    std::vector<TrainingSample> labelSamples(
        const std::vector<TrainingSample>& samples,
        const std::vector<double>& manualLabels) {
        
        std::vector<TrainingSample> labeled = samples;
        
        switch (config_.labelStrategy) {
            case Config::LabelStrategy::MANUAL:
                return applyManualLabels(labeled, manualLabels);
                
            case Config::LabelStrategy::THRESHOLD_BASED:
                return applyThresholdLabels(labeled);
                
            case Config::LabelStrategy::EXPERT_RULES:
                return applyExpertRules(labeled);
        }
        
        return labeled;
    }

    std::vector<TrainingSample> applyManualLabels(
        std::vector<TrainingSample> samples,
        const std::vector<double>& labels) {
        
        if (labels.empty()) {
            throw std::runtime_error("Manual labels requested but none provided");
        }
        
        if (labels.size() != samples.size()) {
            throw std::runtime_error("Label count mismatch");
        }
        
        for (size_t i = 0; i < samples.size(); ++i) {
            samples[i].expectedOutput = std::clamp(labels[i], 0.0, 1.0);
        }
        
        return samples;
    }

    std::vector<TrainingSample> applyThresholdLabels(std::vector<TrainingSample> samples) {
        for (auto& sample : samples) {
            double anomalyScore = 0.0;
            int factors = 0;
            
            // Score based on magnitude of changes
            if (std::abs(sample.inputs.at("SpeedChange")) > 3.0) {
                anomalyScore += 0.25;
                factors++;
            }
            if (std::abs(sample.inputs.at("HeadingChange")) > 30.0) {
                anomalyScore += 0.25;
                factors++;
            }
            if (std::abs(sample.inputs.at("VerticalRateChange")) > 5.0) {
                anomalyScore += 0.25;
                factors++;
            }
            if (std::abs(sample.inputs.at("AltitudeChange")) > 200.0) {
                anomalyScore += 0.25;
                factors++;
            }
            
            sample.expectedOutput = std::clamp(anomalyScore, 0.0, 1.0);
        }
        
        return samples;
    }

    std::vector<TrainingSample> applyExpertRules(std::vector<TrainingSample> samples) {
        for (auto& sample : samples) {
            double speed = sample.inputs.at("SpeedChange");
            double heading = sample.inputs.at("HeadingChange");
            double vertRate = sample.inputs.at("VerticalRateChange");
            double altitude = sample.inputs.at("AltitudeChange");
            double timeGap = sample.inputs.at("TimeGap");
            
            double anomalyLevel = 0.0;
            
            // Rule 1: Normal cruise behavior (low anomaly)
            if (std::abs(speed) < 1.0 && std::abs(heading) < 5.0 && 
                std::abs(vertRate) < 1.0 && timeGap < 5.0) {
                anomalyLevel = 0.1;
            }
            // Rule 2: Coordinated maneuver (low-medium anomaly)
            else if (std::abs(speed) < 2.0 && std::abs(heading) < 20.0 && 
                     std::abs(vertRate) < 3.0) {
                anomalyLevel = 0.3;
            }
            // Rule 3: Aggressive maneuver (medium-high anomaly)
            else if (std::abs(speed) > 4.0 || std::abs(heading) > 45.0 || 
                     std::abs(vertRate) > 7.0) {
                anomalyLevel = 0.7;
            }
            // Rule 4: Extreme changes (high anomaly)
            else if (std::abs(speed) > 7.0 || std::abs(heading) > 90.0 || 
                     std::abs(vertRate) > 12.0 || std::abs(altitude) > 500.0) {
                anomalyLevel = 0.9;
            }
            // Rule 5: Combined anomalies with small time gap (high anomaly)
            else if ((std::abs(speed) > 3.0 && std::abs(heading) > 30.0) && 
                     timeGap < 3.0) {
                anomalyLevel = 0.8;
            }
            // Rule 6: Large time gap reduces anomaly significance
            else if (timeGap > 15.0) {
                anomalyLevel = 0.2;
            }
            // Default: moderate anomaly
            else {
                anomalyLevel = 0.5;
            }
            
            sample.expectedOutput = std::clamp(anomalyLevel, 0.0, 1.0);
        }
        
        return samples;
    }

    // Normalization functions
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

    double normalizeTimeGap(double raw) {
        return std::clamp(raw, 0.0, config_.timeGapMax);
    }

    void printStatistics(const std::vector<std::map<std::string, double>>& inputs,
                        const std::vector<double>& outputs) {
        std::cout << "\n=== Dataset Statistics ===\n";
        std::cout << "Total samples: " << inputs.size() << "\n\n";
        
        // Output distribution
        int low = 0, medium = 0, high = 0;
        for (double out : outputs) {
            if (out < 0.4) low++;
            else if (out < 0.7) medium++;
            else high++;
        }
        
        std::cout << "Anomaly distribution:\n";
        std::cout << "  Low (< 0.4):    " << low << " (" 
                  << (100.0 * low / outputs.size()) << "%)\n";
        std::cout << "  Medium (0.4-0.7): " << medium << " (" 
                  << (100.0 * medium / outputs.size()) << "%)\n";
        std::cout << "  High (> 0.7):   " << high << " (" 
                  << (100.0 * high / outputs.size()) << "%)\n";
        
        // Feature ranges
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
            std::cout << "  " << key << ": [" << minVal << ", " << maxVal 
                      << "] mean=" << mean << "\n";
        }
        std::cout << "\n";
    }
};
}
