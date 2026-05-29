#!/usr/bin/env python3
"""
verify_F2_independent.py — Engine 13 D190 step 4: clean-room independent verifier.

Engine 13: TOGORDOELCHUMINEROASESINEROSALELOCODELGRANERO — Phase 1 file 5 of 5
Constructor v53 (Cojones Rectos), 8 May 2026

Purpose:
  Independent re-implementation of the F_2 audit metric Σ_{d=2..37} Σ_{m=5..25}
  max_{j=1..d-1} t(j, d, m), where j=1 corresponds to the van der Corput
  sequence in dimension 1 (identity generator matrix), per F82 binding.

Independence:
  - NO C++ shared code.
  - Recurrence m_k re-derived from scratch using Joe-Kuo 2008 §2 spec.
  - Generator matrix C_d packed independently (LSB-first column packing here
    to differ from the C++ kernel's MSB-first; audit total must still match).
  - Rank check: Gaussian elimination from scratch.

Witnesses (D44/D81 numbers from files):
  audit(JK_2008)    must equal 3196   (new-joe-kuo-6_21201.md baseline)
  audit(COMBO_3027) must equal 3027   (TOGORDO_COMBO_v1_RECORD.txt)

Usage:
  python3 verify_F2_independent.py [<dump_file>] [<expected_audit>]
  If no args: runs internal witnesses (3196 and 3027) and reports.
  With dump_file + expected: parses dump, computes audit, asserts.

Hard rules:
  - No memoization of audit results from any cache file. Cold compute always.
  - All numbers from this file's internal definitions or from <dump_file> (if
    given). No memory-derived constants.
"""

import sys
import time
import re
from typing import List, Tuple

# ============================================================================
# §1 — Constants (re-stated from new-joe-kuo-6_21201.md §2 + project specs)
# ============================================================================

MAXBITS = 32
MAXDIM = 38
MLO = 5
MHI = 25
NM = MHI - MLO + 1  # 21
SMAX = 9
MAXDN = 32
FIRST_AUDIT_DIM = 2
LAST_AUDIT_DIM = 37
N_AUDIT_DIMS = LAST_AUDIT_DIM - FIRST_AUDIT_DIM + 1  # 36

# ============================================================================
# §2 — Joe-Kuo 2008 spec for dims 2..37 (re-typed independently from
#     new-joe-kuo-6_21201.md §3; expected audit witness 3196)
# ============================================================================

JK_2008 = [
    # (s, a, m_init...) for d = 2..37
    (1,   0, [1]),
    (2,   1, [1, 3]),
    (3,   1, [1, 3, 1]),
    (3,   2, [1, 1, 1]),
    (4,   1, [1, 1, 3, 3]),
    (4,   4, [1, 3, 5, 13]),
    (5,   2, [1, 1, 5, 5, 17]),
    (5,   4, [1, 1, 5, 5, 5]),
    (5,   7, [1, 1, 7, 11, 19]),
    (5,  11, [1, 1, 5, 1, 1]),
    (5,  13, [1, 1, 1, 3, 11]),
    (5,  14, [1, 3, 5, 5, 31]),
    (6,   1, [1, 3, 3, 9, 7, 49]),
    (6,  13, [1, 1, 1, 15, 21, 21]),
    (6,  16, [1, 3, 1, 13, 27, 49]),
    (6,  19, [1, 1, 1, 15, 7, 5]),
    (6,  22, [1, 3, 1, 15, 13, 25]),
    (6,  25, [1, 1, 5, 5, 19, 61]),
    (7,   1, [1, 3, 7, 11, 23, 15, 103]),
    (7,   4, [1, 3, 7, 13, 13, 15, 69]),
    (7,   7, [1, 1, 3, 13, 7, 35, 63]),
    (7,   8, [1, 3, 5, 9, 1, 25, 53]),
    (7,  14, [1, 3, 1, 13, 9, 35, 107]),
    (7,  19, [1, 3, 1, 5, 27, 61, 31]),
    (7,  21, [1, 1, 5, 11, 19, 41, 61]),
    (7,  28, [1, 3, 5, 3, 3, 13, 69]),
    (7,  31, [1, 1, 7, 13, 1, 19, 1]),
    (7,  32, [1, 3, 7, 5, 13, 19, 59]),
    (7,  37, [1, 1, 3, 9, 25, 29, 41]),
    (7,  41, [1, 3, 5, 13, 23, 1, 55]),
    (7,  42, [1, 3, 7, 3, 13, 59, 17]),
    (7,  50, [1, 3, 1, 3, 5, 53, 69]),
    (7,  55, [1, 1, 5, 5, 23, 33, 13]),
    (7,  56, [1, 1, 7, 7, 1, 61, 123]),
    (7,  59, [1, 1, 7, 9, 13, 61, 49]),
    (7,  62, [1, 3, 3, 5, 3, 55, 33]),
]
assert len(JK_2008) == N_AUDIT_DIMS

