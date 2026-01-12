#include "../src/ga/GAEngine.hpp"
#include "../src/ga/Fitness.hpp"
#include "../src/fuzzy/FuzzyInferenceSystem.hpp"
#include "../src/fuzzy/AdsbFuzzyVariable.hpp"
#include "../src/fuzzy/RuleBase.hpp"

#include <iostream>
#include <vector>
#include <map>
#include <random>

std::pair<std::vector<std::map<std::string, double>>, std::vector<double>>
generateSyntheticAdsbData(size_t numSamples) {
    std::vector<std::map<std::string, double>> inputs;
    std::vector<double> expectedOutputs;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    for (size_t i = 0; i < numSamples; ++i) {
        std::map<std::string, double> sample;
        
        double scenario = static_cast<double>(i % 5);
        
        switch (static_cast<int>(scenario)) {
            case 0: // Normal flight
                sample["SpeedChange"] = std::uniform_real_distribution<>(-1.0, 1.0)(gen);
                sample["HeadingChange"] = std::uniform_real_distribution<>(-5.0, 5.0)(gen);
                sample["VerticalRateChange"] = std::uniform_real_distribution<>(-1.0, 1.0)(gen);
                sample["AltitudeChange"] = std::uniform_real_distribution<>(-50.0, 50.0)(gen);
                sample["TimeGap"] = std::uniform_real_distribution<>(1.0, 5.0)(gen);
                expectedOutputs.push_back(0.2);  // Low anomaly
                break;
                
            case 1: // Moderate anomaly
                sample["SpeedChange"] = std::uniform_real_distribution<>(2.0, 4.0)(gen);
                sample["HeadingChange"] = std::uniform_real_distribution<>(10.0, 30.0)(gen);
                sample["VerticalRateChange"] = std::uniform_real_distribution<>(2.0, 4.0)(gen);
                sample["AltitudeChange"] = std::uniform_real_distribution<>(100.0, 300.0)(gen);
                sample["TimeGap"] = std::uniform_real_distribution<>(3.0, 10.0)(gen);
                expectedOutputs.push_back(0.5);  // Medium anomaly
                break;
                
            case 2: // High anomaly
                sample["SpeedChange"] = std::uniform_real_distribution<>(5.0, 8.0)(gen);
                sample["HeadingChange"] = std::uniform_real_distribution<>(50.0, 120.0)(gen);
                sample["VerticalRateChange"] = std::uniform_real_distribution<>(-15.0, -8.0)(gen);
                sample["AltitudeChange"] = std::uniform_real_distribution<>(300.0, 700.0)(gen);
                sample["TimeGap"] = std::uniform_real_distribution<>(1.0, 3.0)(gen);
                expectedOutputs.push_back(0.8);  // High anomaly
                break;
                
            case 3: // Large time gap (less anomalous)
                sample["SpeedChange"] = std::uniform_real_distribution<>(3.0, 5.0)(gen);
                sample["HeadingChange"] = std::uniform_real_distribution<>(15.0, 40.0)(gen);
                sample["VerticalRateChange"] = std::uniform_real_distribution<>(1.0, 3.0)(gen);
                sample["AltitudeChange"] = std::uniform_real_distribution<>(50.0, 150.0)(gen);
                sample["TimeGap"] = std::uniform_real_distribution<>(20.0, 50.0)(gen);
                expectedOutputs.push_back(0.3);  // Low-medium anomaly
                break;
                
            case 4: // Mixed scenario
                sample["SpeedChange"] = std::uniform_real_distribution<>(-3.0, 3.0)(gen);
                sample["HeadingChange"] = std::uniform_real_distribution<>(-20.0, 20.0)(gen);
                sample["VerticalRateChange"] = std::uniform_real_distribution<>(-5.0, 5.0)(gen);
                sample["AltitudeChange"] = std::uniform_real_distribution<>(-200.0, 200.0)(gen);
                sample["TimeGap"] = std::uniform_real_distribution<>(5.0, 15.0)(gen);
                expectedOutputs.push_back(0.4);  // Medium anomaly
                break;
        }
        
        inputs.push_back(sample);
    }
    
    return {inputs, expectedOutputs};
}

