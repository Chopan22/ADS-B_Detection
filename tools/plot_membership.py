#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from pathlib import Path
import sys

# Set style
sns.set_style("whitegrid")
plt.rcParams['figure.figsize'] = (16, 12)

def load_chromosome(filepath='../build/results/optimized_params.txt'):
    """Load chromosome genes from results file"""
    genes = []
    
    try:
        with open(filepath, 'r') as f:
            for line in f:
                line = line.strip()
                # Skip comments and empty lines
                if not line or line.startswith('#'):
                    continue
                # Parse comma-separated values
                values = [float(x.strip()) for x in line.split(',') if x.strip()]
                genes.extend(values)
        
        if len(genes) != 66:
            print(f"Warning: Expected 66 genes, got {len(genes)}")
            return None
            
        print(f" Loaded {len(genes)} genes from {filepath}")
        return genes
        
    except FileNotFoundError:
        print(f" Error: {filepath} not found")
        print("  Make sure you've run the optimizer first")
        return None
    except Exception as e:
        print(f" Error loading chromosome: {e}")
        return None

def z_shape(x, a, b):
    """Z-shaped membership function"""
    result = np.zeros_like(x, dtype=float)
    mask1 = x <= a
    mask2 = (x > a) & (x < b)
    mask3 = x >= b
    
    result[mask1] = 1.0
    t = (x[mask2] - a) / (b - a)
    result[mask2] = 1.0 - 2.0 * t * t
    result[mask3] = 0.0
    
    return result

def s_shape(x, a, b):
    """S-shaped membership function"""
    result = np.zeros_like(x, dtype=float)
    mask1 = x <= a
    mask2 = (x > a) & (x < b)
    mask3 = x >= b
    
    result[mask1] = 0.0
    t = (x[mask2] - a) / (b - a)
    result[mask2] = 2.0 * t * t
    result[mask3] = 1.0
    
    return result

def triangle(x, a, b, c):
    """Triangular membership function"""
    result = np.zeros_like(x, dtype=float)
    
    mask1 = (x > a) & (x < b)
    mask2 = x == b
    mask3 = (x > b) & (x < c)
    
    result[mask1] = (x[mask1] - a) / (b - a)
    result[mask2] = 1.0
    result[mask3] = (c - x[mask3]) / (c - b)
    
    return result

def plot_variable_mfs(ax, x_range, params, labels, title, xlabel):
    """Plot membership functions for a single variable"""
    x = np.linspace(x_range[0], x_range[1], 1000)
    
    colors = ['blue', 'green', 'orange', 'red', 'purple']
    
    # Parse parameters based on structure
    # For 5 MFs (13 params): Z(2), Tri(3), Tri(3), Tri(3), S(2)
    # For 3 MFs (7 params): Z(2), Tri(3), S(2)
    
    if len(params) == 13:  # 5 membership functions
        # Negative_Large (Z-shape)
        y = z_shape(x, params[0], params[1])
        ax.plot(x, y, label=labels[0], linewidth=2, color=colors[0])
        ax.fill_between(x, 0, y, alpha=0.2, color=colors[0])
        
        # Negative (Triangle)
        y = triangle(x, params[2], params[3], params[4])
        ax.plot(x, y, label=labels[1], linewidth=2, color=colors[1])
        ax.fill_between(x, 0, y, alpha=0.2, color=colors[1])
        
        # Zero (Triangle)
        y = triangle(x, params[5], params[6], params[7])
        ax.plot(x, y, label=labels[2], linewidth=2, color=colors[2])
        ax.fill_between(x, 0, y, alpha=0.2, color=colors[2])
        
        # Positive (Triangle)
        y = triangle(x, params[8], params[9], params[10])
        ax.plot(x, y, label=labels[3], linewidth=2, color=colors[3])
        ax.fill_between(x, 0, y, alpha=0.2, color=colors[3])
        
        # Positive_Large (S-shape)
        y = s_shape(x, params[11], params[12])
        ax.plot(x, y, label=labels[4], linewidth=2, color=colors[4])
        ax.fill_between(x, 0, y, alpha=0.2, color=colors[4])
        
    elif len(params) == 7:  # 3 membership functions
        # Small/Low (Z-shape)
        y = z_shape(x, params[0], params[1])
        ax.plot(x, y, label=labels[0], linewidth=2, color=colors[0])
        ax.fill_between(x, 0, y, alpha=0.2, color=colors[0])
        
        # Medium (Triangle)
        y = triangle(x, params[2], params[3], params[4])
        ax.plot(x, y, label=labels[1], linewidth=2, color=colors[2])
        ax.fill_between(x, 0, y, alpha=0.2, color=colors[2])
        
        # Large/High (S-shape)
        y = s_shape(x, params[5], params[6])
        ax.plot(x, y, label=labels[2], linewidth=2, color=colors[4])
        ax.fill_between(x, 0, y, alpha=0.2, color=colors[4])
    
    ax.set_xlabel(xlabel, fontsize=11)
    ax.set_ylabel('Membership Degree', fontsize=11)
    ax.set_title(title, fontsize=12, fontweight='bold')
    ax.set_ylim(-0.05, 1.05)
    ax.set_xlim(x_range[0], x_range[1])
    ax.legend(loc='best', fontsize=9)
    ax.grid(True, alpha=0.3)

