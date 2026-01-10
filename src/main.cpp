#include "adsb/AdsbCsvParser.hpp"
#include "feature/FeatureExtractor.hpp"

#include <iostream>
#include <iomanip>

int main() {
    auto states = AdsbCsvParser::load("data/test_simple.csv");

    std::cout << "States loaded: " << states.size() << "\n";

    for (const auto& s : states) {
        std::cout << s.time << " "
                  << s.lat << " "
                  << s.lon << " "
                  << s.velocity << "\n";
    }

    auto features = FeatureExtractor::extract(states);

    std::cout << "\nExtracted features: " << features.size() << "\n";

    for (const auto& f : features) {
        std::cout << std::fixed << std::setprecision(3)
                  << "dt=" << f.dt
                  << " ds=" << f.d_speed
                  << " acc=" << f.acceleration
                  << " dh=" << f.d_heading
                  << " dz=" << f.d_altitude
                  << " dist=" << f.ground_distance
                  << "\n";
    }

    return 0;
}

