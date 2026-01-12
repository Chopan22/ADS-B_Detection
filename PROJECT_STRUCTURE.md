# Project Structure

## Complete Directory Tree

```
adsb-fuzzy-ga/
│
├── CMakeLists.txt                       # Build configuration
├── README.md                            # Main documentation
│
├── src/                                 # Source code
│   ├── optimizer.cpp                    # Main training program
│   │
│   ├── ga/                              # Genetic Algorithm
│   │   ├── Chromosome.hpp/cpp           # Gene representation & operations
│   │   ├── Population.hpp/cpp           # Population management
│   │   ├── Selection.hpp/cpp            # Tournament selection
│   │   ├── Fitness.hpp/cpp              # Fitness evaluation
│   │   ├── GAEngine.hpp/cpp             # Main GA engine
│   │   └── ga_config.hpp                # GA configuration
│   │
│   ├── fuzzy/                           # Fuzzy Logic System
│   │   ├── FuzzyInferenceSystem.hpp     # Mamdani inference engine
│   │   ├── FuzzyVariable.hpp            # Variable definitions
│   │   ├── MembershipFunction.hpp       # Membership functions (tri, trap, s, z)
│   │   ├── FuzzyRule.hpp                # Rule representation
│   │   ├── AdsbFuzzyVariable.hpp        # ADS-B specific variables
│   │   └── RuleBase.hpp                 # Expert rule base
│   │
│   ├── adsb/                            # ADS-B Data Handling
│   │   ├── AdsbState.hpp                # Aircraft state structure
│   │   └── AdsbCsvParser.hpp            # CSV file parser
│   │
│   ├── features/                        # Feature Engineering
│   │   ├── FeatureVector.hpp            # Feature representation
│   │   └── FeatureExtractor.hpp         # Delta computations, haversine
│   │
│   ├── preprocessing/                   # Data Preprocessing
│   │   └── AdsbDataPreprocessor.hpp     # Pipeline: load → filter → label
│   │
│   └── analysis/                        # Result Analysis
│       └── Analysis.hpp                # Metrics computation & validation
│
├── tools/                               # External Tools
│   └── analyze_results.py               # Python visualization script
│
├── test/                                # Unit & Integration Tests
│   ├── ga_unit_test.cpp                 # GA component tests
│   └── fuzzy_ga_int_test.cpp            # Full system integration test
│
├── data/                                # Data Directory (user-provided)
│   └── README.txt                       # Instructions for placing CSV files
│
└── results/                             # Generated Outputs
    ├── optimized_params.txt             # Trained model parameters
    ├── predictions.csv                  # Validation predictions
    ├── error_analysis.txt               # Error breakdown
    ├── metrics_summary.txt              # Performance summary
    └── graphs/                          # Visualization outputs
        ├── prediction_scatter.png
        ├── error_analysis.png
        ├── residuals.png
        ├── feature_importance.png
        ├── classification_performance.png
        ├── feature_distributions.png
        └── analysis_summary.txt
```

## Module Dependencies

```
optimizer.cpp
    ├─→ preprocessing/AdsbDataPreprocessor
    │   ├─→ adsb/AdsbCsvParser
    │   │   └─→ adsb/AdsbState
    │   └─→ features/FeatureExtractor
    │       └─→ features/FeatureVector
    │
    ├─→ ga/GAEngine
    │   ├─→ ga/Population
    │   │   ├─→ ga/Chromosome
    │   │   └─→ ga/Fitness
    │   │       └─→ fuzzy/FuzzyInferenceSystem
    │   └─→ ga/Selection
    │
    └─→ analysis/Analysis
        └─→ fuzzy/FuzzyInferenceSystem
            ├─→ fuzzy/FuzzyVariable
            │   └─→ fuzzy/MembershipFunction
            ├─→ fuzzy/FuzzyRule
            ├─→ fuzzy/AdsbFuzzyVariable
            └─→ fuzzy/RuleBase
```

## Data Flow

```
┌─────────────┐
│  ADS-B CSV  │  (time, icao24, lat, lon, velocity, heading, ...)
└──────┬──────┘
       │
       ▼
┌─────────────────────┐
│  AdsbCsvParser      │  Parse & filter
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│  AdsbState[]        │  Structured data
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│  FeatureExtractor   │  Compute deltas
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│  FeatureVector[]    │  (d_speed, d_heading, dt, ...)
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│  AdsbDataPreproc    │  Normalize & label
└──────┬──────────────┘
       │
       ▼
┌─────────────────────────────────┐
│  inputs[], outputs[]            │  Training data
└──────┬──────────────────────────┘
       │
       ├─────────────┬────────────┐
       │             │            │
       ▼             ▼            ▼
┌───────────┐  ┌──────────┐  ┌──────────┐
│ Train 80% │  │ Val 20%  │  │ Baseline │
└─────┬─────┘  └────┬─────┘  └────┬─────┘
      │             │             │
      ▼             │             ▼
┌───────────┐       │        ┌──────────┐
│ GA Engine │       │        │  Fuzzy   │
│  - Init   │       │        │  Default │
│  - Select │       │        └────┬─────┘
│  - Cross  │       │             │
│  - Mutate │       │             ▼
│  - Eval   │       │        ┌──────────┐
└─────┬─────┘       │        │Baseline  │
      │             │        │ Metrics  │
      ▼             │        └──────────┘
┌───────────┐       │
│Optimized  │       │
│Chromosome │       │
└─────┬─────┘       │
      │             │
      ├─────────────┤
      │             │
      ▼             ▼
┌─────────────────────┐
│   Fuzzy System      │
│   (optimized)       │
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│  Predictions[]      │
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│  Analysis           │  Compute metrics
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│  Analysis Results   │  CSV, TXT files
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│  analyze_results.py │  Generate graphs
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│  Visualization      │  PNG files
└─────────────────────┘
```