def plot_all_membership_functions(genes, output_dir='results/graphs'):
    """Plot all 6 fuzzy variables"""
    
    # Create figure with subplots
    fig, axes = plt.subplots(3, 2, figsize=(16, 12))
    
    # Variable 1: SpeedChange (genes 0-12, 13 params)
    plot_variable_mfs(
        axes[0, 0],
        x_range=(-10, 10),
        params=genes[0:13],
        labels=['Negative Large', 'Negative', 'Zero', 'Positive', 'Positive Large'],
        title='Speed Change',
        xlabel='Speed Change (m/s)'
    )
    
    # Variable 2: HeadingChange (genes 13-25, 13 params)
    plot_variable_mfs(
        axes[0, 1],
        x_range=(-180, 180),
        params=genes[13:26],
        labels=['Negative Large', 'Negative', 'Zero', 'Positive', 'Positive Large'],
        title='Heading Change',
        xlabel='Heading Change (degrees)'
    )
    
    # Variable 3: VerticalRateChange (genes 26-38, 13 params)
    plot_variable_mfs(
        axes[1, 0],
        x_range=(-20, 20),
        params=genes[26:39],
        labels=['Negative Large', 'Negative', 'Zero', 'Positive', 'Positive Large'],
        title='Vertical Rate Change',
        xlabel='Vertical Rate Change (m/s)'
    )
    
    # Variable 4: AltitudeChange (genes 39-51, 13 params)
    plot_variable_mfs(
        axes[1, 1],
        x_range=(-1000, 1000),
        params=genes[39:52],
        labels=['Negative Large', 'Negative', 'Zero', 'Positive', 'Positive Large'],
        title='Altitude Change',
        xlabel='Altitude Change (m)'
    )
    
    # Variable 5: TimeGap (genes 52-58, 7 params)
    plot_variable_mfs(
        axes[2, 0],
        x_range=(0, 60),
        params=genes[52:59],
        labels=['Small', 'Medium', 'Large'],
        title='Time Gap',
        xlabel='Time Gap (s)'
    )
    
    # Variable 6: AnomalyLevel (genes 59-65, 7 params)
    plot_variable_mfs(
        axes[2, 1],
        x_range=(0, 1),
        params=genes[59:66],
        labels=['Low', 'Medium', 'High'],
        title='Anomaly Level (Output)',
        xlabel='Anomaly Level'
    )
    
    plt.suptitle('Optimized Fuzzy Membership Functions', 
                 fontsize=16, fontweight='bold', y=0.995)
    plt.tight_layout(rect=[0, 0, 1, 0.99])
    
    # Save figure
    output_path = Path(output_dir)
    output_path.mkdir(parents=True, exist_ok=True)
    filepath = output_path / 'membership_functions.png'
    plt.savefig(filepath, dpi=300, bbox_inches='tight')
    print(f" Saved: {filepath}")
    
    plt.close()