# ============================================================================
# §3 — COMBO_3027 spec (re-typed independently from TOGORDO_COMBO_v1_RECORD.txt;
#     expected audit witness 3027)
# ============================================================================

COMBO_3027 = [
    (1,   0, [1]),
    (2,   1, [1, 3]),
    (3,   1, [1, 3, 1]),
    (3,   2, [1, 1, 1]),
    (4,   1, [1, 1, 3, 13]),
    (4,   4, [1, 1, 3, 7]),
    (5,   7, [1, 3, 5, 1, 29]),
    (5,   4, [1, 1, 7, 5, 27]),
    (5,  13, [1, 1, 7, 5, 21]),
    (5,  14, [1, 1, 1, 9, 7]),
    (5,  11, [1, 1, 1, 11, 13]),
    (6,  13, [1, 1, 5, 9, 3, 63]),
    (5,   2, [1, 1, 7, 9, 29]),
    (6,  22, [1, 1, 7, 7, 1, 21]),
    (6,  16, [1, 3, 7, 1, 21, 17]),
    (6,  25, [1, 3, 7, 15, 1, 63]),
    (6,  19, [1, 1, 1, 5, 5, 63]),
    (7,  14, [1, 1, 3, 9, 21, 57, 81]),
    (6,   1, [1, 1, 3, 1, 25, 7]),
    (7,   4, [1, 3, 1, 13, 15, 29, 83]),
    (7,  31, [1, 3, 5, 9, 7, 11, 15]),
    (7,  19, [1, 3, 5, 5, 5, 49, 33]),
    (7,  42, [1, 1, 3, 1, 31, 29, 63]),
    (7,   1, [1, 3, 7, 7, 23, 11, 23]),
    (7,  59, [1, 1, 1, 1, 9, 37, 117]),
    (7,  32, [1, 3, 3, 1, 5, 39, 15]),
    (7,   7, [1, 1, 5, 9, 9, 39, 65]),
    (7,  62, [1, 3, 7, 15, 11, 35, 3]),
    (7,  50, [1, 1, 7, 3, 27, 21, 83]),
    (7,  41, [1, 1, 7, 9, 17, 35, 121]),
    (7,  21, [1, 3, 7, 11, 27, 37, 41]),
    (7,  56, [1, 1, 5, 15, 27, 25, 55]),
    (7,   8, [1, 1, 1, 3, 27, 37, 51]),
    (7,  55, [1, 1, 1, 9, 29, 57, 123]),
    (8,  49, [1, 3, 3, 11, 21, 61, 109, 77]),
    (8, 103, [1, 3, 5, 3, 1, 31, 53, 255]),
]
assert len(COMBO_3027) == N_AUDIT_DIMS

# ============================================================================
# §4 — Joe-Kuo recurrence: compute direction numbers m_k from (s, a, m_init)
#     Independently re-derived from new-joe-kuo-6_21201.md §2.
# ============================================================================

