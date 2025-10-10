#!/usr/bin/env python3
# plot_results.py
# Usage: python3 plot_results.py [results.csv]
import sys
import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

fn = sys.argv[1] if len(sys.argv) > 1 else "results.csv"
out_summary = "summary_results.csv"
plots_dir = "plots"

if not os.path.exists(fn):
    print("File not found:", fn)
    sys.exit(1)

os.makedirs(plots_dir, exist_ok=True)

df = pd.read_csv(fn)
# normalize column names if necessary
df = df.rename(columns=lambda x: x.strip())

# group by N and max_threads computing mean elapsed
agg = df.groupby(['N', 'max_threads']).agg({'elapsed':'mean'}).reset_index()
# get baseline T(1) for each N
baselines = agg[agg['max_threads']==1].set_index('N')['elapsed']

rows = []
for _, row in agg.iterrows():
    N = row['N']; m = int(row['max_threads']); t = float(row['elapsed'])
    t1 = baselines.loc[N]
    speedup = t1 / t if t > 0 else np.nan
    efficiency = speedup / m if m > 0 else np.nan
    rows.append({'N':N,'max_threads':m,'elapsed':t,'speedup':speedup,'efficiency':efficiency})

out = pd.DataFrame(rows)
out.to_csv(out_summary, index=False)
print("Summary saved to", out_summary)

# Plot per N
for N in sorted(out['N'].unique()):
    sub = out[out['N']==N].sort_values('max_threads')
    plt.figure()
    plt.plot(sub['max_threads'], sub['elapsed'], marker='o')
    plt.xlabel('max_threads')
    plt.ylabel('elapsed (s)')
    plt.title(f'N={N} elapsed')
    plt.grid(True)
    plt.savefig(os.path.join(plots_dir, f"{N}_elapsed.png"))
    plt.close()

    plt.figure()
    plt.plot(sub['max_threads'], sub['speedup'], marker='o')
    plt.xlabel('max_threads')
    plt.ylabel('speedup')
    plt.title(f'N={N} speedup')
    plt.grid(True)
    plt.savefig(os.path.join(plots_dir, f"{N}_speedup.png"))
    plt.close()

    plt.figure()
    plt.plot(sub['max_threads'], sub['efficiency'], marker='o')
    plt.xlabel('max_threads')
    plt.ylabel('efficiency')
    plt.title(f'N={N} efficiency')
    plt.grid(True)
    plt.savefig(os.path.join(plots_dir, f"{N}_efficiency.png"))
    plt.close()

print("Plots saved to", plots_dir)