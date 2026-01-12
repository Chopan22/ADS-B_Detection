#pragma once

#include "../ga/Chromosome.hpp"
#include "../fuzzy/FuzzyInferenceSystem.hpp"
#include "../fuzzy/AdsbFuzzyVariable.hpp"
#include "../fuzzy/RuleBase.hpp"

#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cmath>

namespace analysis {

struct ValidationMetrics {
    double mse;
    double mae;
    double rmse;
    double r_squared;
    
    int true_positives;
    int false_positives;
    int true_negatives;
    int false_negatives;
    
    double precision() const {
        int total = true_positives + false_positives;
        return total > 0 ? static_cast<double>(true_positives) / total : 0.0;
    }
    
    double recall() const {
        int total = true_positives + false_negatives;
        return total > 0 ? static_cast<double>(true_positives) / total : 0.0;
    }
    
    double f1_score() const {
        double p = precision();
        double r = recall();
        return (p + r > 0) ? 2 * p * r / (p + r) : 0.0;
    }
    
    double accuracy() const {
        int total = true_positives + true_negatives + false_positives + false_negatives;
        return total > 0 ? 
            static_cast<double>(true_positives + true_negatives) / total : 0.0;
    }
};

class Validator {
public:
    static ValidationMetrics evaluate(
        const std::vector<std::map<std::string, double>>& inputs,
        const std::vector<double>& expected,
        const ga::Chromosome& chromo,
        double threshold = 0.5) {
        
        auto predicted = evaluateFuzzySystem(inputs, chromo);
        return calculateMetrics(expected, predicted, threshold);
    }
    
    static void printMetrics(const std::string& label, const ValidationMetrics& m) {
        std::cout << "\n=== " << label << " ===\n";
        std::cout << std::fixed << std::setprecision(4);
        
        std::cout << "Regression Metrics:\n";
        std::cout << "  MSE:        " << m.mse << "\n";
        std::cout << "  MAE:        " << m.mae << "\n";
        std::cout << "  RMSE:       " << m.rmse << "\n";
        std::cout << "  R²:         " << m.r_squared << "\n";
        
        std::cout << "\nClassification Metrics:\n";
        std::cout << "  Accuracy:   " << m.accuracy() << "\n";
        std::cout << "  Precision:  " << m.precision() << "\n";
        std::cout << "  Recall:     " << m.recall() << "\n";
        std::cout << "  F1-Score:   " << m.f1_score() << "\n";
        
        std::cout << "\nConfusion Matrix:\n";
        std::cout << "              Pred Neg   Pred Pos\n";
        std::cout << "  Actual Neg: " << std::setw(8) << m.true_negatives 
                  << "   " << std::setw(8) << m.false_positives << "\n";
        std::cout << "  Actual Pos: " << std::setw(8) << m.false_negatives 
                  << "   " << std::setw(8) << m.true_positives << "\n";
        std::cout << "\n";
    }
    
