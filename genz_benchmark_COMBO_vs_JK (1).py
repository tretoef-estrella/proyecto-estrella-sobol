#!/usr/bin/env python3
"""
genz_benchmark_COMBO_vs_JK.py — D217 Genz benchmark patch (Auditor D4.1)

Compares Genz integration error of:
  - COMBO_3027 direction numbers (current project record, audit=3027)
  - JK_2008 baseline (audit=3196, reference)

Functions: 6 Genz test functions standard QMC suite.
  1. Oscillatory:    f(x) = cos(2π·u + Σ a_i·x_i)
  2. Product peak:   f(x) = Π (a_i^-2 + (x_i - u_i)^2)^-1
  3. Corner peak:    f(x) = (1 + Σ a_i·x_i)^-(d+1)
  4. Gaussian:       f(x) = exp(-Σ a_i^2·(x_i - u_i)^2)
  5. Continuous:     f(x) = exp(-Σ a_i·|x_i - u_i|)
  6. Discontinuous:  f(x) = exp(Σ a_i·x_i) if x_1 < u_1 ∧ x_2 < u_2, else 0

Dimensions: d ∈ {10, 20, 30}.
Sample size: N = 2^16 (65536) per replicate.
Replicates: 30 per (function, dim, sequence).

Output:
  - Per-function table: mean abs error JK_2008 vs COMBO_3027.
  - F75 dual-form test: ¿F_2 audit improvement (5.3%) translates to Genz error reduction?
  - PASS criterion: COMBO_3027 mean error ≤ JK_2008 mean error in ≥4 of 6 functions × 3 dims.

Requires: numpy, scipy, qmcpy (already installed on Architect M2).

Usage:
  cd ~/Downloads
  python3 genz_benchmark_COMBO_vs_JK.py TOGORDO_COMBO_v1_RECORD.txt new-joe-kuo-6.21201
"""

import sys
import time
import numpy as np
from pathlib import Path

# ============================================================================
# Genz functions
# ============================================================================

def genz_oscillatory(x, a, u):
    """ f(x) = cos(2π·u_0 + Σ a_i·x_i) """
    return np.cos(2 * np.pi * u[0] + np.sum(a * x, axis=1))

def genz_product_peak(x, a, u):
    """ f(x) = Π (a_i^-2 + (x_i - u_i)^2)^-1 """
    return np.prod(1.0 / (a ** -2 + (x - u) ** 2), axis=1)

def genz_corner_peak(x, a, u):
    """ f(x) = (1 + Σ a_i·x_i)^-(d+1) """
    d = x.shape[1]
    return (1 + np.sum(a * x, axis=1)) ** -(d + 1)

def genz_gaussian(x, a, u):
    """ f(x) = exp(-Σ a_i^2·(x_i - u_i)^2) """
    return np.exp(-np.sum(a ** 2 * (x - u) ** 2, axis=1))

def genz_continuous(x, a, u):
    """ f(x) = exp(-Σ a_i·|x_i - u_i|) """
    return np.exp(-np.sum(a * np.abs(x - u), axis=1))

def genz_discontinuous(x, a, u):
    """ f(x) = exp(Σ a_i·x_i) if x_1 < u_1 ∧ x_2 < u_2, else 0 """
    mask = (x[:, 0] < u[0]) & (x[:, 1] < u[1]) if x.shape[1] >= 2 else (x[:, 0] < u[0])
    result = np.zeros(x.shape[0])
    if np.any(mask):
        result[mask] = np.exp(np.sum(a * x[mask], axis=1))
    return result

GENZ_FUNCTIONS = [
    ("Oscillatory",    genz_oscillatory),
    ("Product peak",   genz_product_peak),
    ("Corner peak",    genz_corner_peak),
    ("Gaussian",       genz_gaussian),
    ("Continuous",     genz_continuous),
    ("Discontinuous",  genz_discontinuous),
]

# ============================================================================
# Reference solutions via Monte Carlo high-N (truth proxy)
# ============================================================================

def reference_value(func, a, u, d, n_ref=2**22, seed=42):
    """ High-N Monte Carlo reference (truth proxy). """
    rng = np.random.default_rng(seed)
    x = rng.random((n_ref, d))
    return func(x, a, u).mean()

