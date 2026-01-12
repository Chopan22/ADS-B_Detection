#pragma once

#include "../adsb/AdsbState.hpp"
#include "FeatureVector.hpp"

#include <vector>
#include <cmath>
#include <algorithm>

class FeatureExtractor {
public:
    static std::vector<FeatureVector>
    extract(const std::vector<AdsbState>& states) {
        if (states.size() < 2) {
            return {};
        }

        std::vector<FeatureVector> features;
        features.reserve(states.size() - 1);

        for (size_t i = 1; i < states.size(); ++i) {
            const auto& prev = states[i - 1];
            const auto& curr = states[i];

            double dt = static_cast<double>(curr.time - prev.time);
            if (dt <= 0.0) continue;

            FeatureVector fv;
            fv.dt = dt;
            fv.d_speed = curr.velocity - prev.velocity;
            fv.d_heading = headingDelta(prev.heading, curr.heading);
            fv.d_vert_rate = curr.vert_rate - prev.vert_rate;
            fv.d_altitude = curr.baro_altitude - prev.baro_altitude;
            fv.ground_distance = haversine(
                prev.lat, prev.lon,
                curr.lat, curr.lon
            );
            fv.acceleration = fv.d_speed / dt;

            fv.target_score = curr.target_score;
            features.push_back(fv);
        }

        return features;
    }

private:
    static double headingDelta(double h1, double h2) {
        double delta = h2 - h1;
        while (delta > 180.0) delta -= 360.0;
        while (delta < -180.0) delta += 360.0;
        return delta;
    }

    static double haversine(
        double lat1, double lon1,
        double lat2, double lon2
    ) {
        constexpr double R = 6371000.0; // Earth radius [m]

        double phi1 = deg2rad(lat1);
        double phi2 = deg2rad(lat2);
        double dphi = deg2rad(lat2 - lat1);
        double dlambda = deg2rad(lon2 - lon1);

        double a = std::sin(dphi / 2) * std::sin(dphi / 2) +
                   std::cos(phi1) * std::cos(phi2) *
                   std::sin(dlambda / 2) * std::sin(dlambda / 2);

        double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
        return R * c;
    }

    static double deg2rad(double deg) {
        return deg * M_PI / 180.0;
    }
};

