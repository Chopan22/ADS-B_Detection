#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>

struct FlightState {
  long long time;
  double    lat, lon, vel, head, vr, alt;
};

int main() {
  std::ofstream file("generated_data.csv");
  file << "time,icao24,lat,lon,velocity,heading,vertrate,callsign,onground,alert,spi,squawk,"
          "baroaltitude,geoaltitude,lastposupdate,lastcontact,target_score\n";

  std::mt19937                           gen(1337);
  std::uniform_real_distribution<double> jitter(-0.02, 0.02); // Tiny "real-world" noise

  FlightState p = {1654495200, 51.0, 4.0, 230.0, 90.0, 0.0, 10000.0};
  const int   SAMPLES = 3600; // 2 hours at ~2s

  for (int i = 0; i < SAMPLES; ++i) {
    double    score = 0.0;
    long long dt = 2;

    // Apply natural drift so it's never a constant line
    p.vel += jitter(gen);
    p.vr += jitter(gen);
    p.head += (jitter(gen) * 0.5);

    double oLat = p.lat, oLon = p.lon, oVel = p.vel, oAlt = p.alt, oVR = p.vr;

    // --- THE EXPERT SYSTEM SCENARIOS ---

    // SCENARIO 1: Physics Break (1.0) - The "Teleport"
    if (i == 500) {
      oLat += 0.1; // Impossible jump for 2 seconds
      score = 1.0;
    }

    // SCENARIO 2: Edge of Normal (0.5) - The "Hard Maneuver"
    // A sudden jump in vertical rate to 35 m/s (~7000 fpm)
    // Possible for a fighter jet, but "maybe anomaly" for a commercial flight.
    else if (i == 1000) {
      oVR = 35.0;
      score = 0.5;
    }

    // SCENARIO 3: Time Gap + Physics Break (1.0)
    else if (i == 1500) {
      dt = 60;         // 1 minute gap
      oAlt += 15000.0; // Climbed 15km in 60s (250 m/s climb - Impossible)
      score = 1.0;
    }

    // SCENARIO 4: Time Gap + Edge Case (0.5)
    else if (i == 2000) {
      dt = 60;
      oVel += 40.0; // Speeding up by 80 knots in a minute (Aggressive but possible)
      score = 0.5;
    }

    // SCENARIO 5: Kinematic Incoherence (1.0)
    // Velocity reports 230m/s, but position hasn't moved at all.
    else if (i >= 2500 && i <= 2505) {
      oVel = 230.0;
      // Lat/Lon will stay frozen in the CSV output
      score = 1.0;
    }

    // Write row
    file << p.time << ",4ca765," << std::fixed << std::setprecision(8) << oLat << "," << oLon << ","
         << std::setprecision(2) << oVel << "," << p.head << "," << oVR
         << ",TEST123,False,False,False,0100," << oAlt << "," << oAlt + 50 << "," << p.time - 0.5
         << "," << p.time << "," << score << "\n";

    // Update Physics (The "Expected" Path)
    p.alt += p.vr * dt;
    double r = p.head * M_PI / 180.0;
    if (!(i >= 2500 && i <= 2505)) { // Don't move physics for the freeze test
      p.lat += (p.vel * dt * std::cos(r) * 0.000009);
      p.lon += (p.vel * dt * std::sin(r) * 0.000015);
    }
    p.time += dt;
  }
  return 0;
}