def plot_comparison(genes, output_dir='results/graphs'):
    """Plot comparison between default and optimized MFs for one variable"""
    
    # Default parameters for SpeedChange
    default_params = [-500,-100,-500,-100,0,-50,0,50,0,100,500,100,500]
    optimized_params = genes[39:52]
    
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))
    
    # Plot default
    plot_variable_mfs(
        ax1,
        x_range=(-1000, 1000),
        params=default_params,
        labels=['Negative Large', 'Negative', 'Zero', 'Positive', 'Positive Large'],
        title='Speed Change - Default Parameters',
        xlabel='Speed Change (m/s)'
    )
    
    # Plot optimized
    plot_variable_mfs(
        ax2,
        x_range=(-1000, 1000),
        params=optimized_params,
        labels=['Negative Large', 'Negative', 'Zero', 'Positive', 'Positive Large'],
        title='Speed Change - Optimized Parameters',
        xlabel='Speed Change (m/s)'
    )
    
    plt.suptitle('Before vs After GA Optimization', 
                 fontsize=14, fontweight='bold')
    plt.tight_layout(rect=[0, 0, 1, 0.96])
    
    filepath = Path(output_dir) / 'mf_comparison.png'
    plt.savefig(filepath, dpi=300, bbox_inches='tight')
    print(f" Saved: {filepath}")
    
    plt.close()

def print_parameter_summary(genes):
    """Print summary of optimized parameters"""
    print("\n" + "="*60)
    print("OPTIMIZED PARAMETERS SUMMARY")
    print("="*60)
    
    variables = [
        ("Speed Change", genes[0:13], (-10, 10)),
        ("Heading Change", genes[13:26], (-180, 180)),
        ("Vertical Rate Change", genes[26:39], (-20, 20)),
        ("Altitude Change", genes[39:52], (-1000, 1000)),
        ("Time Gap", genes[52:59], (0, 60)),
        ("Anomaly Level", genes[59:66], (0, 1))
    ]
    
    for name, params, (min_val, max_val) in variables:
        print(f"\n{name}:")
        print(f"  Range: [{min_val}, {max_val}]")
        if len(params) == 13:
            print(f"  Neg_Large:  [{params[0]:.2f}, {params[1]:.2f}]")
            print(f"  Negative:   [{params[2]:.2f}, {params[3]:.2f}, {params[4]:.2f}]")
            print(f"  Zero:       [{params[5]:.2f}, {params[6]:.2f}, {params[7]:.2f}]")
            print(f"  Positive:   [{params[8]:.2f}, {params[9]:.2f}, {params[10]:.2f}]")
            print(f"  Pos_Large:  [{params[11]:.2f}, {params[12]:.2f}]")
        else:  # 7 params
            print(f"  Small/Low:  [{params[0]:.2f}, {params[1]:.2f}]")
            print(f"  Medium:     [{params[2]:.2f}, {params[3]:.2f}, {params[4]:.2f}]")
            print(f"  Large/High: [{params[5]:.2f}, {params[6]:.2f}]")
    
    print("\n" + "="*60)

def main():
    print("="*60)
    print("MEMBERSHIP FUNCTION VISUALIZER")
    print("="*60)
    print()
    
    # Load chromosome
    if len(sys.argv) > 1:
        filepath = sys.argv[1]
    else:
        filepath = '../build/results/optimized_params.txt'
    
    genes = load_chromosome(filepath)
    
    if genes is None:
        sys.exit(1)
    
    print()
    
    # Print parameter summary
    print_parameter_summary(genes)
    
    # Create output directory
    output_dir = Path('results/graphs')
    output_dir.mkdir(parents=True, exist_ok=True)
    
    print("\nGenerating visualizations...")
    print()
    
    # Plot all membership functions
    plot_all_membership_functions(genes, output_dir)
    
    # Plot before/after comparison
    plot_comparison(genes, output_dir)
    
    print()
    print("="*60)
    print("VISUALIZATION COMPLETE")
    print("="*60)
    print()
    print("Generated files:")
    print("  - results/graphs/membership_functions.png")
    print("  - results/graphs/mf_comparison.png")
    print()

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print("\n\nInterrupted by user")
        sys.exit(1)
    except Exception as e:
        print(f"\n✗ Error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
