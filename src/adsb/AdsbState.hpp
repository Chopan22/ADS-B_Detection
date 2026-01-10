#pragma once
#include <string>

struct AdsbState {
    long long time;
    long long last_pos_update;
    long long last_contact;
    
    std::string icao24;
    std::string squawk;
    std::string callsign;

    double lat;
    double lon;
    double baro_altitude;
    double geo_altitude;

    double velocity;
    double heading;
    double vert_rate;

    bool onground;
    bool alert;
    bool spi;
};