# ============================================================================
# Sobol generator from JK-spec direction numbers (LSB-first column packing)
# ============================================================================

class JKSobol:
    """Generates Sobol points from JK-spec direction numbers."""

    def __init__(self, dim_specs, n_bits=32):
        """
        dim_specs: list of dicts {dim, s, a, m} for d=1..D
        n_bits: number of bits (default 32 → up to 2^32 points)
        """
        self.D = len(dim_specs)
        self.n_bits = n_bits
        # Generate direction numbers V[d, k] for k=0..n_bits-1, dim d
        self.V = np.zeros((self.D, n_bits), dtype=np.uint64)
        for d_idx, spec in enumerate(dim_specs):
            self._compute_direction_numbers(d_idx, spec)

    def _compute_direction_numbers(self, d_idx, spec):
        s = spec['s']
        a = spec['a']
        m = spec['m']  # m_init array of length s
        n_bits = self.n_bits

        # Fill m_k for k = 0..n_bits-1 using Joe-Kuo recurrence
        mk = np.zeros(n_bits, dtype=np.uint64)
        for k in range(s):
            mk[k] = m[k]
        # Decode a as bits a_{s-1}, ..., a_1 (MSB-first)
        a_bits = np.zeros(s + 1, dtype=np.uint64)
        a_bits[s] = 1
        a_bits[0] = 1
        for i in range(1, s):
            a_bits[s - i] = (a >> (s - 1 - i)) & 1

        for k in range(s, n_bits):
            val = mk[k - s] ^ (mk[k - s] << s)
            for i in range(1, s):
                if a_bits[i]:
                    val ^= mk[k - i] << i
            mk[k] = val

        # Direction numbers V[d, k] = m_k << (n_bits - 1 - k)
        for k in range(n_bits):
            self.V[d_idx, k] = mk[k] << (n_bits - 1 - k)

    def generate(self, n):
        """Generate n Sobol points in [0,1)^D."""
        n_bits = self.n_bits
        x_int = np.zeros((n, self.D), dtype=np.uint64)
        # Gray code Sobol generation
        for i in range(1, n):
            # Find lowest 0 bit in i-1 → c
            c = 0
            j = i
            while j & 1:
                c += 1
                j >>= 1
            for d_idx in range(self.D):
                x_int[i, d_idx] = x_int[i - 1, d_idx] ^ self.V[d_idx, c]
        # Normalize to [0, 1)
        return x_int.astype(np.float64) / (1 << n_bits)

# ============================================================================
# Parsers
# ============================================================================

def _parse_line_amichis(line):
    """
    Parse Amichis-format line:  'd= 2: s=1 a=  0 m: 1'
    Returns dict {dim, s, a, m} or None.
    """
    # Strip and replace '=' and ':' with whitespace, then tokenize
    cleaned = line.replace('=', ' ').replace(':', ' ').replace(',', ' ')
    parts = cleaned.split()
    # Expected tokens: ['d', '<dim>', 's', '<s>', 'a', '<a>', 'm', '<m1>', ..., '<ms>']
    try:
        if 'd' not in parts or 's' not in parts or 'a' not in parts or 'm' not in parts:
            return None
        idx_d = parts.index('d')
        idx_s = parts.index('s')
        idx_a = parts.index('a')
        idx_m = parts.index('m')
        d = int(parts[idx_d + 1])
        s = int(parts[idx_s + 1])
        a = int(parts[idx_a + 1])
        m_tokens = parts[idx_m + 1: idx_m + 1 + s]
        if len(m_tokens) != s:
            return None
        m = [int(x) for x in m_tokens]
        return {'dim': d, 's': s, 'a': a, 'm': m}
    except (ValueError, IndexError):
        return None

def _parse_line_jk(line):
    """
    Parse JK-format line:  '2  1  0  1'  or  '2 1 0 1'.
    Returns dict {dim, s, a, m} or None.
    """
    parts = line.split()
    if len(parts) < 4:
        return None
    # Skip header lines like 'd  s  a  m_i'
    if parts[0] == 'd':
        return None
    try:
        d = int(parts[0])
        s = int(parts[1])
        a = int(parts[2])
        m_tokens = parts[3:3 + s]
        if len(m_tokens) != s:
            return None
        m = [int(x) for x in m_tokens]
        return {'dim': d, 's': s, 'a': a, 'm': m}
    except (ValueError, IndexError):
        return None

