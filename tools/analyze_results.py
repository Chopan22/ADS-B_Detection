import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
from pathlib import Path
import sys

sns.set_style("whitegrid")
plt.rcParams['figure.figsize'] = (12, 8)

def load_predictions(filepath='results/predictions.csv'):
    """Load prediction results"""
    try:
        df = pd.read_csv(filepath)
        print(f"✓ Loaded {len(df)} predictions from {filepath}")
        return df
    except FileNotFoundError:
        print(f"✗ Error: {filepath} not found")
        print("  Make sure you've run the optimizer first")
        sys.exit(1)

def plot_prediction_scatter(df, output_dir='results/graphs'):
    """Scatter plot of expected vs predicted values"""
    plt.figure(figsize=(10, 10))
    
    plt.scatter(df['Expected'], df['Predicted'], alpha=0.5, s=20)
    
    # Add perfect prediction line
    min_val = min(df['Expected'].min(), df['Predicted'].min())
    max_val = max(df['Expected'].max(), df['Predicted'].max())
    plt.plot([min_val, max_val], [min_val, max_val], 'r--', 
             linewidth=2, label='Perfect Prediction')
    
    plt.xlabel('Expected Anomaly Level', fontsize=12)
    plt.ylabel('Predicted Anomaly Level', fontsize=12)
    plt.title('Predicted vs Expected Anomaly Levels', fontsize=14, fontweight='bold')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    
    filepath = Path(output_dir) / 'prediction_scatter.png'
    plt.savefig(filepath, dpi=300, bbox_inches='tight')
    print(f"✓ Saved: {filepath}")
    plt.close()

def plot_error_distribution(df, output_dir='results/graphs'):
    """Histogram of prediction errors"""
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    
    # Error distribution
    axes[0, 0].hist(df['Error'], bins=50, edgecolor='black', alpha=0.7)
    axes[0, 0].axvline(0, color='r', linestyle='--', linewidth=2)
    axes[0, 0].set_xlabel('Prediction Error')
    axes[0, 0].set_ylabel('Frequency')
    axes[0, 0].set_title('Error Distribution')
    axes[0, 0].grid(True, alpha=0.3)
    
    # Absolute error distribution
    axes[0, 1].hist(df['AbsError'], bins=50, edgecolor='black', alpha=0.7, color='orange')
    axes[0, 1].set_xlabel('Absolute Error')
    axes[0, 1].set_ylabel('Frequency')
    axes[0, 1].set_title('Absolute Error Distribution')
    axes[0, 1].grid(True, alpha=0.3)
    
    # Error vs Expected
    axes[1, 0].scatter(df['Expected'], df['AbsError'], alpha=0.5, s=20)
    axes[1, 0].set_xlabel('Expected Value')
    axes[1, 0].set_ylabel('Absolute Error')
    axes[1, 0].set_title('Error vs Expected Value')
    axes[1, 0].grid(True, alpha=0.3)
    
    # Error vs Predicted
    axes[1, 1].scatter(df['Predicted'], df['AbsError'], alpha=0.5, s=20, color='green')
    axes[1, 1].set_xlabel('Predicted Value')
    axes[1, 1].set_ylabel('Absolute Error')
    axes[1, 1].set_title('Error vs Predicted Value')
    axes[1, 1].grid(True, alpha=0.3)
    
    plt.tight_layout()
    filepath = Path(output_dir) / 'error_analysis.png'
    plt.savefig(filepath, dpi=300, bbox_inches='tight')
    print(f"✓ Saved: {filepath}")
    plt.close()

def plot_residuals(df, output_dir='results/graphs'):
    """Residual plots"""
    fig, axes = plt.subplots(1, 2, figsize=(16, 6))
    
    # Residuals over index (time)
    axes[0].scatter(df.index, df['Error'], alpha=0.5, s=20)
    axes[0].axhline(0, color='r', linestyle='--', linewidth=2)
    axes[0].set_xlabel('Sample Index')
    axes[0].set_ylabel('Residual (Error)')
    axes[0].set_title('Residuals Over Time')
    axes[0].grid(True, alpha=0.3)
    
    # Q-Q plot
    from scipy import stats
    stats.probplot(df['Error'], dist="norm", plot=axes[1])
    axes[1].set_title('Q-Q Plot (Normality Check)')
    axes[1].grid(True, alpha=0.3)
    
    plt.tight_layout()
    filepath = Path(output_dir) / 'residuals.png'
    plt.savefig(filepath, dpi=300, bbox_inches='tight')
    print(f"✓ Saved: {filepath}")
    plt.close()