int main() {
    std::cout << "========================================\n";
    std::cout << "FUZZY + GA INTEGRATION TEST\n";
    std::cout << "========================================\n\n";
    
    // Generate synthetic ADSB test data
    std::cout << "Generating synthetic ADSB scenarios...\n";
    auto [testInputs, expectedOutputs] = generateSyntheticAdsbData(50);
    std::cout << "Generated " << testInputs.size() << " test scenarios\n\n";
    
    // Test 1: Verify fuzzy system works with default parameters
    std::cout << "Test 1: Fuzzy system with default parameters\n";
    std::cout << "----------------------------------------------\n";
    
    fuzzy::FuzzyInferenceSystem fis;
    fis.addInputVariable(fuzzy::createSpeedChangeVariable());
    fis.addInputVariable(fuzzy::createHeadingChangeVariable());
    fis.addInputVariable(fuzzy::createVerticalRateChangeVariable());
    fis.addInputVariable(fuzzy::createAltitudeChangeVariable());
    fis.addInputVariable(fuzzy::createTimeGapVariable());
    fis.setOutputVariable(fuzzy::createAnomalyLevelVariable());
    
    for (const auto& rule : fuzzy::createAdsbRuleBase()) {
        fis.addRule(rule);
    }
    
    double defaultMSE = 0.0;
    for (size_t i = 0; i < testInputs.size(); ++i) {
        double output = fis.evaluate(testInputs[i]);
        double error = output - expectedOutputs[i];
        defaultMSE += error * error;
    }
    defaultMSE /= testInputs.size();
    
    double defaultFitness = 1.0 / (1.0 + defaultMSE);
    std::cout << "Default MSE: " << defaultMSE << "\n";
    std::cout << "Default Fitness: " << defaultFitness << "\n\n";
    
    // Test 2: Optimize fuzzy parameters with GA
    std::cout << "Test 2: GA optimization of fuzzy parameters\n";
    std::cout << "----------------------------------------------\n";
    
    ga::Fitness fitness(testInputs, expectedOutputs);
    
    ga::GAEngine ga(
        100,   // population size
        30,    // generations (fewer for integration test)
        0.8,   // crossover probability
        0.2,   // mutation probability
        3      // tournament size
    );
    
    ga.setFitnessEvaluator(&fitness);
    
    std::cout << "Running GA optimization...\n\n";
    ga.run();
    
    std::cout << "\n========================================\n";
    std::cout << "INTEGRATION TEST RESULTS\n";
    std::cout << "========================================\n\n";
    
    std::cout << "Default Parameters:\n";
    std::cout << "  Fitness: " << defaultFitness << "\n";
    std::cout << "  MSE: " << defaultMSE << "\n\n";
    
    std::cout << "Optimized Parameters:\n";
    std::cout << "  Fitness: " << ga.bestFitness() << "\n";
    double optimizedMSE = (1.0 / ga.bestFitness()) - 1.0;
    std::cout << "  MSE: " << optimizedMSE << "\n\n";
    
    double improvement = ((ga.bestFitness() - defaultFitness) / defaultFitness) * 100.0;
    std::cout << "Improvement: " << improvement << "%\n\n";
    
    // Display optimized chromosome
    const auto& best = ga.bestChromosome();
    std::cout << "Optimized membership function parameters:\n";
    std::cout << "  SpeedChange (13 params): ";
    for (size_t i = 0; i < 13; ++i) std::cout << best.genes[i] << " ";
    std::cout << "\n";
    
    std::cout << "  HeadingChange (13 params): ";
    for (size_t i = 13; i < 26; ++i) std::cout << best.genes[i] << " ";
    std::cout << "\n";
    
    std::cout << "  VerticalRateChange (13 params): ";
    for (size_t i = 26; i < 39; ++i) std::cout << best.genes[i] << " ";
    std::cout << "\n";
    
    std::cout << "  AltitudeChange (13 params): ";
    for (size_t i = 39; i < 52; ++i) std::cout << best.genes[i] << " ";
    std::cout << "\n";
    
    std::cout << "  TimeGap (7 params): ";
    for (size_t i = 52; i < 59; ++i) std::cout << best.genes[i] << " ";
    std::cout << "\n";
    
    std::cout << "  AnomalyLevel (7 params): ";
    for (size_t i = 59; i < 66; ++i) std::cout << best.genes[i] << " ";
    std::cout << "\n\n";
    
    // Success criteria
    std::cout << "========================================\n";
    std::cout << "TEST EVALUATION\n";
    std::cout << "========================================\n\n";
    
    bool testPassed = true;
    
    if (ga.bestFitness() > defaultFitness) {
        std::cout << "GA improved over default parameters\n";
    } else {
        std::cout << "GA did not improve over default\n";
        testPassed = false;
    }
    
    if (improvement > 5.0) {
        std::cout << "Significant improvement achieved (> 5%)\n";
    } else if (improvement > 0.0) {
        std::cout << "Marginal improvement (" << improvement << "%)\n";
    } else {
        std::cout << "No improvement\n";
        testPassed = false;
    }
    
    if (ga.bestFitness() > 0.7) {
        std::cout << "Good absolute fitness (> 0.7)\n";
    } else if (ga.bestFitness() > 0.5) {
        std::cout << "~ Moderate fitness (" << ga.bestFitness() << ")\n";
    } else {
        std::cout << "Poor absolute fitness\n";
        testPassed = false;
    }
    
    std::cout << "\n";
    if (testPassed) {
        std::cout << "INTEGRATION TEST PASSED\n";
        return 0;
    } else {
        std::cout << "INTEGRATION TEST FAILED\n";
        std::cout << "\nTroubleshooting:\n";
        std::cout << "- Check that default parameters are reasonable\n";
        std::cout << "- Try increasing generations or population size\n";
        std::cout << "- Verify test data scenarios are diverse enough\n";
        return 1;
    }
}