def parse_dump_file(path):
    """
    Parse any dump file. Auto-detects:
      - Amichis format:  'd= 2: s=1 a=  0 m: 1'
      - JK format:       '2  1  0  1'
    Skips comment lines, headers, and lines that don't parse.
    Returns sorted list of dim_specs.
    """
    dim_specs = [{'dim': 1, 's': 1, 'a': 0, 'm': [1]}]  # van der Corput trivial
    seen_dims = {1}

    with open(path, encoding='utf-8', errors='replace') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#') or line.startswith('>') or line.startswith('|'):
                continue
            # Skip markdown sections
            if line.startswith('-') or line.startswith('*') or line.startswith('`'):
                continue
            # Try Amichis format first (has 'd=' marker)
            spec = None
            if 'd=' in line or 'd =' in line:
                spec = _parse_line_amichis(line)
            else:
                # Try JK format
                spec = _parse_line_jk(line)
            if spec is None:
                continue
            if spec['dim'] in seen_dims:
                continue  # duplicate guard
            seen_dims.add(spec['dim'])
            dim_specs.append(spec)

    dim_specs.sort(key=lambda x: x['dim'])
    return dim_specs

# Backward compat: parse_jk_2008 == parse_dump_file
parse_jk_2008 = parse_dump_file

# ============================================================================
# Benchmark
# ============================================================================