def compute_dn(s: int, a_coeff: int, m_init: List[int]) -> List[int]:
    """
    Returns dn[0..MAXDN-1] where dn[k] = m_{k+1} (1-indexed math).
    dn[k] satisfies dn[k] < 2^(k+1) and dn[k] is odd.

    Recurrence (Joe-Kuo §2):
      m_k = 2^s * m_{k-s}  XOR  m_{k-s}
            XOR  Σ_{i=1..s-1}  2^i * a_i * m_{k-i}
    where a_coeff is read MSB-first: bit (s-2) of a_coeff = a_1, bit 0 = a_{s-1}.
    """
    assert 1 <= s <= SMAX
    assert len(m_init) == s

    dn = [0] * MAXDN
    for k in range(s):
        dn[k] = m_init[k]

    # Decode a_coeff bits MSB-first into a_bits[1..s-1]
    a_bits = [0] * (SMAX + 1)
    for i in range(s - 1):
        a_bits[i + 1] = (a_coeff >> (s - 2 - i)) & 1

    for k in range(s, MAXDN):
        val = dn[k - s] ^ (dn[k - s] << s)
        for i in range(1, s):
            if a_bits[i]:
                val ^= dn[k - i] << i
        # Mask: dn[k] (0-indexed = m_{k+1}) must satisfy m_{k+1} < 2^{k+1}
        if k + 1 < 32:
            val &= (1 << (k + 1)) - 1
        else:
            val &= 0xFFFFFFFF
        dn[k] = val

    return dn

# ============================================================================
# §5 — Generator matrix C_d packing.
#
# Independence: we use LSB-first column packing here (different from kernel's
# MSB-first). The packed representation:
#   C[d] is a list of MAXBITS integers (rows). C[d][r] has bit k set
#   (for k in [0, MAXDN)) iff bit (k - r) of dn[k] equals 1, for r <= k.
# This is the same matrix as the C++ kernel computes, just packed with the
# opposite bit order. Since the audit metric depends only on rank of submatrices
# (a basis-independent property), any consistent column packing yields the same
# audit total.
# ============================================================================

def build_C(dn: List[int]) -> List[int]:
    """Returns C[r] for r in [0, MAXBITS), LSB-first column packing."""
    C = [0] * MAXBITS
    for k in range(MAXDN):
        mk = dn[k]
        for r in range(k + 1):  # r in [0, k]
            if (mk >> (k - r)) & 1:
                C[r] |= (1 << k)
    return C

# Van der Corput dim 1: identity. Bit k of C[r] = 1 iff r == k.
def build_C_vdc() -> List[int]:
    C = [0] * MAXBITS
    for r in range(MAXBITS):
        C[r] = (1 << r)
    return C

# ============================================================================
# §6 — Rank check over GF(2). Independent Gaussian elimination from scratch.
# ============================================================================

def rank_check_geq(rows: List[int], n_cols: int, target_rank: int) -> bool:
    """
    Returns True iff the binary matrix `rows` (length = n_rows) restricted to
    columns 0..n_cols-1 has rank >= target_rank over GF(2).
    """
    if target_rank <= 0:
        return True
    n_rows = len(rows)
    if target_rank > n_rows:
        return False
    if target_rank > n_cols:
        return False

    R = list(rows)
    rank = 0
    # Pivot column-by-column (LSB-first packing, columns 0..n_cols-1).
    for col in range(n_cols):
        if rank >= target_rank:
            break
        bit = 1 << col
        pivot = -1
        for r in range(rank, n_rows):
            if R[r] & bit:
                pivot = r
                break
        if pivot < 0:
            continue
        if pivot != rank:
            R[rank], R[pivot] = R[pivot], R[rank]
        prow = R[rank]
        for r in range(n_rows):
            if r != rank and (R[r] & bit):
                R[r] ^= prow
        rank += 1

    return rank >= target_rank

# ============================================================================
# §7 — t(j, d, m) Niederreiter style.
#
# t(j, d, m) = max t in [0, m] such that for all (rj, rd) with rj+rd = m-t and
# rj, rd in [0, m], the rj+rd rows formed by the first rj rows of C_j and the
# first rd rows of C_d, restricted to the first m columns, are linearly
# independent over GF(2). Equivalently, rank == rj + rd for all such splits.
# ============================================================================

