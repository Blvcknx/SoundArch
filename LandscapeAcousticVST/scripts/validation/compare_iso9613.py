#!/usr/bin/env python3
"""
ISO 9613-2 Validation Script
Compare plugin calculations against reference values
"""

import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
import json

def iso9613_reference_divergence(distance_m):
    """Reference divergence calculation (ISO 9613-2 Eq. 7)"""
    return 20 * np.log10(distance_m) + 11.0

def iso9613_reference_atmospheric(distance_m, frequency_hz, temp_c=20.0, humidity_pct=70.0):
    """
    Simplified atmospheric absorption (ISO 9613-1)
    Uses empirical approximation for validation
    """
    # Temperature factor
    temp_factor = 1.0 + 0.01 * (temp_c - 20.0)
    
    # Humidity factor (simplified)
    humidity_factor = 1.0 + 0.003 * (70.0 - humidity_pct) / 70.0
    
    # Base absorption coefficient (dB/m) - simplified model
    if frequency_hz < 500:
        alpha = 0.0005 * (frequency_hz / 500.0)
    elif frequency_hz < 2000:
        alpha = 0.0005 + 0.004 * ((frequency_hz - 500.0) / 1500.0)
    else:
        alpha = 0.0045 + 0.015 * ((frequency_hz - 2000.0) / 6000.0)
    
    alpha *= temp_factor * humidity_factor
    return alpha * distance_m

def iso9613_reference_ground(distance_m, source_height_m, receiver_height_m, 
                           frequency_hz, ground_factor=1.0):
    """
    Simplified ground effect calculation
    Based on ISO 9613-2 methodology
    """
    h_eff = np.sqrt(source_height_m * receiver_height_m)
    f_peak = 343.0 / (4.0 * h_eff)
    
    freq_factor = 1.0 / (1.0 + ((frequency_hz / f_peak) - 1.0)**2)
    max_attenuation = ground_factor * 8.0  # Max 8 dB for porous ground
    
    return max_attenuation * freq_factor

def iso9613_reference_barrier(d1_m, d2_m, barrier_height_m, frequency_hz):
    """
    Knife-edge diffraction (ISO 9613-2)
    """
    if barrier_height_m <= 0:
        return 0.0
    
    wavelength = 343.0 / frequency_hz
    nu = (2.0 / wavelength) * np.sqrt((d1_m * d2_m) / (d1_m + d2_m)) * barrier_height_m
    
    if nu < -0.2:
        return 0.0
    else:
        return 10.0 + 13.0 * nu

def calculate_reference_total_attenuation(distance_m, frequency_hz, 
                                        source_height_m=2.0, receiver_height_m=2.0,
                                        temp_c=20.0, humidity_pct=70.0, 
                                        ground_factor=1.0, barrier_height_m=0.0):
    """Calculate total reference attenuation using ISO 9613-2"""
    
    A_div = iso9613_reference_divergence(distance_m)
    A_atm = iso9613_reference_atmospheric(distance_m, frequency_hz, temp_c, humidity_pct)
    A_gr = iso9613_reference_ground(distance_m, source_height_m, receiver_height_m, 
                                   frequency_hz, ground_factor)
    
    A_bar = 0.0
    if barrier_height_m > 0:
        # Assume barrier at midpoint
        d1 = d2 = distance_m / 2.0
        A_bar = iso9613_reference_barrier(d1, d2, barrier_height_m, frequency_hz)
    
    return A_div + A_atm + A_gr + A_bar

def generate_validation_data():
    """Generate reference dataset for plugin validation"""
    
    # Test scenarios
    scenarios = [
        {
            'name': 'Flat terrain, short distance',
            'distance_m': 100.0,
            'source_height_m': 2.0,
            'receiver_height_m': 2.0,
            'temperature_c': 20.0,
            'humidity_pct': 70.0,
            'ground_factor': 1.0,  # Porous
            'barrier_height_m': 0.0
        },
        {
            'name': 'Flat terrain, medium distance',
            'distance_m': 500.0,
            'source_height_m': 2.0,
            'receiver_height_m': 2.0,
            'temperature_c': 15.0,
            'humidity_pct': 60.0,
            'ground_factor': 0.0,  # Hard
            'barrier_height_m': 0.0
        },
        {
            'name': 'Flat terrain, long distance',
            'distance_m': 1000.0,
            'source_height_m': 3.0,
            'receiver_height_m': 1.5,
            'temperature_c': 25.0,
            'humidity_pct': 80.0,
            'ground_factor': 0.5,  # Mixed
            'barrier_height_m': 0.0
        },
        {
            'name': 'With barrier obstruction',
            'distance_m': 300.0,
            'source_height_m': 2.0,
            'receiver_height_m': 2.0,
            'temperature_c': 20.0,
            'humidity_pct': 70.0,
            'ground_factor': 1.0,
            'barrier_height_m': 5.0
        }
    ]
    
    # Octave band frequencies
    frequencies = [63, 125, 250, 500, 1000, 2000, 4000, 8000]
    
    validation_data = {}
    
    for scenario in scenarios:
        scenario_results = {}
        
        for freq in frequencies:
            atten = calculate_reference_total_attenuation(
                distance_m=scenario['distance_m'],
                frequency_hz=freq,
                source_height_m=scenario['source_height_m'],
                receiver_height_m=scenario['receiver_height_m'],
                temp_c=scenario['temperature_c'],
                humidity_pct=scenario['humidity_pct'],
                ground_factor=scenario['ground_factor'],
                barrier_height_m=scenario['barrier_height_m']
            )
            scenario_results[f'{freq}Hz'] = round(atten, 2)
        
        # Add scenario parameters
        scenario_results['parameters'] = scenario
        validation_data[scenario['name']] = scenario_results
    
    return validation_data

