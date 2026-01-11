#include "Fitness.hpp"
#include "../fuzzy/AdsbFuzzyVariable.hpp"
#include "../fuzzy/RuleBase.hpp"
#include <stdexcept>
#include <cmath>

namespace ga {

Fitness::Fitness(const std::vector<std::map<std::string, double>>& inputs,
                 const std::vector<double>& expectedOutputs)
    : testInputs_(inputs), expectedOutputs_(expectedOutputs)
{
    if (inputs.size() != expectedOutputs.size())
        throw std::runtime_error("Inputs and expected outputs size mismatch");
}
#ifdef GA_TEST_MODE

double Fitness::evaluate(const Chromosome& c) {
    double sum = 0.0;
    for (double g : c.genes)
        sum += g;
    return sum;
}

#else

double Fitness::evaluate(const Chromosome& chromo)
{
    size_t idx = 0;
    auto nextGenes = [&](size_t count) -> std::vector<double> {
        std::vector<double> v(chromo.genes.begin() + idx, chromo.genes.begin() + idx + count);
        idx += count;
        return v;
    };

    auto speedVar   = fuzzy::createSpeedChangeVariable(nextGenes(13));
    auto headingVar = fuzzy::createHeadingChangeVariable(nextGenes(13));
    auto vertVar    = fuzzy::createVerticalRateChangeVariable(nextGenes(13));
    auto altVar     = fuzzy::createAltitudeChangeVariable(nextGenes(13));
    auto timeVar    = fuzzy::createTimeGapVariable(nextGenes(7));
    auto anomalyVar = fuzzy::createAnomalyLevelVariable(nextGenes(7));

    fuzzy::FuzzyInferenceSystem fis;
    fis.addInputVariable(speedVar);
    fis.addInputVariable(headingVar);
    fis.addInputVariable(vertVar);
    fis.addInputVariable(altVar);
    fis.addInputVariable(timeVar);
    fis.setOutputVariable(anomalyVar);

    for (const auto& rule : fuzzy::createAdsbRuleBase())
        fis.addRule(rule);

    double mse = 0.0;
    for (size_t i = 0; i < testInputs_.size(); ++i) {
        double out = fis.evaluate(testInputs_[i]);
        double err = out - expectedOutputs_[i];
        mse += err * err;
    }
    mse /= testInputs_.size();

    return 1.0 / (1.0 + mse);
}

#endif

}
