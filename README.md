# ADS-B Anomaly Detection using Fuzzy Logic and Genetic Algorithms

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.15+
- Python 3.7+ (for analysis)
- Python packages: pandas, matplotlib, seaborn, scipy, scikit-learn

### Build

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build .

# Optional: Build with tests
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
cmake --build .
```

### Install Python Dependencies

```bash
pip3 install pandas matplotlib seaborn scipy scikit-learn
```

## Usage

### 1. Train the System

Run the optimizer to train fuzzy parameters:

```bash
./optimizer data/your_flight_data.csv \
    --generations 100 \
    --population 200 \
    --train-split 0.8 \
    --output results/optimized_params.txt
```

Training process:
1. **Data Preprocessing**: Load CSV, extract features, apply expert rules
2. **Train/Val Split**: Divide data (default 80/20)
3. **Baseline Evaluation**: Test default parameters
4. **GA Optimization**: Evolve better parameters
5. **Validation**: Test on held-out data
6. **Save Results**: Export optimized parameters and predictions

### 3. Analyze Results

Generate visualization and metrics:

```bash
python3 tools/analyze_results.py
```

This creates:
- `results/graphs/prediction_scatter.png` - Predicted vs Expected
- `results/graphs/error_analysis.png` - Error distributions
- `results/graphs/residuals.png` - Residual analysis
- `results/graphs/feature_importance.png` - Feature correlations
- `results/graphs/classification_performance.png` - ROC curve & confusion matrix
- `results/graphs/feature_distributions.png` - Feature histograms
- `results/analysis_summary.txt` - Text report

## Input Data Format

ADS-B CSV file must have these columns:
```
time,icao24,lat,lon,velocity,heading,vert_rate,callsign,onground,alert,spi,squawk,baro_altitude,geo_altitude,last_pos_update,last_contact
```

Example:
```csv
time,icao24,lat,lon,velocity,heading,vert_rate,callsign,onground,alert,spi,squawk,baro_altitude,geo_altitude,last_pos_update,last_contact
1609459200,a12b34,40.7128,-74.0060,250.5,90.0,5.2,AAL123,false,false,false,1200,10000,9500,1609459200,1609459200
```

## Results

### Key Metrics

**Regression Metrics:**
- **MSE** (Mean Squared Error): Average squared prediction error (lower is better)
- **RMSE** (Root MSE): Square root of MSE, in same units as output
- **MAE** (Mean Absolute Error): Average absolute error
- **R²**: Proportion of variance explained (0 to 1, higher is better)

**Classification Metrics** (for anomaly detection):
- **Accuracy**: Overall correctness
- **Precision**: When predicting anomaly, how often correct?
- **Recall**: What fraction of actual anomalies detected?
- **F1 Score**: Harmonic mean of precision and recall

### Good Performance Indicators

  **Excellent:**
- Validation F1 > 0.85
- R² > 0.80
- Train F1 ≈ Val F1 (within 0.1)

  **Good:**
- Validation F1 > 0.75
- R² > 0.70
- Improvement over baseline > 15%

  **Warning Signs:**
- Train F1 >> Val F1 (gap > 0.15) → Overfitting
- Val F1 < 0.60 → Poor generalization
- Negative improvement → Optimization failed

## Customization

### Adjust GA Parameters

Edit optimizer call:
```bash
./optimizer data.csv \
    --generations 200    # More thorough but slower
    --population 300     # Larger search space
    --train-split 0.7    # More validation data
```

### Change Fuzzy Variables

Edit `src/fuzzy/AdsbFuzzyVariable.hpp` to adjust membership function shapes.

## Troubleshooting

### "No valid samples after preprocessing"
- Check CSV format matches expected schema
- Reduce filtering thresholds in `AdsbDataPreprocessor`
- Ensure data contains in-flight records (not on ground)

### Poor validation performance
- Collect more diverse data
- Increase generations: `--generations 200`
- Try different train/val split: `--train-split 0.7`

### Overfitting (Train >> Val performance)
- Reduce model complexity
- Get more training data
- Decrease generations

### Slow training
- Reduce population size: `--population 50`
- Reduce generations: `--generations 50`
- Use smaller dataset for initial tests

## Documentation

- **[TRAINING.md](docs/TRAINING.md)** - Detailed training guide
- **[ANALYSIS.md](docs/ANALYSIS.md)** - Result analysis and interpretation
- **[ARCHITECTURE.md](docs/ARCHITECTURE.md)** - System design and implementation