def plot_frequency_response(validation_data):
    """Plot frequency response curves for visual validation"""
    
    frequencies = [63, 125, 250, 500, 1000, 2000, 4000, 8000]
    
    fig, axes = plt.subplots(2, 2, figsize=(12, 10))
    axes = axes.flatten()
    
    for i, (scenario_name, data) in enumerate(validation_data.items()):
        if i >= 4:  # Only plot first 4 scenarios
            break
            
        attenuations = [data[f'{freq}Hz'] for freq in frequencies]
        
        axes[i].semilogx(frequencies, attenuations, 'b-o', linewidth=2, markersize=6)
        axes[i].set_title(f'{scenario_name}')
        axes[i].set_xlabel('Frequency (Hz)')
        axes[i].set_ylabel('Attenuation (dB)')
        axes[i].grid(True, alpha=0.3)
        axes[i].set_xlim(50, 10000)
    
    plt.tight_layout()
    plt.savefig('iso9613_validation_curves.png', dpi=150, bbox_inches='tight')
    plt.show()

def validate_plugin_results(plugin_results_file, validation_data, tolerance_db=2.0):
    """
    Compare plugin results against reference calculations
    
    Args:
        plugin_results_file: JSON file with plugin calculation results
        validation_data: Reference calculation results
        tolerance_db: Acceptable deviation (dB)
    """
    
    try:
        with open(plugin_results_file, 'r') as f:
            plugin_data = json.load(f)
    except FileNotFoundError:
        print(f"Plugin results file not found: {plugin_results_file}")
        print("Generate plugin results first by running test scenarios")
        return False
    
    print("ISO 9613-2 Validation Results")
    print("=" * 50)
    
    all_passed = True
    
    for scenario_name, ref_data in validation_data.items():
        if scenario_name not in plugin_data:
            print(f"❌ Missing plugin data for scenario: {scenario_name}")
            all_passed = False
            continue
        
        plugin_scenario = plugin_data[scenario_name]
        print(f"\n📊 Scenario: {scenario_name}")
        
        scenario_passed = True
        for freq_key in ['63Hz', '125Hz', '250Hz', '500Hz', '1000Hz', '2000Hz', '4000Hz', '8000Hz']:
            if freq_key in ref_data and freq_key in plugin_scenario:
                ref_value = ref_data[freq_key]
                plugin_value = plugin_scenario[freq_key]
                deviation = abs(plugin_value - ref_value)
                
                status = "✅" if deviation <= tolerance_db else "❌"
                print(f"  {freq_key:>6}: Ref={ref_value:6.1f} dB, Plugin={plugin_value:6.1f} dB, "
                      f"Δ={deviation:4.1f} dB {status}")
                
                if deviation > tolerance_db:
                    scenario_passed = False
        
        if scenario_passed:
            print(f"  ✅ Scenario PASSED (all frequencies within {tolerance_db} dB)")
        else:
            print(f"  ❌ Scenario FAILED (some frequencies exceed {tolerance_db} dB tolerance)")
            all_passed = False
    
    print("\n" + "=" * 50)
    if all_passed:
        print("🎉 ALL VALIDATION TESTS PASSED")
        print("Plugin calculations match ISO 9613-2 reference within tolerance")
    else:
        print("⚠️  VALIDATION ISSUES DETECTED")
        print("Review failing test cases and check plugin implementation")
    
    return all_passed

def main():
    """Main validation routine"""
    
    print("ISO 9613-2 Validation Script for LandscapeAcousticVST")
    print("=" * 60)
    
    # Generate reference validation data
    print("📋 Generating reference calculations...")
    validation_data = generate_validation_data()
    
    # Save reference data
    with open('iso9613_reference_data.json', 'w') as f:
        json.dump(validation_data, f, indent=2)
    print("✅ Reference data saved to: iso9613_reference_data.json")
    
    # Display reference results
    print("\n📊 Reference Calculation Results:")
    print("-" * 40)
    
    for scenario_name, data in validation_data.items():
        print(f"\n{scenario_name}:")
        params = data['parameters']
        print(f"  Distance: {params['distance_m']} m")
        print(f"  Heights: {params['source_height_m']} m → {params['receiver_height_m']} m")
        print(f"  Conditions: {params['temperature_c']}°C, {params['humidity_pct']}% RH")
        print(f"  Ground: {'Hard' if params['ground_factor']==0 else 'Porous' if params['ground_factor']==1 else 'Mixed'}")
        
        print("  Frequency response:")
        for freq in [63, 125, 250, 500, 1000, 2000, 4000, 8000]:
            atten = data[f'{freq}Hz']
            print(f"    {freq:>4} Hz: {atten:6.1f} dB")
    
    # Generate validation plots
    print("\n📈 Generating validation plots...")
    plot_frequency_response(validation_data)
    
    # Check for plugin results
    plugin_results_file = 'plugin_test_results.json'
    if Path(plugin_results_file).exists():
        print(f"\n🔍 Validating against plugin results...")
        validate_plugin_results(plugin_results_file, validation_data)
    else:
        print(f"\n💡 To validate plugin:")
        print(f"   1. Run plugin with test scenarios")
        print(f"   2. Save results to: {plugin_results_file}")
        print(f"   3. Re-run this script for validation")

if __name__ == "__main__":
    main()