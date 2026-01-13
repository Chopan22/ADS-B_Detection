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

  std::mt19937 gen(888);
  // Small noise to ensure data isn't "perfectly" constant
  std::uniform_real_distribution<double> noise(-0.1, 0.1);

  FlightState p = {1654495200, 51.0, 4.0, 230.0, 90.0, 0.0, 10000.0};
  const int   SAMPLES = 9000;

  for (int i = 0; i < SAMPLES; ++i) {
    double    score = 0.0;
    long long dt = 2; // High frequency base

    double oLat = p.lat, oLon = p.lon, oVel = p.vel, oAlt = p.alt, oVR = p.vr;

    // --- CYCLIC ANOMALY INJECTION BASED ON YOUR BOUNDS ---

    // 1. Every 120 samples: Rule 1 - Extreme Altitude/Speed (1.0)
    if (i % 120 == 0) {
      oAlt += 950.0; // Near the 1000.0 limit
      oVel -= 9.5;   // Near the -10.0 limit (Hard Brake)
      score = 1.0;
    }
    // 2. Every 180 samples: Rule 2 - Impossible Rotation (0.9)
    else if (i % 180 == 0) {
      p.head += 110.0; // Exceeds the 90.0 logic threshold
      score = 0.9;
    }
    // 3. Every 80 samples: Rule 3 - Compound Aggressive (0.8)
    else if (i % 80 == 0) {
      oVel += 6.0;    // > 5.0
      p.head += 50.0; // > 45.0
      score = 0.8;
    }
    // 4. Every 50 samples: Rule 4 - Performance Edge (0.5)
    else if (i % 50 == 0) {
      oVR = 12.0; // > 8.0
      score = 0.5;
    }
    // 5. Every 30 samples: Rule 5 - Normal Maneuvering (0.2)
    else if (i % 30 == 0) {
      p.head += 12.0; // > 10.0
      score = 0.2;
    }
    // 6. Every 400 samples: Rule 6 - Long Gap Silence
    else if (i % 400 == 0) {
      dt = 45; // Exceeds the 30s logic threshold
      score = 0.1;
    }

    // Write to CSV
    file << p.time << ",4ca765," << std::fixed << std::setprecision(8) << oLat << "," << oLon << ","
         << std::setprecision(2) << oVel << "," << p.head << "," << oVR
         << ",BOUND_TEST,False,False,False,1000," << oAlt << "," << oAlt + 50 << "," << p.time - 0.5
         << "," << p.time << "," << score << "\n";

    // Physics Update
    p.vel += noise(gen); // Add jitter
    p.alt += p.vr * dt;
    double r = p.head * M_PI / 180.0;
    p.lat += (p.vel * dt * std::cos(r) * 0.000009);
    p.lon += (p.vel * dt * std::sin(r) * 0.000015);
    p.time += dt;

    // Normalize heading to 0-360
    if (p.head >= 360.0)
      p.head -= 360.0;
    if (p.head < 0.0)
      p.head += 360.0;
  }
  return 0;
}