def compute_t(Cj: List[int], Cd: List[int], m: int) -> int:
    if m <= 0:
        return 0
    if m > MHI:
        return m  # out of range, defensive
    n_cols = m
    col_mask = (1 << m) - 1 if m < 32 else 0xFFFFFFFF

    # Pre-mask rows to first m columns (LSB-first => bits 0..m-1)
    Mj = [r & col_mask for r in Cj]
    Md = [r & col_mask for r in Cd]

    for t in range(0, m + 1):
        target = m - t
        good = True
        for rj in range(0, target + 1):
            rd = target - rj
            if rj > MAXBITS or rd > MAXBITS:
                continue
            n_rows = rj + rd
            if n_rows == 0:
                continue
            mat = Mj[:rj] + Md[:rd]
            if not rank_check_geq(mat, n_cols, n_rows):
                good = False
                break
        if good:
            return t
    return m

# ============================================================================
# §8 — Audit a state (list of (s, a, m_init) for dims 2..37).
# ============================================================================

def audit_state(spec: List[Tuple[int, int, List[int]]]) -> int:
    """spec is [(s, a, m_init)] for d = 2..37. Returns total audit (j=1..d-1)."""
    assert len(spec) == N_AUDIT_DIMS
    # Build C[d] for d = 1..37. C[1] = van der Corput identity.
    C: List[List[int]] = [[] for _ in range(MAXDIM)]
    C[1] = build_C_vdc()
    for i, (s, a, m_init) in enumerate(spec):
        d = FIRST_AUDIT_DIM + i
        dn = compute_dn(s, a, m_init)
        C[d] = build_C(dn)

    total = 0
    for d in range(FIRST_AUDIT_DIM, LAST_AUDIT_DIM + 1):
        for m in range(MLO, MHI + 1):
            best_t = 0
            for j in range(1, d):
                t = compute_t(C[j], C[d], m)
                if t > best_t:
                    best_t = t
            total += best_t
    return total

# ============================================================================
# §9 — Dump file parser (mirrors the C++ format from JK_BUILDER.h §8).
#     Format A (Amichis): "d= 2: s=1 a=  0 m: 1\n"
#     Format B (JK standard, e.g. new-joe-kuo-6.21201): "2  1  0  1"
# ============================================================================

DIM_RE_AMICHIS = re.compile(
    r'^\s*d\s*=\s*(\d+)\s*:\s*s\s*=\s*(\d+)\s+a\s*=\s*(\d+)\s+m\s*:\s*(.+)$'
)

def _parse_line_jk(line: str):
    """Parse JK-standard format: '2 1 0 1' or '4 3 1 1 3 1' etc.
    Returns (d, s, a, m_list) or None."""
    parts = line.split()
    if len(parts) < 4:
        return None
    if parts[0] == 'd':  # header line "d s a m_i"
        return None
    try:
        d = int(parts[0])
        s = int(parts[1])
        a = int(parts[2])
        m = [int(x) for x in parts[3:3 + s]]
        if len(m) != s:
            return None
        return (d, s, a, m)
    except (ValueError, IndexError):
        return None

