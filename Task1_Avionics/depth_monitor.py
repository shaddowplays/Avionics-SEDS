# Name: Siddarth Varma Kalidindi
# ID: 2026B5PS0896H

import matplotlib.pyplot as plt
import matplotlib.animation as animation
import pandas as pd

# --- 1. Data Extraction and Cleaning ---
file_path = 'Depth Data.csv'

df = pd.read_csv(file_path)
df['Depth_clean'] = pd.to_numeric(df['Depth (m)'], errors='coerce')

df['Depth_clean'] = df['Depth_clean'].interpolate(method='linear')

df['Depth_mag'] = df['Depth_clean'].abs()

rolling_median = df['Depth_mag'].rolling(window=5, min_periods=1, center=True).median()
is_spike = (df['Depth_mag'] - rolling_median).abs() > 100
df['Filtered'] = df['Depth_mag'].copy()
df.loc[is_spike, 'Filtered'] = rolling_median[is_spike]

df['Smoothed'] = df['Filtered'].ewm(span=5, adjust=False).mean()

time_steps = df['Point'].values
raw_depths = df['Depth_mag'].values
clean_depths = df['Smoothed'].values

# --- 2. ANIMATED GRAPH  ---
fig, ax = plt.subplots(figsize=(10, 5))
fig.suptitle("Athena's Navigation System - Sea Floor Depth Monitor", fontsize=14, fontweight='bold')

ax.set_xlim(0, max(time_steps))
ax.set_ylim(0, max(clean_depths) + 50)
ax.set_xlabel("Time (seconds)", fontsize=11)
ax.set_ylabel("Depth (meters)", fontsize=11)
ax.grid(True, linestyle='--', alpha=0.6)

raw_line, = ax.plot([], [], color='red', alpha=0.4, linestyle=':', label='Raw Sensor Data (Corrupted)')
clean_line, = ax.plot([], [], color='navy', linewidth=2, label='Filtered Sea Floor Depth')
ax.legend(loc='upper right')

def update(frame):
    raw_line.set_data(time_steps[:frame], raw_depths[:frame])
    clean_line.set_data(time_steps[:frame], clean_depths[:frame])
    return raw_line, clean_line

ani = animation.FuncAnimation(fig, update, frames=len(time_steps) + 1, interval=100, repeat=False)

plt.tight_layout()
plt.show()