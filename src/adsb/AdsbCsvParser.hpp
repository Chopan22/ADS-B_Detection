#pragma once 

#include "AdsbState.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cmath>

class AdsbCsvParser {
  public: 
    static std::vector<AdsbState> load(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open ADS-B CSV file");
        }

        std::vector<AdsbState> data;
        std::string line;

        std::getline(file, line);

        while (std::getline(file, line)) {
            AdsbState s;
            if (parseLine(line, s)) {
                data.push_back(s);
            }
        }

        return data;
    }

  private:
    static bool parseLine(const std::string& line, AdsbState& s) {
        std::stringstream ss(line);
        std::string token;

        // --- Helper lambdas ---
        auto getString = [&](std::string& out) {
            std::getline(ss, out, ',');
        };

        auto getDouble = [&](double& out) {
            std::getline(ss, token, ',');
            out = token.empty() ? NAN : std::stod(token);
        };

        auto getLong = [&](long long& out) {
            std::getline(ss, token, ',');
            out = token.empty() ? -1 : std::stoll(token);
        };

        auto getBool = [&](bool& out) {
            std::getline(ss, token, ',');
            out = (token == "true" || token == "1");
        };

        getLong(s.time);
        getString(s.icao24);
        getDouble(s.lat);
        getDouble(s.lon);
        getDouble(s.velocity);
        getDouble(s.heading);
        getDouble(s.vert_rate);
        getString(s.callsign);
        getBool(s.onground);
        getBool(s.alert);
        getBool(s.spi);
        getString(s.squawk);
        getDouble(s.baro_altitude);
        getDouble(s.geo_altitude);
        getLong(s.last_pos_update);
        getLong(s.last_contact);

        if (s.icao24.empty()) return false;
        if (s.onground) return false;
        if (std::isnan(s.lat) || std::isnan(s.lon)) return false;

        return true;
    }
};