def run_benchmark(combo_path, jk_path, dims=[10, 20, 30],
                  n_points=2**16, n_reps=30, n_ref=2**22, seed_offset=0):
    """Run full benchmark COMBO vs JK."""
    print(f"=" * 78)
    print(f"  Genz Benchmark — COMBO_3027 vs JK_2008")
    print(f"  Functions: 6 Genz standard")
    print(f"  Dimensions: {dims}")
    print(f"  N points per rep: {n_points}")
    print(f"  Reps per (func, dim, sequence): {n_reps}")
    print(f"  Reference truth: MC N={n_ref}")
    print(f"=" * 78)

    print(f"\n[Parse] Loading dumps...")
    combo_specs = parse_dump_file(combo_path)
    jk_specs = parse_jk_2008(jk_path)
    print(f"  COMBO_3027: {len(combo_specs)} dims loaded")
    print(f"  JK_2008:    {len(jk_specs)} dims loaded")

    max_dim = max(dims)
    if len(combo_specs) < max_dim or len(jk_specs) < max_dim:
        print(f"ERROR: need at least {max_dim} dims, have {len(combo_specs)} / {len(jk_specs)}")
        return

    # Build Sobol generators (per max dim needed)
    print(f"\n[Build] Sobol generators...")
    t0 = time.time()
    combo_specs_subset = combo_specs[:max_dim]
    jk_specs_subset = jk_specs[:max_dim]
    sobol_combo = JKSobol(combo_specs_subset, n_bits=20)
    sobol_jk = JKSobol(jk_specs_subset, n_bits=20)
    print(f"  Built in {time.time() - t0:.1f}s")

    # Pre-generate sequences for max dim
    print(f"\n[Generate] Sobol sequences N={n_points}...")
    t0 = time.time()
    pts_combo_full = sobol_combo.generate(n_points)
    pts_jk_full = sobol_jk.generate(n_points)
    print(f"  Generated in {time.time() - t0:.1f}s")

    results = {}  # results[(func_name, dim)] = (combo_err_mean, jk_err_mean, combo_better)

    for dim in dims:
        print(f"\n[Run] dim={dim}")
        pts_combo = pts_combo_full[:, :dim]
        pts_jk = pts_jk_full[:, :dim]

        for func_name, func in GENZ_FUNCTIONS:
            errs_combo = []
            errs_jk = []

            for rep in range(n_reps):
                # Random Genz parameters per replicate
                rng = np.random.default_rng(rep + seed_offset)
                a = rng.random(dim) * 5 + 0.1  # [0.1, 5.1]
                u = rng.random(dim)

                # Reference truth (high-N MC)
                truth = reference_value(func, a, u, dim, n_ref=n_ref, seed=rep + 10000)

                # COMBO estimate
                est_combo = func(pts_combo, a, u).mean()
                err_combo = abs(est_combo - truth)
                errs_combo.append(err_combo)

                # JK estimate
                est_jk = func(pts_jk, a, u).mean()
                err_jk = abs(est_jk - truth)
                errs_jk.append(err_jk)

            err_combo_mean = np.mean(errs_combo)
            err_jk_mean = np.mean(errs_jk)
            combo_better = err_combo_mean < err_jk_mean
            results[(func_name, dim)] = (err_combo_mean, err_jk_mean, combo_better)

            ratio = err_combo_mean / err_jk_mean if err_jk_mean > 0 else float('inf')
            marker = "✓ COMBO" if combo_better else "✗ JK   "
            print(f"  {func_name:18s} d={dim:2d}: "
                  f"COMBO={err_combo_mean:.3e}  JK={err_jk_mean:.3e}  "
                  f"ratio={ratio:.3f}  {marker}")

    # Summary table
    print(f"\n" + "=" * 78)
    print(f"  SUMMARY")
    print(f"=" * 78)
    print(f"\n  | Function          | dim |  COMBO error  |  JK error    | Ratio  | Winner |")
    print(f"  |-------------------|-----|---------------|--------------|--------|--------|")
    n_combo_wins = 0
    n_total = 0
    for func_name, _ in GENZ_FUNCTIONS:
        for dim in dims:
            err_c, err_j, c_better = results[(func_name, dim)]
            ratio = err_c / err_j if err_j > 0 else float('inf')
            winner = "COMBO" if c_better else "JK"
            if c_better:
                n_combo_wins += 1
            n_total += 1
            print(f"  | {func_name:17s} | {dim:3d} | {err_c:.3e}    | {err_j:.3e}   | {ratio:.3f}  | {winner:6s} |")

    print(f"\n  COMBO_3027 wins: {n_combo_wins} / {n_total} ({100.0 * n_combo_wins / n_total:.1f}%)")

    # F75 dual-form verdict
    print(f"\n" + "=" * 78)
    print(f"  F75 DUAL-FORM TEST")
    print(f"=" * 78)
    if n_combo_wins >= 0.66 * n_total:  # ≥ 2/3 wins
        print(f"  COMBO_3027 dominates JK_2008 in {n_combo_wins}/{n_total} (≥66%).")
        print(f"  → F_2 audit improvement (3196 → 3027, -5.3%) DOES translate to Genz error reduction.")
        print(f"  → Practitioner-relevance RATIFIED. Record peer-review-defensible.")
    elif n_combo_wins >= 0.5 * n_total:  # 50-66% wins
        print(f"  COMBO_3027 marginal advantage in {n_combo_wins}/{n_total} (50-66%).")
        print(f"  → F_2 audit improvement translates partially to Genz error reduction.")
        print(f"  → Practitioner-relevance MIXED. Defensible with caveat.")
    else:
        print(f"  COMBO_3027 wins only {n_combo_wins}/{n_total} (<50%).")
        print(f"  → F_2 audit improvement DOES NOT translate to Genz error reduction.")
        print(f"  → F75 v52 dual-form RATIFIED: F_2 audit and F_2 Genz error NOT monotonically related.")
        print(f"  → Practitioner-relevance NOT RATIFIED. Record audit-metric only.")
    print(f"=" * 78)

    return results


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(f"Usage: python3 {sys.argv[0]} TOGORDO_COMBO_v1_RECORD.txt new-joe-kuo-6.21201")
        sys.exit(1)

    combo_path = sys.argv[1]
    jk_path = sys.argv[2]

    if not Path(combo_path).exists():
        print(f"ERROR: {combo_path} not found")
        sys.exit(2)
    if not Path(jk_path).exists():
        print(f"ERROR: {jk_path} not found")
        sys.exit(3)

    t0 = time.time()
    run_benchmark(combo_path, jk_path)
    print(f"\nTotal runtime: {time.time() - t0:.1f}s")
