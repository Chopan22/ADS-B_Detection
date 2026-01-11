#include "GAEngine.hpp"
#include "Fitness.hpp"
#include <iostream>
#include <vector>
#include <map>

int main() {
#ifdef GA_TEST_MODE
    std::cout << "Running GA in TEST MODE\n";
    
    // Create simple test data
    std::vector<std::map<std::string, double>> testInputs;
    std::vector<double> expectedOutputs;
    
    // Add some dummy test cases for TEST MODE
    for (int i = 0; i < 5; ++i) {
        std::map<std::string, double> input;
        input["dummy"] = i * 1.0;
        testInputs.push_back(input);
        expectedOutputs.push_back(i * 2.0);
    }
    
    ga::Fitness fitness(testInputs, expectedOutputs);
    
#else
    std::cout << "Running GA in PRODUCTION MODE\n";
    
    // In production mode, you would load actual ADSB data here
    std::vector<std::map<std::string, double>> testInputs;
    std::vector<double> expectedOutputs;
    
    // TODO: Load real data
    
    ga::Fitness fitness(testInputs, expectedOutputs);
#endif

    ga::GAEngine ga;
    ga.setFitnessEvaluator(&fitness);
    ga.run();

    std::cout << "\nBest chromosome found with fitness: " << ga.bestFitness() << "\n";

    return 0;
}
