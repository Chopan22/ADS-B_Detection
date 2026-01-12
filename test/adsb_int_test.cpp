#include "../src/ga/GAEngine.hpp"
#include "../src/ga/Fitness.hpp"
#include "../src/fuzzy/FuzzyInferenceSystem.hpp"
#include "../src/fuzzy/AdsbFuzzyVariable.hpp"
#include "../src/fuzzy/RuleBase.hpp"
#include "../src/AdsbDataPreprocessor.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <cmath>
#include <iomanip>

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

class RealAdsbValidator {
public:
    static ValidationMetrics evaluate(
        const std::vector<std::map<std::string, double>>& inputs,
        const std::vector<double>& expected,
        const std::vector<double>& predicted,
        double threshold = 0.5) {
        
        if (inputs.size() != expected.size() || expected.size() != predicted.size()) {
            throw std::runtime_error("Size mismatch in validation data");
        }
        
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
    
    static void printMetrics(const std::string& label, const ValidationMetrics& m) {
        std::cout << "\n=== " << label << " ===\n";
        std::cout << std::fixed << std::setprecision(4);
        
        std::cout << "Regression Metrics:\n";
        std::cout << "  MSE:        " << m.mse << "\n";
        std::cout << "  MAE:        " << m.mae << "\n";
        std::cout << "  RMSE:       " << m.rmse << "\n";
        std::cout << "  R²:         " << m.r_squared << "\n";
        
        std::cout << "\nClassification Metrics (threshold=0.5):\n";
        std::cout << "  Accuracy:   " << m.accuracy() << "\n";
        std::cout << "  Precision:  " << m.precision() << "\n";
        std::cout << "  Recall:     " << m.recall() << "\n";
        std::cout << "  F1-Score:   " << m.f1_score() << "\n";
        
        std::cout << "\nConfusion Matrix:\n";
        std::cout << "              Predicted Neg  Predicted Pos\n";
        std::cout << "  Actual Neg: " << std::setw(12) << m.true_negatives 
                  << "  " << std::setw(12) << m.false_positives << "\n";
        std::cout << "  Actual Pos: " << std::setw(12) << m.false_negatives 
                  << "  " << std::setw(12) << m.true_positives << "\n";
        std::cout << "\n";
    }
};

std::vector<double> evaluateFuzzySystem(
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

void saveResults(const std::string& filename, 
                const ga::Chromosome& best,
                const ValidationMetrics& trainMetrics,
                const ValidationMetrics& valMetrics) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Warning: Could not save results to " << filename << "\n";
        return;
    }
    
    out << "# Optimized Fuzzy System Parameters\n";
    out << "# Generated by GA optimization on real ADS-B data\n\n";
    
    out << "# Training MSE: " << trainMetrics.mse << "\n";
    out << "# Training F1:  " << trainMetrics.f1_score() << "\n";
    out << "# Validation MSE: " << valMetrics.mse << "\n";
    out << "# Validation F1:  " << valMetrics.f1_score() << "\n\n";
    
    out << "# Chromosome genes (66 parameters):\n";
    for (size_t i = 0; i < best.genes.size(); ++i) {
        out << best.genes[i];
        if (i < best.genes.size() - 1) out << ",";
        if ((i + 1) % 13 == 0) out << "\n";
    }
    
    out.close();
    std::cout << "Results saved to " << filename << "\n";
}

