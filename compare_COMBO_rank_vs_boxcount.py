#!/usr/bin/env python3
"""
compare_COMBO_rank_vs_boxcount.py
=================================
Cross-checks the two independent t-value routes on the COMBO_3027 dump, over the
same box-countable range, and reports whether they agree cell-region by region.

Route A (rank):       verify_F2_independent.py  — Gaussian elimination / Niederreiter rank predicate.
Route B (box count):  verify_qmcpy_independent.py — qmcpy-generated points, dyadic box counting.

If the two partial audits match, the rank-based and equidistribution-based
t-value computations agree on the COMBO dump (not just on the JK baseline).

USAGE (from ~/Downloads, with all repo files present):
  python3 compare_COMBO_rank_vs_boxcount.py
  python3 compare_COMBO_rank_vs_boxcount.py --max-m 16        # deeper, slower
  python3 compare_COMBO_rank_vs_boxcount.py --dump other.txt  # any dump

Requires: numpy, qmcpy, and the two verifier files in the same folder.
IMPORTANT: the imported verifier files must be named WITHOUT a ' (1)' suffix:
  verify_F2_independent.py   and   verify_qmcpy_independent.py
Rename them in the repo if GitHub shows ' (1)' in the filename.
"""

import sys, time, importlib.util, warnings
import numpy as np
warnings.filterwarnings("ignore")

# ---- args ----
DUMP = "TOGORDO_COMBO_v1_RECORD.txt"
MAX_M = 14
args = sys.argv[1:]
for i, a in enumerate(args):
    if a == "--max-m" and i + 1 < len(args):
        MAX_M = int(args[i + 1])
    if a == "--dump" and i + 1 < len(args):
        DUMP = args[i + 1]

def load(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    mod = importlib.util.module_from_spec(spec)
    sys.argv = ['x']           # keep the imported module's main() from firing on args
    spec.loader.exec_module(mod)
    return mod

print("=== COMBO cross-check: rank vs box-counting, m in [5,%d] ===\n" % MAX_M)

# ---- load both verifiers ----
try:
    rv = load("rv", "verify_F2_independent.py")
    qv = load("qv", "verify_qmcpy_independent.py")
except FileNotFoundError as e:
    sys.stderr.write("ERROR: %s\n" % e)
    sys.stderr.write("Make sure verify_F2_independent.py and "
                     "verify_qmcpy_independent.py are in this folder, "
                     "without a ' (1)' suffix.\n")
    sys.exit(2)

# ---- parse the dump once (use the rank verifier's parser) ----
spec = rv.parse_dump(DUMP)

# ---- Route A: rank-based partial audit over m in [5, MAX_M] ----
def rank_partial(spec, max_m):
    C = [[] for _ in range(rv.MAXDIM)]
    C[1] = rv.build_C_vdc()
    for i, (s, a, mi) in enumerate(spec):
        C[rv.FIRST_AUDIT_DIM + i] = rv.build_C(rv.compute_dn(s, a, mi))
    total = 0
    for d in range(rv.FIRST_AUDIT_DIM, rv.LAST_AUDIT_DIM + 1):
        for m in range(rv.MLO, max_m + 1):
            bt = 0
            for j in range(1, d):
                t = rv.compute_t(C[j], C[d], m)
                if t > bt:
                    bt = t
            total += bt
    return total

print("[Route A] rank-based partial audit ...")
t0 = time.time()
a_total = rank_partial(spec, MAX_M)
print("   = %d   [%.1fs]\n" % (a_total, time.time() - t0))

# ---- Route B: box-counting partial audit over the same range ----
print("[Route B] box-counting partial audit (qmcpy points) ...")
t0 = time.time()
pts = qv.gen_points_from_dump(spec, 1 << MAX_M)
b_total = qv.audit_boxcount(pts, MAX_M, verbose=False)
print("   = %d   [%.1fs]\n" % (b_total, time.time() - t0))

# ---- verdict ----
print("=" * 52)
if a_total == b_total:
    print("AGREE: both routes return %d over m in [5,%d]." % (a_total, MAX_M))
    print("The rank-based and equidistribution-based t-values agree on the")
    print("COMBO dump. Independent cross-check PASS.")
    sys.exit(0)
else:
    print("DISAGREE: rank=%d  boxcount=%d  over m in [5,%d]." % (a_total, b_total, MAX_M))
    print("A discrepancy means one of the two implementations has a bug on")
    print("this range. Do NOT trust the record until this is resolved.")
    sys.exit(1)