def plot_feature_importance(df, output_dir='results/graphs'):
    """Correlation of features with error"""
    features = ['SpeedChange', 'HeadingChange', 'VerticalRateChange', 
                'AltitudeChange', 'TimeGap']
    
    correlations = []
    for feature in features:
        corr = df[feature].corr(df['AbsError'])
        correlations.append(corr)
    
    fig, ax = plt.subplots(figsize=(10, 6))
    colors = ['red' if c > 0 else 'blue' for c in correlations]
    bars = ax.barh(features, correlations, color=colors, alpha=0.7)
    ax.axvline(0, color='black', linewidth=1)
    ax.set_xlabel('Correlation with Absolute Error', fontsize=12)
    ax.set_title('Feature Correlation with Prediction Error', 
                 fontsize=14, fontweight='bold')
    ax.grid(True, alpha=0.3, axis='x')
    
    # Add value labels
    for i, (bar, corr) in enumerate(zip(bars, correlations)):
        ax.text(corr, i, f' {corr:.3f}', va='center', 
                ha='left' if corr > 0 else 'right')
    
    plt.tight_layout()
    filepath = Path(output_dir) / 'feature_importance.png'
    plt.savefig(filepath, dpi=300, bbox_inches='tight')
    print(f"✓ Saved: {filepath}")
    plt.close()

def plot_anomaly_classification(df, output_dir='results/graphs'):
    """Classification performance visualization"""
    threshold = 0.5
    
    df['ExpectedClass'] = (df['Expected'] > threshold).astype(int)
    df['PredictedClass'] = (df['Predicted'] > threshold).astype(int)
    
    # Confusion matrix
    from sklearn.metrics import confusion_matrix
    cm = confusion_matrix(df['ExpectedClass'], df['PredictedClass'])
    
    fig, axes = plt.subplots(1, 2, figsize=(14, 6))
    
    # Confusion matrix heatmap
    sns.heatmap(cm, annot=True, fmt='d', cmap='Blues', ax=axes[0],
                xticklabels=['Normal', 'Anomaly'],
                yticklabels=['Normal', 'Anomaly'])
    axes[0].set_xlabel('Predicted')
    axes[0].set_ylabel('Actual')
    axes[0].set_title('Confusion Matrix')
    
    # ROC-like curve (varying threshold)
    thresholds = np.linspace(0, 1, 100)
    tpr_list = []
    fpr_list = []
    
    for t in thresholds:
        pred_class = (df['Predicted'] > t).astype(int)
        exp_class = (df['Expected'] > threshold).astype(int)
        
        tp = ((pred_class == 1) & (exp_class == 1)).sum()
        fp = ((pred_class == 1) & (exp_class == 0)).sum()
        tn = ((pred_class == 0) & (exp_class == 0)).sum()
        fn = ((pred_class == 0) & (exp_class == 1)).sum()
        
        tpr = tp / (tp + fn) if (tp + fn) > 0 else 0
        fpr = fp / (fp + tn) if (fp + tn) > 0 else 0
        
        tpr_list.append(tpr)
        fpr_list.append(fpr)
    
    axes[1].plot(fpr_list, tpr_list, linewidth=2)
    axes[1].plot([0, 1], [0, 1], 'r--', linewidth=2, label='Random')
    axes[1].set_xlabel('False Positive Rate')
    axes[1].set_ylabel('True Positive Rate')
    axes[1].set_title('ROC Curve')
    axes[1].legend()
    axes[1].grid(True, alpha=0.3)
    
    plt.tight_layout()
    filepath = Path(output_dir) / 'classification_performance.png'
    plt.savefig(filepath, dpi=300, bbox_inches='tight')
    print(f"✓ Saved: {filepath}")
    plt.close()

def plot_feature_distributions(df, output_dir='results/graphs'):
    """Feature value distributions"""
    features = ['SpeedChange', 'HeadingChange', 'VerticalRateChange', 
                'AltitudeChange', 'TimeGap']
    
    fig, axes = plt.subplots(2, 3, figsize=(16, 10))
    axes = axes.flatten()
    
    for i, feature in enumerate(features):
        axes[i].hist(df[feature], bins=50, edgecolor='black', alpha=0.7)
        axes[i].set_xlabel(feature)
        axes[i].set_ylabel('Frequency')
        axes[i].set_title(f'{feature} Distribution')
        axes[i].grid(True, alpha=0.3)
    
    # Remove empty subplot
    fig.delaxes(axes[5])
    
    plt.tight_layout()
    filepath = Path(output_dir) / 'feature_distributions.png'
    plt.savefig(filepath, dpi=300, bbox_inches='tight')
    print(f"✓ Saved: {filepath}")
    plt.close()