    static void saveDetailedResults(
        const std::vector<std::map<std::string, double>>& trainInputs,
        const std::vector<double>& trainOutputs,
        const std::vector<std::map<std::string, double>>& valInputs,
        const std::vector<double>& valOutputs,
        const ValidationMetrics& baselineTrain,
        const ValidationMetrics& baselineVal,
        const ValidationMetrics& optTrain,
        const ValidationMetrics& optVal,
        const ga::Chromosome& optimized) {
        
        savePredictionsCsv(valInputs, valOutputs, optimized, "results/predictions.csv");
        
        saveErrorAnalysis(valInputs, valOutputs, optimized, "results/error_analysis.txt");
        
        saveMetricsSummary(baselineTrain, baselineVal, optTrain, optVal, 
                          "results/metrics_summary.txt");
        
        std::cout << "✓ Detailed results saved to results/ directory\n";
        std::cout << "  - predictions.csv: Prediction results\n";
        std::cout << "  - error_analysis.txt: Error breakdown\n";
        std::cout << "  - metrics_summary.txt: Performance metrics\n";
        std::cout << "\nRun the analysis tool to generate graphs:\n";
        std::cout << "  python3 tools/analyze_results.py\n";
    }

private:
    static std::vector<double> evaluateFuzzySystem(
        const std::vector<std::map<std::string, double>>& inputs,
        const ga::Chromosome& chromo) {
        
        size_t idx = 0;
        auto nextGenes = [&](size_t count) -> std::vector<double> {
            std::vector<double> v(chromo.genes.begin() + idx, 
                                 chromo.genes.begin() + idx + count);
            idx += count;
            return v;
        };
        
        auto speedVar = fuzzy::createSpeedChangeVariable(nextGenes(13));
        auto headingVar = fuzzy::createHeadingChangeVariable(nextGenes(13));
        auto vertVar = fuzzy::createVerticalRateChangeVariable(nextGenes(13));
        auto altVar = fuzzy::createAltitudeChangeVariable(nextGenes(13));
        auto timeVar = fuzzy::createTimeGapVariable(nextGenes(7));
        auto anomalyVar = fuzzy::createAnomalyLevelVariable(nextGenes(7));
        
        fuzzy::FuzzyInferenceSystem fis;
        fis.addInputVariable(speedVar);
        fis.addInputVariable(headingVar);
        fis.addInputVariable(vertVar);
        fis.addInputVariable(altVar);
        fis.addInputVariable(timeVar);
        fis.setOutputVariable(anomalyVar);
        
        for (const auto& rule : fuzzy::createAdsbRuleBase()) {
            fis.addRule(rule);
        }
        
        std::vector<double> outputs;
        outputs.reserve(inputs.size());
        
        for (const auto& input : inputs) {
            outputs.push_back(fis.evaluate(input));
        }
        
        return outputs;
    }
    
    static ValidationMetrics calculateMetrics(
        const std::vector<double>& expected,
        const std::vector<double>& predicted,
        double threshold) {
        
        ValidationMetrics metrics = {};
        
        double sumSquaredError = 0.0;
        double sumAbsError = 0.0;
        double meanExpected = 0.0;
        
        for (size_t i = 0; i < expected.size(); ++i) {
            double error = predicted[i] - expected[i];
            sumSquaredError += error * error;
            sumAbsError += std::abs(error);
            meanExpected += expected[i];
            
            bool predAnomaly = predicted[i] > threshold;
            bool trueAnomaly = expected[i] > threshold;
            
            if (predAnomaly && trueAnomaly) metrics.true_positives++;
            else if (predAnomaly && !trueAnomaly) metrics.false_positives++;
            else if (!predAnomaly && !trueAnomaly) metrics.true_negatives++;
            else metrics.false_negatives++;
        }
        
        size_t n = expected.size();
        metrics.mse = sumSquaredError / n;
        metrics.mae = sumAbsError / n;
        metrics.rmse = std::sqrt(metrics.mse);
        
        meanExpected /= n;
        
        double sumSquaredTotal = 0.0;
        for (double exp : expected) {
            double diff = exp - meanExpected;
            sumSquaredTotal += diff * diff;
        }
        
        metrics.r_squared = (sumSquaredTotal > 0) ? 
            1.0 - (sumSquaredError / sumSquaredTotal) : 0.0;
        
        return metrics;
    }
    
    static void savePredictionsCsv(
        const std::vector<std::map<std::string, double>>& inputs,
        const std::vector<double>& expected,
        const ga::Chromosome& chromo,
        const std::string& filename) {
        
        auto predicted = evaluateFuzzySystem(inputs, chromo);
        
        std::ofstream out(filename);
        if (!out.is_open()) return;
        
        out << "Index,Expected,Predicted,Error,AbsError,SpeedChange,HeadingChange,"
            << "VerticalRateChange,AltitudeChange,TimeGap\n";
        
        for (size_t i = 0; i < inputs.size(); ++i) {
            double error = predicted[i] - expected[i];
            out << i << ","
                << expected[i] << ","
                << predicted[i] << ","
                << error << ","
                << std::abs(error) << ","
                << inputs[i].at("SpeedChange") << ","
                << inputs[i].at("HeadingChange") << ","
                << inputs[i].at("VerticalRateChange") << ","
                << inputs[i].at("AltitudeChange") << ","
                << inputs[i].at("TimeGap") << "\n";
        }
        
        out.close();
    }
    