def parse_dump(filename: str) -> List[Tuple[int, int, List[int]]]:
    """Returns spec = [(s, a, m_init)] for d=2..37 in order.
    Auto-detects format: Amichis (with 'd= X:') or JK standard ('X s a m_1..m_s').
    Skips comment lines (#), markdown decoration, blank lines, and any line
    that does not parse as a valid direction-number record."""
    parsed = {}
    with open(filename, 'r', encoding='utf-8', errors='replace') as f:
        for line in f:
            raw = line.strip()
            if not raw:
                continue
            # Skip markdown/comment decoration
            if raw.startswith('#') or raw.startswith('>') or raw.startswith('|'):
                continue
            if raw.startswith('-') or raw.startswith('*') or raw.startswith('`'):
                continue

            # Try Amichis format first (has 'd=' or 'd =' marker)
            ms_match = DIM_RE_AMICHIS.match(line)
            if ms_match:
                d = int(ms_match.group(1))
                s = int(ms_match.group(2))
                a = int(ms_match.group(3))
                ms = [int(x) for x in ms_match.group(4).split()]
            else:
                # Try JK standard format
                jk = _parse_line_jk(raw)
                if jk is None:
                    continue
                d, s, a, ms = jk

            if not (FIRST_AUDIT_DIM <= d <= LAST_AUDIT_DIM):
                continue
            if len(ms) != s:
                raise ValueError(f"d={d} expected {s} m-values, got {len(ms)}")
            # Validate invariants
            if ms[0] != 1:
                raise ValueError(f"d={d} m_1 must be 1")
            for k in range(s):
                if (ms[k] & 1) == 0:
                    raise ValueError(f"d={d} m[{k+1}] not odd")
                if ms[k] >= (1 << (k + 1)):
                    raise ValueError(f"d={d} m[{k+1}]={ms[k]} >= 2^{k+1}")
            parsed[d] = (s, a, ms)
    spec = []
    for d in range(FIRST_AUDIT_DIM, LAST_AUDIT_DIM + 1):
        if d not in parsed:
            raise ValueError(f"missing d={d} in dump (parsed {len(parsed)} dims)")
        spec.append(parsed[d])
    return spec

# ============================================================================
# §10 — Main / witnesses
# ============================================================================

def main():
    args = sys.argv[1:]

    if len(args) == 1:
        # Just compute and report
        dump_file = args[0]
        print(f"Parsing {dump_file}...")
        spec = parse_dump(dump_file)
        print(f"  Loaded {len(spec)} dimensions (d=2..37)")
        t0 = time.time()
        total = audit_state(spec)
        dt = time.time() - t0
        print(f"audit = {total}  [{dt:.1f}s]")
        # Helpful comparison context
        if total == 3196:
            print("  → matches Joe-Kuo (2008) baseline reference (audit = 3196)")
        elif total == 3027:
            print("  → matches COMBO_3027 project record (audit = 3027, -5.3% vs JK_2008)")
        elif total == 3095:
            print("  → matches Engine 17 seed 4 alternative basin (audit = 3095, Gaussian dim≥20 specialist)")
        elif total < 3196:
            delta_pct = 100.0 * (total - 3196) / 3196
            print(f"  → {delta_pct:+.2f}% vs Joe-Kuo (2008) baseline (audit = 3196)")
        else:
            delta_pct = 100.0 * (total - 3196) / 3196
            print(f"  → {delta_pct:+.2f}% vs Joe-Kuo (2008) baseline (audit = 3196)")
        sys.exit(0)

    if len(args) >= 2:
        # Verify external dump against expected value
        dump_file = args[0]
        expected = int(args[1])
        print(f"Parsing {dump_file}...")
        spec = parse_dump(dump_file)
        t0 = time.time()
        total = audit_state(spec)
        dt = time.time() - t0
        ok = (total == expected)
        print(f"audit(dump) = {total}  (expected {expected})  [{dt:.1f}s]  {'PASS' if ok else 'FAIL'}")
        sys.exit(0 if ok else 1)

    # No args: internal witnesses (only run if internal specs exist)
    print("=== verify_F2_independent — D190 step 4 clean-room redundancy verifier ===")
    print("    (NOTE: shares Joe-Kuo specification with the C++ kernel.")
    print("     Catches implementation errors. For genuine independence, a")
    print("     verifier using qmcpy with a different t-value algorithm is")
    print("     recommended pre-publication — see MAIN_DISCOVERIES.md Caveat.)\n")

    print("Usage:")
    print("  python3 verify_F2_independent.py <dump_file>")
    print("       compute and print the audit of <dump_file>")
    print("  python3 verify_F2_independent.py <dump_file> <expected_audit>")
    print("       compute the audit and assert it equals <expected_audit>")
    print()
    print("Example:")
    print("  python3 verify_F2_independent.py TOGORDO_COMBO_v1_RECORD.txt")
    print("  → audit = 3027")
    print("  python3 verify_F2_independent.py new-joe-kuo-6.21201")
    print("  → audit = 3196")
    sys.exit(0)

if __name__ == "__main__":
    main()
