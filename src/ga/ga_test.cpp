#include "GAEngine.hpp"
#include "Fitness.hpp"
#include <iostream>
#include <vector>
#include <map>

int main() {
#ifdef GA_TEST_MODE
    std::cout << "Running GA in REALISTIC TEST MODE\n";
    std::cout << "Simulating fuzzy inference optimization without real ADSB data\n\n";
    
    std::vector<std::map<std::string, double>> testInputs;
    std::vector<double> expectedOutputs;
    
    for (int i = 0; i < 20; ++i) {
        std::map<std::string, double> input;
        
        input["sensor1"] = static_cast<double>(i) / 20.0;
        input["sensor2"] = static_cast<double>(i % 5) / 5.0;
        input["sensor3"] = static_cast<double>((i * 3) % 7) / 7.0;
        
        testInputs.push_back(input);
        
        expectedOutputs.push_back(0.6 + 0.1 * std::sin(i * 0.5));
    }
    
    std::cout << "Generated " << testInputs.size() << " synthetic test cases\n";
    std::cout << "Target: Optimize fuzzy membership functions for anomaly detection\n\n";
    
    ga::Fitness fitness(testInputs, expectedOutputs);
    
#else
    std::cout << "Running GA in PRODUCTION MODE\n";
    std::cout << "Optimizing real ADSB anomaly detection fuzzy system\n\n";
    
    std::vector<std::map<std::string, double>> testInputs;
    std::vector<double> expectedOutputs;
    
    if (testInputs.empty()) {
        std::cerr << "ERROR: No training data loaded!\n";
        std::cerr << "Please implement ADSB data loading before running in production mode.\n";
        return 1;
    }
    
    ga::Fitness fitness(testInputs, expectedOutputs);
#endif

    std::cout << "GA Parameters:\n";
    std::cout << "  Population Size: 100\n";
    std::cout << "  Generations: 50\n";
    std::cout << "  Crossover Probability: 0.8\n";
    std::cout << "  Mutation Probability: 0.2\n";
    std::cout << "  Tournament Size: 3\n\n";

    ga::GAEngine ga(
        100,   // population size
        50,    // generations
        0.8,   // crossover probability
        0.2,   // mutation probability
        3      // tournament size
    );
    
    ga.setFitnessEvaluator(&fitness);
    ga.run();

    std::cout << "\n========================================\n";
    std::cout << "OPTIMIZATION COMPLETE\n";
    std::cout << "========================================\n";
    std::cout << "Best fitness achieved: " << ga.bestFitness() << "\n";
    
    const auto& bestChrom = ga.bestChromosome();
    std::cout << "\nOptimized chromosome (membership function parameters):\n";
    for (size_t i = 0; i < bestChrom.genes.size(); ++i) {
        std::cout << "  Gene[" << i << "] = " << bestChrom.genes[i] << "\n";
    }
    
#ifdef GA_TEST_MODE
    std::cout << "\nTest mode complete. Ready to switch to production mode.\n";
    std::cout << "To use real data:\n";
    std::cout << "  1. Comment out '#define GA_TEST_MODE' in ga_config.hpp\n";
    std::cout << "  2. Implement ADSB data loading in this file\n";
    std::cout << "  3. Rebuild and run\n";
#else
    std::cout << "\nProduction optimization complete.\n";
    std::cout << "Apply these optimized parameters to your fuzzy inference system.\n";
#endif

    return 0;
}