## Build Artifacts

After building:

```
build/
├── optimizer                  # Main executable
├── validator                  # Data validator tool
├── ga_unit_test              # (if BUILD_TESTS=ON)
├── fuzzy_ga_integration_test # (if BUILD_TESTS=ON)
│
├── CMakeFiles/               # CMake internals
├── CMakeCache.txt
├── Makefile
│
└── libga.a                   # Static GA library
```

## Key File Descriptions

### Core System Files

**src/optimizer.cpp**
- Main entry point
- Command-line argument parsing
- Orchestrates entire training pipeline
- Saves optimized parameters

**src/ga/GAEngine.cpp**
- GA main loop
- Population initialization
- Evolution cycle management
- Best solution tracking

**src/fuzzy/FuzzyInferenceSystem.hpp**
- Mamdani inference implementation
- Fuzzification
- Rule evaluation (min aggregation)
- Defuzzification (centroid method)

**src/preprocessing/AdsbDataPreprocessor.hpp**
- CSV loading via AdsbCsvParser
- Feature extraction
- Outlier filtering
- Expert rule labeling
- Statistics computation

**src/analysis/Analysis.hpp**
- Fuzzy system evaluation
- Metrics calculation (MSE, F1, etc.)
- Prediction export
- Error analysis

### Data Handling

**src/adsb/AdsbCsvParser.hpp**
- CSV parsing with validation
- Filters ground aircraft
- Handles missing values
- Returns AdsbState vector

**src/features/FeatureExtractor.hpp**
- Computes feature deltas
- Heading normalization (-180 to 180)
- Haversine distance calculation
- Temporal gap computation

### GA Components

**src/ga/Chromosome.cpp**
- 66-gene representation
- Dynamic bounds based on fuzzy variable constraints
- Repair mechanism for validity
- Variable-level crossover
- Bounded mutation

**src/ga/Population.cpp**
- Population management
- Tournament selection
- Evolution operations
- Fitness evaluation

**src/ga/Fitness.cpp**
- Builds fuzzy system from chromosome
- Evaluates on training data
- Computes MSE-based fitness

### Fuzzy System

**src/fuzzy/AdsbFuzzyVariable.hpp**
- Parameterized variable creators
- Default configurations
- 5 fuzzy variables (inputs)
- 1 output variable

**src/fuzzy/RuleBase.hpp**
- Expert rules for ADS-B anomaly detection
- ~13 rules covering:
  - Normal behavior
  - Strong anomalies
  - Time gap effects
  - Compound anomalies

### Analysis Tools

**tools/analyze_results.py**
- Loads predictions CSV
- Generates 6 visualization types
- Computes comprehensive metrics
- Exports text summary

## File Size Summary

```
Component          Files    Description
──────────────────────────────────────────
GA Module             6     Genetic algorithm
Fuzzy Module          6     Fuzzy inference
ADS-B Data            2     Data structures
Features              2     Feature extraction  
Preprocessing         1     Data pipeline
Analysis              1     Validation & metrics
Tools                 1     Data validator
Main Program          1     Orchestration
Python Analysis       1     Visualization
Tests                 2     Unit & integration
──────────────────────────────────────────
TOTAL                23   
```

## Compilation Units

When building, these are compiled separately:

1. **libga.a** (static library):
   - Chromosome.cpp
   - Selection.cpp
   - Fitness.cpp
   - Population.cpp
   - GAEngine.cpp

2. **optimizer** (executable):
   - optimizer.cpp
   - Links: libga.a

3. **validator** (executable):
   - tools/validator.cpp
   - No library dependencies

4. **Tests** (optional):
   - ga_unit_test.cpp → Links libga.a
   - fuzzy_ga_int_test.cpp → Links libga.a

## Memory Layout

### Chromosome Structure
```
Chromosome {
    genes[66]:        66 × 8 bytes = 528 bytes
    bounds[66]:       66 × 16 bytes = 1056 bytes
    ──────────────────────────────────────────
    Total per individual: ~1.6 KB
}
```

### Population Memory
```
Population (size N):
    chromosomes:      N × 1.6 KB
    fitnessValues:    N × 8 bytes
    ──────────────────────────────────────────
    N=100:  ~160 KB
    N=200:  ~320 KB
    N=500:  ~800 KB
```

### Training Data
```
Dataset (M samples):
    inputs:  M × 5 × 8 bytes = 40M bytes
    outputs: M × 8 bytes = 8M bytes
    ──────────────────────────────────────────
    M=1000:   ~48 KB
    M=5000:   ~240 KB
    M=10000:  ~480 KB
```

## Configuration Files

1. **CMakeLists.txt**
   - Build system configuration
   - Compiler flags
   - Target definitions

2. **ga_config.hpp**
   - GA compilation mode toggle
   - Total gene count constant

3. **Command-line args** (optimizer)
   - No config files needed
   - All parameters via CLI

