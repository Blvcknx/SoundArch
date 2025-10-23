#!/usr/bin/env python3
"""
Generate Test Impulse Responses for LandscapeAcousticVST Validation

Creates reference impulse responses for known terrain scenarios
to validate plugin IR generation accuracy.
"""

import numpy as np
import scipy.signal
import wave
import json
from pathlib import Path

def generate_test_ir(distance_m, attenuations_db, sample_rate=44100, ir_length_s=0.5):
    """
    Generate test impulse response with frequency-dependent attenuation
    
    Args:
        distance_m: Distance between source and receiver
        attenuations_db: List of attenuations for octave bands (63Hz-8kHz)
        sample_rate: Audio sample rate
        ir_length_s: IR length in seconds
    
    Returns:
        numpy array: Stereo impulse response
    """
    
    ir_length_samples = int(ir_length_s * sample_rate)
    
    # Calculate delay based on distance
    speed_of_sound = 343.0  # m/s
    delay_s = distance_m / speed_of_sound
    delay_samples = int(delay_s * sample_rate)
    
    if delay_samples >= ir_length_samples:
        delay_samples = int(ir_length_samples * 0.9)  # Keep within bounds
    
    # Octave band center frequencies
    octave_bands = [63, 125, 250, 500, 1000, 2000, 4000, 8000]
    
    # Initialize IR buffer
    ir = np.zeros((ir_length_samples, 2))  # Stereo
    
    # Generate frequency components
    for i, freq in enumerate(octave_bands):
        if i < len(attenuations_db):
            # Convert attenuation to linear amplitude
            amplitude = 10**(-attenuations_db[i] / 20.0)
            
            # Generate windowed sinc for this frequency band
            window_size = min(int(sample_rate / freq), 512)
            window_size = max(window_size, 8)
            
            # Create sinc function centered at delay
            t = np.arange(-window_size//2, window_size//2) / sample_rate
            sinc_wave = np.sinc(2 * freq * t / sample_rate) * amplitude
            
            # Apply Hann window
            hann_window = 0.5 * (1 + np.cos(2 * np.pi * np.arange(window_size) / (window_size - 1)))
            sinc_wave *= hann_window
            
            # Add to IR at delayed position
            start_idx = max(0, delay_samples - window_size//2)
            end_idx = min(ir_length_samples, start_idx + len(sinc_wave))
            
            if start_idx < end_idx:
                wave_start = max(0, -(delay_samples - window_size//2))
                wave_end = wave_start + (end_idx - start_idx)
                
                # Add to both channels with slight variation for stereo
                ir[start_idx:end_idx, 0] += sinc_wave[wave_start:wave_end]
                ir[start_idx:end_idx, 1] += sinc_wave[wave_start:wave_end] * 0.95
    
    # Apply fade in/out to prevent clicks
    fade_samples = int(0.005 * sample_rate)  # 5ms fade
    
    # Fade in
    fade_in = np.linspace(0, 1, fade_samples)
    if fade_samples < ir_length_samples:
        ir[:fade_samples, :] *= fade_in.reshape(-1, 1)
    
    # Fade out
    fade_out = np.linspace(1, 0, fade_samples)
    if fade_samples < ir_length_samples:
        ir[-fade_samples:, :] *= fade_out.reshape(-1, 1)
    
    # Normalize to prevent clipping
    max_amplitude = np.max(np.abs(ir))
    if max_amplitude > 0:
        ir *= 0.8 / max_amplitude  # Leave some headroom
    
    return ir

def save_wav_ir(ir_data, filename, sample_rate=44100):
    """Save impulse response as WAV file"""
    
    # Convert to 16-bit integer
    ir_int16 = (ir_data * 32767).astype(np.int16)
    
    # Save as WAV
    with wave.open(filename, 'wb') as wav_file:
        wav_file.setnchannels(2)  # Stereo
        wav_file.setsampwidth(2)  # 16-bit
        wav_file.setframerate(sample_rate)
        wav_file.writeframes(ir_int16.tobytes())

def generate_test_scenarios():
    """Generate test IRs for various acoustic scenarios"""
    
    # Test scenarios matching validation data
    scenarios = [
        {
            'name': 'flat_100m_porous',
            'description': 'Flat terrain, 100m, porous ground',
            'distance_m': 100.0,
            'attenuations_db': [51.1, 51.2, 51.4, 51.8, 52.5, 53.8, 56.2, 60.1],
            'metadata': {
                'distance_m': 100.0,
                'ground_type': 'porous',
                'temperature_c': 20.0,
                'humidity_pct': 70.0,
                'source_height_m': 2.0,
                'receiver_height_m': 2.0
            }
        },
        {
            'name': 'flat_500m_hard',
            'description': 'Flat terrain, 500m, hard ground',
            'distance_m': 500.0,
            'attenuations_db': [65.0, 65.1, 65.3, 65.8, 66.7, 68.4, 72.1, 78.3],
            'metadata': {
                'distance_m': 500.0,
                'ground_type': 'hard',
                'temperature_c': 15.0,
                'humidity_pct': 60.0,
                'source_height_m': 2.0,
                'receiver_height_m': 2.0
            }
        },
        {
            'name': 'flat_1000m_mixed',
            'description': 'Flat terrain, 1000m, mixed ground',
            'distance_m': 1000.0,
            'attenuations_db': [71.0, 71.2, 71.6, 72.4, 74.1, 77.2, 83.8, 95.6],
            'metadata': {
                'distance_m': 1000.0,
                'ground_type': 'mixed',
                'temperature_c': 25.0,
                'humidity_pct': 80.0,
                'source_height_m': 3.0,
                'receiver_height_m': 1.5
            }
        },
        {
            'name': 'barrier_300m_5m',
            'description': 'With 5m barrier at 300m distance',
            'distance_m': 300.0,
            'attenuations_db': [64.5, 67.8, 71.2, 74.5, 77.8, 81.1, 84.4, 87.7],
            'metadata': {
                'distance_m': 300.0,
                'ground_type': 'porous',
                'temperature_c': 20.0,
                'humidity_pct': 70.0,
                'source_height_m': 2.0,
                'receiver_height_m': 2.0,
                'barrier_height_m': 5.0
            }
        }
    ]
    
    print("Generating Test Impulse Responses")
    print("=" * 40)
    
    # Create output directory
    output_dir = Path('test_impulse_responses')
    output_dir.mkdir(exist_ok=True)
    
    all_metadata = {}
    
    for scenario in scenarios:
        print(f"📊 Generating: {scenario['name']}")
        print(f"   {scenario['description']}")
        
        # Generate IR
        ir = generate_test_ir(
            distance_m=scenario['distance_m'],
            attenuations_db=scenario['attenuations_db'],
            sample_rate=44100,
            ir_length_s=0.5
        )
        
        # Save WAV file
        wav_filename = output_dir / f"{scenario['name']}.wav"
        save_wav_ir(ir, str(wav_filename), 44100)
        
        # Store metadata
        all_metadata[scenario['name']] = scenario['metadata']
        all_metadata[scenario['name']]['wav_file'] = str(wav_filename)
        all_metadata[scenario['name']]['attenuations_db'] = scenario['attenuations_db']
        
        print(f"   ✅ Saved: {wav_filename}")
        print(f"   📏 Length: {len(ir)} samples ({len(ir)/44100:.2f} seconds)")
        print(f"   📊 Peak: {np.max(np.abs(ir)):.3f}")
        print()
    
    # Save metadata JSON
    metadata_file = output_dir / 'test_ir_metadata.json'
    with open(metadata_file, 'w') as f:
        json.dump(all_metadata, f, indent=2)
    
    print(f"💾 Metadata saved: {metadata_file}")
    print(f"📁 All files in: {output_dir}")
    
    return all_metadata

def analyze_ir_spectrum(ir_data, sample_rate=44100):
    """Analyze frequency spectrum of generated IR"""
    
    # Compute FFT
    fft = np.fft.rfft(ir_data[:, 0])  # Use left channel
    freqs = np.fft.rfftfreq(len(ir_data), 1/sample_rate)
    magnitude_db = 20 * np.log10(np.abs(fft) + 1e-10)
    
    # Find octave band levels
    octave_bands = [63, 125, 250, 500, 1000, 2000, 4000, 8000]
    octave_levels = []
    
    for freq in octave_bands:
        # Find frequency bin closest to octave band center
        idx = np.argmin(np.abs(freqs - freq))
        octave_levels.append(magnitude_db[idx])
    
    return octave_bands, octave_levels

def validate_generated_irs():
    """Validate that generated IRs have expected frequency characteristics"""
    
    print("\nValidating Generated Impulse Responses")
    print("=" * 50)
    
    metadata_file = Path('test_impulse_responses/test_ir_metadata.json')
    
    if not metadata_file.exists():
        print("❌ Metadata file not found. Run generate_test_scenarios() first.")
        return
    
    with open(metadata_file, 'r') as f:
        metadata = json.load(f)
    
    for scenario_name, data in metadata.items():
        wav_file = Path(data['wav_file'])
        
        if not wav_file.exists():
            print(f"❌ WAV file not found: {wav_file}")
            continue
        
        print(f"\n🔍 Analyzing: {scenario_name}")
        
        # Load WAV file
        with wave.open(str(wav_file), 'rb') as wav:
            frames = wav.readframes(wav.getnframes())
            ir_data = np.frombuffer(frames, dtype=np.int16).reshape(-1, 2).astype(np.float32) / 32767.0
        
        # Analyze spectrum
        octave_bands, measured_levels = analyze_ir_spectrum(ir_data, 44100)
        expected_attenuations = data['attenuations_db']
        
        print(f"   Distance: {data['distance_m']} m")
        print(f"   Ground: {data['ground_type']}")
        print("   Frequency analysis:")
        
        for i, freq in enumerate(octave_bands):
            if i < len(expected_attenuations):
                expected = -expected_attenuations[i]  # Convert attenuation to gain
                measured = measured_levels[i]
                error = abs(measured - expected)
                
                status = "✅" if error < 5.0 else "⚠️"  # 5 dB tolerance for spectral analysis
                print(f"     {freq:>4} Hz: Expected={expected:6.1f} dB, Measured={measured:6.1f} dB, Error={error:4.1f} dB {status}")
    
    print("\n✅ IR validation complete")

def main():
    """Main test IR generation routine"""
    
    print("LandscapeAcousticVST Test Impulse Response Generator")
    print("=" * 55)
    
    # Generate test scenarios
    generate_test_scenarios()
    
    # Validate generated IRs
    validate_generated_irs()
    
    print("\n📝 Usage Notes:")
    print("• Load generated WAV files into your DAW for listening tests")
    print("• Compare plugin-generated IRs against these reference files")
    print("• Use metadata JSON for automated validation scripts")
    print("• Expected frequency response should match attenuation patterns")

if __name__ == "__main__":
    main()