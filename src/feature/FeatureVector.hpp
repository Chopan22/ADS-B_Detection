#pragma once

struct FeatureVector {
  double dt;
  double d_speed;
  double d_heading;
  double d_vert_rate;
  double d_altitude;
  double ground_distance;
  double acceleration;
  double target_score;
};