def generate_summary_report(df, output_dir='results'):
    """Generate text summary report"""
    report = []
    report.append("=" * 60)
    report.append("OPTIMIZATION RESULTS SUMMARY")
    report.append("=" * 60)
    report.append("")
    
    report.append(f"Total Samples: {len(df)}")
    report.append("")
    
    report.append("Error Statistics:")
    report.append(f"  Mean Error:     {df['Error'].mean():8.4f}")
    report.append(f"  Std Error:      {df['Error'].std():8.4f}")
    report.append(f"  Mean Abs Error: {df['AbsError'].mean():8.4f}")
    report.append(f"  Max Abs Error:  {df['AbsError'].max():8.4f}")
    report.append(f"  Min Abs Error:  {df['AbsError'].min():8.4f}")
    report.append("")
    
    # MSE and RMSE
    mse = (df['Error'] ** 2).mean()
    rmse = np.sqrt(mse)
    report.append(f"  MSE:            {mse:8.4f}")
    report.append(f"  RMSE:           {rmse:8.4f}")
    report.append("")
    
    # R²
    ss_res = (df['Error'] ** 2).sum()
    ss_tot = ((df['Expected'] - df['Expected'].mean()) ** 2).sum()
    r2 = 1 - (ss_res / ss_tot) if ss_tot > 0 else 0
    report.append(f"  R² Score:       {r2:8.4f}")
    report.append("")
    
    # Classification metrics
    threshold = 0.5
    df['ExpectedClass'] = (df['Expected'] > threshold).astype(int)
    df['PredictedClass'] = (df['Predicted'] > threshold).astype(int)
    
    tp = ((df['PredictedClass'] == 1) & (df['ExpectedClass'] == 1)).sum()
    fp = ((df['PredictedClass'] == 1) & (df['ExpectedClass'] == 0)).sum()
    tn = ((df['PredictedClass'] == 0) & (df['ExpectedClass'] == 0)).sum()
    fn = ((df['PredictedClass'] == 0) & (df['ExpectedClass'] == 1)).sum()
    
    accuracy = (tp + tn) / (tp + tn + fp + fn) if (tp + tn + fp + fn) > 0 else 0
    precision = tp / (tp + fp) if (tp + fp) > 0 else 0
    recall = tp / (tp + fn) if (tp + fn) > 0 else 0
    f1 = 2 * precision * recall / (precision + recall) if (precision + recall) > 0 else 0
    
    report.append("Classification Performance (threshold=0.5):")
    report.append(f"  Accuracy:       {accuracy:8.4f}")
    report.append(f"  Precision:      {precision:8.4f}")
    report.append(f"  Recall:         {recall:8.4f}")
    report.append(f"  F1 Score:       {f1:8.4f}")
    report.append("")
    
    report.append("=" * 60)
    
    filepath = Path(output_dir) / 'analysis_summary.txt'
    with open(filepath, 'w') as f:
        f.write('\n'.join(report))
    
    print(f"✓ Saved: {filepath}")
    
    # Also print to console
    print("\n" + '\n'.join(report))

def main():
    print("=" * 60)
    print("ADS-B FUZZY SYSTEM RESULTS ANALYZER")
    print("=" * 60)
    print()
    
    # Create output directory
    output_dir = Path('results/graphs')
    output_dir.mkdir(parents=True, exist_ok=True)
    
    # Load data
    df = load_predictions()
    print()
    
    # Generate all plots
    print("Generating visualizations...")
    print()
    
    plot_prediction_scatter(df, output_dir)
    plot_error_distribution(df, output_dir)
    plot_residuals(df, output_dir)
    plot_feature_importance(df, output_dir)
    plot_anomaly_classification(df, output_dir)
    plot_feature_distributions(df, output_dir)
    
    print()
    generate_summary_report(df)
    
    print()
    print("=" * 60)
    print("ANALYSIS COMPLETE")
    print("=" * 60)
    print(f"\nAll graphs saved to: {output_dir}/")
    print("\nGenerated files:")
    print("  - prediction_scatter.png")
    print("  - error_analysis.png")
    print("  - residuals.png")
    print("  - feature_importance.png")
    print("  - classification_performance.png")
    print("  - feature_distributions.png")
    print("  - analysis_summary.txt")

if __name__ == '__main__':
    try:
        main()
    except Exception as e:
        print(f"\n✗ Error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