    static void saveErrorAnalysis(
        const std::vector<std::map<std::string, double>>& inputs,
        const std::vector<double>& expected,
        const ga::Chromosome& chromo,
        const std::string& filename) {
        
        auto predicted = evaluateFuzzySystem(inputs, chromo);
        
        struct ErrorSample {
            size_t index;
            double expected;
            double predicted;
            double absError;
            std::map<std::string, double> inputs;
        };
        
        std::vector<ErrorSample> errors;
        for (size_t i = 0; i < expected.size(); ++i) {
            errors.push_back({
                i, expected[i], predicted[i],
                std::abs(predicted[i] - expected[i]),
                inputs[i]
            });
        }
        
        std::sort(errors.begin(), errors.end(),
            [](const ErrorSample& a, const ErrorSample& b) {
                return a.absError > b.absError;
            });
        
        std::ofstream out(filename);
        if (!out.is_open()) return;
        
        out << "Error Analysis Report\n";
        out << "=====================\n\n";
        
        out << "Top 20 Largest Errors:\n";
        out << std::string(80, '-') << "\n";
        out << std::setw(6) << "Rank"
            << std::setw(8) << "Index"
            << std::setw(12) << "Expected"
            << std::setw(12) << "Predicted"
            << std::setw(12) << "AbsError\n";
        out << std::string(80, '-') << "\n";
        
        for (size_t i = 0; i < std::min(size_t(20), errors.size()); ++i) {
            const auto& e = errors[i];
            out << std::setw(6) << (i + 1)
                << std::setw(8) << e.index
                << std::setw(12) << std::fixed << std::setprecision(4) << e.expected
                << std::setw(12) << e.predicted
                << std::setw(12) << e.absError << "\n";
        }
        
        out.close();
    }
    
    static void saveMetricsSummary(
        const ValidationMetrics& baselineTrain,
        const ValidationMetrics& baselineVal,
        const ValidationMetrics& optTrain,
        const ValidationMetrics& optVal,
        const std::string& filename) {
        
        std::ofstream out(filename);
        if (!out.is_open()) return;
        
        out << "Metrics Summary\n";
        out << "===============\n\n";
        
        out << std::setw(20) << std::left << "Metric"
            << std::setw(15) << "Baseline Train"
            << std::setw(15) << "Baseline Val"
            << std::setw(15) << "Opt Train"
            << std::setw(15) << "Opt Val\n";
        out << std::string(80, '-') << "\n";
        
        out << std::fixed << std::setprecision(4);
        out << std::setw(20) << std::left << "F1 Score"
            << std::setw(15) << baselineTrain.f1_score()
            << std::setw(15) << baselineVal.f1_score()
            << std::setw(15) << optTrain.f1_score()
            << std::setw(15) << optVal.f1_score() << "\n";
        
        out << std::setw(20) << std::left << "Accuracy"
            << std::setw(15) << baselineTrain.accuracy()
            << std::setw(15) << baselineVal.accuracy()
            << std::setw(15) << optTrain.accuracy()
            << std::setw(15) << optVal.accuracy() << "\n";
        
        out << std::setw(20) << std::left << "MSE"
            << std::setw(15) << baselineTrain.mse
            << std::setw(15) << baselineVal.mse
            << std::setw(15) << optTrain.mse
            << std::setw(15) << optVal.mse << "\n";
        
        out.close();
    }
};
}
