
#include "../src/fuzzy/AdsbFuzzyVariable.hpp"
#include "../src/fuzzy/FuzzyInferenceSystem.hpp"
#include "../src/fuzzy/RuleBase.hpp"

#include <iostream>
#include <map>

using namespace fuzzy;

void runTest(const std::string& name, FuzzyInferenceSystem& fis,
             const std::map<std::string, double>& input) {
  double output = fis.evaluate(input);

  std::cout << "Test: " << name << "\n";
  for (const auto& [k, v] : input)
    std::cout << "  " << k << " = " << v << "\n";

  std::cout << "  → AnomalyScore = " << output << "\n\n";
}

int main() {
  FuzzyInferenceSystem fis;

  fis.addInputVariable(createSpeedChangeVariable());
  fis.addInputVariable(createHeadingChangeVariable());
  fis.addInputVariable(createVerticalRateChangeVariable());
  fis.addInputVariable(createAltitudeChangeVariable());
  fis.addInputVariable(createTimeGapVariable());

  fis.setOutputVariable(createAnomalyLevelVariable());

  for (const auto& rule : createAdsbRuleBase())
    fis.addRule(rule);

  runTest("Nominal stable flight", fis,
          {{"SpeedChange", 0.0},
           {"HeadingChange", 0.0},
           {"VerticalRateChange", 0.0},
           {"AltitudeChange", 0.0},
           {"TimeGap", 1.0}});

  runTest("Exact zero boundaries", fis,
          {{"SpeedChange", 0.0},
           {"HeadingChange", 0.0},
           {"VerticalRateChange", 0.0},
           {"AltitudeChange", 0.0},
           {"TimeGap", 0.0}});

  runTest("Upper in-range speed boundary", fis,
          {{"SpeedChange", 10.0},
           {"HeadingChange", 0.0},
           {"VerticalRateChange", 0.0},
           {"AltitudeChange", 0.0},
           {"TimeGap", 1.0}});

  runTest("Lower in-range vertical rate boundary", fis,
          {{"SpeedChange", 0.0},
           {"HeadingChange", 0.0},
           {"VerticalRateChange", -20.0},
           {"AltitudeChange", 0.0},
           {"TimeGap", 1.0}});

  runTest("Isolated speed anomaly (in range)", fis,
          {{"SpeedChange", 8.0},
           {"HeadingChange", 0.0},
           {"VerticalRateChange", 0.0},
           {"AltitudeChange", 0.0},
           {"TimeGap", 1.0}});

  runTest("Isolated heading anomaly", fis,
          {{"SpeedChange", 0.0},
           {"HeadingChange", 60.0},
           {"VerticalRateChange", 0.0},
           {"AltitudeChange", 0.0},
           {"TimeGap", 1.0}});

  runTest("Isolated vertical rate anomaly", fis,
          {{"SpeedChange", 0.0},
           {"HeadingChange", 0.0},
           {"VerticalRateChange", 15.0},
           {"AltitudeChange", 0.0},
           {"TimeGap", 1.0}});

  runTest("Coupled speed + vertical rate", fis,
          {{"SpeedChange", 6.0},
           {"HeadingChange", 0.0},
           {"VerticalRateChange", 10.0},
           {"AltitudeChange", 0.0},
           {"TimeGap", 1.0}});

  runTest("Kinematic maneuver (turn + climb)", fis,
          {{"SpeedChange", 3.0},
           {"HeadingChange", -45.0},
           {"VerticalRateChange", 12.0},
           {"AltitudeChange", 200.0},
           {"TimeGap", 1.0}});

  runTest("Moderate data gap", fis,
          {{"SpeedChange", 0.0},
           {"HeadingChange", 0.0},
           {"VerticalRateChange", 0.0},
           {"AltitudeChange", 0.0},
           {"TimeGap", 8.0}});

  runTest("Large data gap", fis,
          {{"SpeedChange", 0.0},
           {"HeadingChange", 0.0},
           {"VerticalRateChange", 0.0},
           {"AltitudeChange", 0.0},
           {"TimeGap", 30.0}});

  runTest("Extreme speed jump (out of range)", fis,
          {{"SpeedChange", 40.0},
           {"HeadingChange", 0.0},
           {"VerticalRateChange", 0.0},
           {"AltitudeChange", 0.0},
           {"TimeGap", 1.0}});

  runTest("Extreme vertical rate (out of range)", fis,
          {{"SpeedChange", 0.0},
           {"HeadingChange", 0.0},
           {"VerticalRateChange", 50.0},
           {"AltitudeChange", 0.0},
           {"TimeGap", 1.0}});

  runTest("Fully out-of-model observation", fis,
          {{"SpeedChange", 50.0},
           {"HeadingChange", 120.0},
           {"VerticalRateChange", 40.0},
           {"AltitudeChange", 2000.0},
           {"TimeGap", 90.0}});

  return 0;
}