int main(int argc, char* argv[]) {
    std::cout << "=========================================\n";
    std::cout << "REAL ADS-B DATA OPTIMIZATION TEST\n";
    std::cout << "=========================================\n\n";
    
    // Parse command line arguments
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <adsb_csv_file> [options]\n";
        std::cerr << "\nOptions:\n";
        std::cerr << "  --generations N    Number of generations (default: 100)\n";
        std::cerr << "  --population N     Population size (default: 100)\n";
        std::cerr << "  --train-split R    Training split ratio (default: 0.8)\n";
        std::cerr << "  --output FILE      Save results to file (default: results.txt)\n";
        return 1;
    }
    
    std::string csvPath = argv[1];
    int generations = 100;
    int populationSize = 100;
    double trainSplit = 0.8;
    std::string outputFile = "results.txt";
    
    // Parse optional arguments
    for (int i = 2; i < argc - 1; i += 2) {
        std::string arg = argv[i];
        if (arg == "--generations") generations = std::stoi(argv[i + 1]);
        else if (arg == "--population") populationSize = std::stoi(argv[i + 1]);
        else if (arg == "--train-split") trainSplit = std::stod(argv[i + 1]);
        else if (arg == "--output") outputFile = argv[i + 1];
    }
    
    std::cout << "Configuration:\n";
    std::cout << "  CSV file:       " << csvPath << "\n";
    std::cout << "  Generations:    " << generations << "\n";
    std::cout << "  Population:     " << populationSize << "\n";
    std::cout << "  Train/Val split: " << trainSplit << "/" << (1.0 - trainSplit) << "\n";
    std::cout << "  Output file:    " << outputFile << "\n\n";
    
    try {
        // Step 1: Load and preprocess data
        std::cout << "STEP 1: Data Preprocessing\n";
        std::cout << "===========================\n";
        
        adsb::AdsbDataPreprocessor::Config config;
        config.labelStrategy = adsb::AdsbDataPreprocessor::Config::LabelStrategy::EXPERT_RULES;
        
        adsb::AdsbDataPreprocessor preprocessor(config);
        auto [inputs, outputs] = preprocessor.process(csvPath);
        
        if (inputs.empty()) {
            std::cerr << "Error: No valid samples after preprocessing\n";
            return 1;
        }
        
        // Step 2: Split into training and validation sets
        std::cout << "STEP 2: Train/Validation Split\n";
        std::cout << "===============================\n";
        
        size_t trainSize = static_cast<size_t>(inputs.size() * trainSplit);
        
        std::vector<std::map<std::string, double>> trainInputs(
            inputs.begin(), inputs.begin() + trainSize);
        std::vector<double> trainOutputs(
            outputs.begin(), outputs.begin() + trainSize);
        
        std::vector<std::map<std::string, double>> valInputs(
            inputs.begin() + trainSize, inputs.end());
        std::vector<double> valOutputs(
            outputs.begin() + trainSize, outputs.end());
        
        std::cout << "Training samples:   " << trainInputs.size() << "\n";
        std::cout << "Validation samples: " << valInputs.size() << "\n\n";
        
        // Step 3: Evaluate baseline (default parameters)
        std::cout << "STEP 3: Baseline Evaluation\n";
        std::cout << "============================\n";
        
        ga::Chromosome defaultChromosome;
        
        auto defaultTrainPred = evaluateFuzzySystem(trainInputs, defaultChromosome);
        auto defaultValPred = evaluateFuzzySystem(valInputs, defaultChromosome);
        
        auto baselineTrainMetrics = RealAdsbValidator::evaluate(
            trainInputs, trainOutputs, defaultTrainPred);
        auto baselineValMetrics = RealAdsbValidator::evaluate(
            valInputs, valOutputs, defaultValPred);
        
        RealAdsbValidator::printMetrics("Baseline Training", baselineTrainMetrics);
        RealAdsbValidator::printMetrics("Baseline Validation", baselineValMetrics);
        
        // Step 4: Run GA optimization
        std::cout << "STEP 4: GA Optimization\n";
        std::cout << "=======================\n";
        
        ga::Fitness fitness(trainInputs, trainOutputs);
        ga::GAEngine ga(populationSize, generations, 0.8, 0.2, 3);
        ga.setFitnessEvaluator(&fitness);
        
        std::cout << "Starting optimization...\n\n";
        ga.run();
        
        // Step 5: Evaluate optimized system
        std::cout << "\nSTEP 5: Optimized System Evaluation\n";
        std::cout << "====================================\n";
        
        const auto& bestChromosome = ga.bestChromosome();
        
        auto optTrainPred = evaluateFuzzySystem(trainInputs, bestChromosome);
        auto optValPred = evaluateFuzzySystem(valInputs, bestChromosome);
        
        auto optTrainMetrics = RealAdsbValidator::evaluate(
            trainInputs, trainOutputs, optTrainPred);
        auto optValMetrics = RealAdsbValidator::evaluate(
            valInputs, valOutputs, optValPred);
        
        RealAdsbValidator::printMetrics("Optimized Training", optTrainMetrics);
        RealAdsbValidator::printMetrics("Optimized Validation", optValMetrics);
        
        // Step 6: Summary and comparison
        std::cout << "STEP 6: Final Summary\n";
        std::cout << "=====================\n\n";
        
        double trainImprovement = 
            ((optTrainMetrics.f1_score() - baselineTrainMetrics.f1_score()) / 
             baselineTrainMetrics.f1_score()) * 100.0;
        double valImprovement = 
            ((optValMetrics.f1_score() - baselineValMetrics.f1_score()) / 
             baselineValMetrics.f1_score()) * 100.0;
        
        std::cout << "Performance Comparison:\n";
        std::cout << "                    Baseline    Optimized    Improvement\n";
        std::cout << "  Train F1:         " << std::setw(8) << baselineTrainMetrics.f1_score()
                  << "    " << std::setw(8) << optTrainMetrics.f1_score()
                  << "    " << std::setw(8) << trainImprovement << "%\n";
        std::cout << "  Val F1:           " << std::setw(8) << baselineValMetrics.f1_score()
                  << "    " << std::setw(8) << optValMetrics.f1_score()
                  << "    " << std::setw(8) << valImprovement << "%\n";
        
        std::cout << "\n  Train MSE:        " << std::setw(8) << baselineTrainMetrics.mse
                  << "    " << std::setw(8) << optTrainMetrics.mse << "\n";
        std::cout << "  Val MSE:          " << std::setw(8) << baselineValMetrics.mse
                  << "    " << std::setw(8) << optValMetrics.mse << "\n\n";
        
        // Check for overfitting
        double trainValGap = optTrainMetrics.f1_score() - optValMetrics.f1_score();
        if (trainValGap > 0.1) {
            std::cout << "Warning: Possible overfitting detected\n";
            std::cout << "  Train/Val F1 gap: " << trainValGap << "\n\n";
        }
        
        // Save results
        saveResults(outputFile, bestChromosome, optTrainMetrics, optValMetrics);
        
        std::cout << "\n=========================================\n";
        std::cout << "OPTIMIZATION COMPLETE\n";
        std::cout << "=========================================\n";
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << "\n";
        return 1;
    }
}
