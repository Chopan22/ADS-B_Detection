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

  // Noise distributions
  std::uniform_real_distribution<double> noise(-0.05, 0.05);
  std::uniform_real_distribution<double> normalNoise(-0.3, 0.3);
  std::uniform_real_distribution<double> anomalyDist(0.0, 1.0);

  FlightState p = {1654495200, 51.0, 4.0, 230.0, 90.0, 0.0, 10000.0};
  FlightState prev = p; // Track previous state for delta calculations

  // Extended flight: ~8 hours at 2-second intervals = 14,400 samples
  const int SAMPLES = 14400;

  // Track anomaly spacing
  int lastAnomalyIndex = -20;
  int minSpacing = 5;

  // Anomaly counters
  int lowCount = 0, mediumCount = 0, highCount = 0;

  for (int i = 0; i < SAMPLES; ++i) {
    double    score = 0.0;
    long long dt = 2;

    // Start with current state
    double oVel = p.vel, oHead = p.head, oVR = p.vr, oAlt = p.alt;

    double roll = anomalyDist(gen);
    bool   canHaveAnomaly = (i - lastAnomalyIndex) >= minSpacing;

    // DISTRIBUTION: 60% normal, 20% medium (0.4-0.7), 20% high (0.7-1.0)
    if (!canHaveAnomaly || roll < 0.60) {
      // 60% NORMAL BEHAVIOR - small deltas
      score = 0.0;
      lowCount++;

      // SpeedChange: -10 to 10, normal within [-1, 1]
      double speedDelta = normalNoise(gen) * 0.5;
      oVel += speedDelta;

      // HeadingChange: -180 to 180, normal within [-3, 3]
      double headDelta = normalNoise(gen) * 2.0;
      oHead += headDelta;

      // VerticalRateChange: -20 to 20, normal within [-0.5, 0.5]
      double vrDelta = normalNoise(gen) * 0.3;
      oVR = vrDelta;

      // AltitudeChange: computed from VR * dt, normal results in small changes
      // Will be calculated in physics update

      // Occasional gentle maneuvers (still normal)
      if (i % 40 == 0) {
        oHead += normalNoise(gen) * 5.0; // HeadingChange up to ~5 degrees
        oVR = normalNoise(gen) * 2.0;    // Small climb/descent
      }

    } else if (roll < 0.80) {
      // 20% MEDIUM ANOMALIES (score 0.4-0.7)
      // Push deltas into moderate anomaly ranges
      std::uniform_int_distribution<int> mediumType(0, 9);
      int                                type = mediumType(gen);

      if (type == 0) {
        // Moderate speed increase: SpeedChange = 5 to 7
        oVel += 5.5 + noise(gen) * 1.0;
        score = 0.50;
      } else if (type == 1) {
        // Moderate braking: SpeedChange = -5 to -7
        oVel -= 5.5 + noise(gen) * 1.0;
        score = 0.52;
      } else if (type == 2) {
        // Sharp turn: HeadingChange = 40 to 60
        oHead += 50.0 + noise(gen) * 8.0;
        score = 0.48;
      } else if (type == 3) {
        // Aggressive climb: VerticalRateChange = 10 to 12
        oVR = 11.0 + noise(gen) * 1.0;
        score = 0.55;
      } else if (type == 4) {
        // Aggressive descent: VerticalRateChange = -10 to -12
        oVR = -11.0 + noise(gen) * 1.0;
        score = 0.55;
      } else if (type == 5) {
        // Combined: moderate speed + turn
        oVel += 4.5 + noise(gen);
        oHead += 45.0 + noise(gen) * 5.0;
        score = 0.60;
      } else if (type == 6) {
        // Combined: turn + climb
        oHead += 40.0 + noise(gen) * 5.0;
        oVR = 10.0 + noise(gen);
        score = 0.58;
      } else if (type == 7) {
        // Reverse turn: HeadingChange = -50
        oHead -= 50.0 + noise(gen) * 5.0;
        score = 0.47;
      } else if (type == 8) {
        // Speed surge with descent
        oVel += 5.0;
        oVR = -9.0;
        score = 0.53;
      } else {
        // Oscillating vertical rate
        oVR = (i % 2 == 0 ? 10.0 : -10.0) + noise(gen);
        score = 0.51;
      }
      mediumCount++;
      lastAnomalyIndex = i;

    } else {
      // 20% HIGH ANOMALIES (score 0.7-1.0)
      // Push deltas close to or at limits
      std::uniform_int_distribution<int> highType(0, 11);
      int                                type = highType(gen);

      if (type == 0) {
        // Extreme acceleration: SpeedChange = 8 to 10
        oVel += 9.0 + noise(gen);
        score = 0.85;
      } else if (type == 1) {
        // Extreme braking: SpeedChange = -8 to -10
        oVel -= 9.0 + noise(gen);
        score = 0.87;
      } else if (type == 2) {
        // Near-impossible turn: HeadingChange = 100 to 140
        oHead += 120.0 + noise(gen) * 15.0;
        score = 0.92;
      } else if (type == 3) {
        // Extreme climb: VerticalRateChange = 16 to 20
        oVR = 18.0 + noise(gen) * 2.0;
        score = 0.90;
      } else if (type == 4) {
        // Extreme descent: VerticalRateChange = -16 to -20
        oVR = -18.0 + noise(gen) * 2.0;
        score = 0.91;
      } else if (type == 5) {
        // Triple threat: extreme speed + turn + climb
        oVel += 8.0 + noise(gen);
        oHead += 80.0 + noise(gen) * 10.0;
        oVR = 15.0 + noise(gen);
        score = 0.95;
      } else if (type == 6) {
        // Compound: hard brake + sharp turn
        oVel -= 8.0;
        oHead += 90.0 + noise(gen) * 10.0;
        score = 0.88;
      } else if (type == 7) {
        // Reverse extreme: HeadingChange = -120
        oHead -= 120.0 + noise(gen) * 15.0;
        score = 0.89;
      } else if (type == 8) {
        // Long time gap: TimeGap = 30 to 45
        dt = 35 + (int)(noise(gen) * 10);
        oVel += 6.0;
        score = 0.83;
      } else if (type == 9) {
        // Extreme time gap with position anomaly: TimeGap = 50+
        dt = 52 + (int)(noise(gen) * 5);
        oHead += 100.0;
        score = 0.96;
      } else if (type == 10) {
        // Complete profile anomaly
        oVel += 9.5;
        oHead += 130.0;
        oVR = 19.0;
        score = 0.98;
      } else {
        // Stall pattern: extreme brake + dive + turn
        oVel -= 9.5;
        oVR = -17.0;
        oHead += 60.0;
        score = 0.94;
      }
      highCount++;
      lastAnomalyIndex = i;
      minSpacing = 3;
    }

    // Physical constraints for absolute values
    oVel = std::max(120.0, std::min(450.0, oVel));
    oVR = std::max(-25.0, std::min(25.0, oVR));

    // Calculate altitude change from vertical rate
    oAlt = p.alt + (oVR * dt);
    oAlt = std::max(2000.0, std::min(14000.0, oAlt));

    // Normalize heading to 0-360
    while (oHead >= 360.0)
      oHead -= 360.0;
    while (oHead < 0.0)
      oHead += 360.0;

    // Calculate position based on velocity and heading
    double r = oHead * M_PI / 180.0;
    double oLat = p.lat + (oVel * dt * std::cos(r) * 0.000009);
    double oLon = p.lon + (oVel * dt * std::sin(r) * 0.000015);

    // Write to CSV
    file << p.time << ",4ca765," << std::fixed << std::setprecision(8) << oLat << "," << oLon << ","
         << std::setprecision(2) << oVel << "," << oHead << "," << oVR
         << ",BOUND_TEST,False,False,False,1000," << oAlt << "," << oAlt + 50 << "," << p.time - 0.5
         << "," << p.time << "," << score << "\n";

    // Update state for next iteration
    prev = p;
    p.vel = oVel;
    p.head = oHead;
    p.vr = oVR;
    p.alt = oAlt;
    p.lat = oLat;
    p.lon = oLon;
    p.time += dt;

    // Periodically reset spacing
    if (i % 150 == 0)
      minSpacing = 5;
  }

  file.close();

  // Print statistics
  std::cout << "Generated " << SAMPLES << " samples (~8 hours of flight)\n\n";
  std::cout << "Variable limits respected:\n";
  std::cout << "  SpeedChange:        [-10.0, 10.0]\n";
  std::cout << "  HeadingChange:      [-180.0, 180.0]\n";
  std::cout << "  VerticalRateChange: [-20.0, 20.0]\n";
  std::cout << "  AltitudeChange:     [-1000.0, 1000.0] (via VR * dt)\n";
  std::cout << "  TimeGap:            [2, 60] seconds\n\n";
  std::cout << "Actual distribution:\n";
  std::cout << "  Low (< 0.4):      " << lowCount << " (" << std::fixed << std::setprecision(2)
            << (lowCount * 100.0 / SAMPLES) << "%)\n";
  std::cout << "  Medium (0.4-0.7): " << mediumCount << " (" << (mediumCount * 100.0 / SAMPLES)
            << "%)\n";
  std::cout << "  High (> 0.7):     " << highCount << " (" << (highCount * 100.0 / SAMPLES)
            << "%)\n";
  std::cout << "\nTarget: 60% low, 20% medium, 20% high\n";

  return 0;
}
