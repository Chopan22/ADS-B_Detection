#include "Selection.hpp"
#include <algorithm>
#include <stdexcept>

namespace ga {

Selection::Selection(size_t tournamentSize)
    : tournamentSize_(tournamentSize)
{
    if (tournamentSize_ < 2)
        throw std::invalid_argument("Tournament size must be >= 2");
}

Chromosome Selection::tournamentSelect(const std::vector<Chromosome>& population, const std::vector<double>& fitnesses, std::mt19937& rng)
{
    if (population.empty() || population.size() != fitnesses.size())
        throw std::runtime_error("Population and fitnesses must be same non-zero size");

    std::uniform_int_distribution<size_t> dist(0, population.size() - 1);
    size_t bestIdx = dist(rng);
    double bestFitness = fitnesses[bestIdx];

    for (size_t i = 1; i < tournamentSize_; ++i) {
        size_t idx = dist(rng);
        if (fitnesses[idx] > bestFitness) {
            bestFitness = fitnesses[idx];
            bestIdx = idx;
        }
    }
    return population[bestIdx];
}

std::vector<Chromosome> Selection::selectPopulation(const std::vector<Chromosome>& population, const std::vector<double>& fitnesses, size_t numSelected, std::mt19937& rng)
{
    std::vector<Chromosome> selected;
    selected.reserve(numSelected);

    for (size_t i = 0; i < numSelected; ++i) {
        selected.push_back(tournamentSelect(population, fitnesses, rng));
    }
    return selected;
}
}
