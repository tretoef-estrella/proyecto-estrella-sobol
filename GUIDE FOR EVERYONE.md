# GUIDE_FOR_EVERYONE

**A plain-language tour of Proyecto Estrella.** No mathematical prerequisites. If you have already read `MAIN_DISCOVERIES.md` and want detail, skip this file. If you arrived from a link and are wondering "what is this and why should I care", start here.

---

## What kind of thing is this?

This is a piece of mathematical and software work that produces a better version of a tool used by many software libraries that you have probably already used indirectly: the **Sobol sequence**.

A Sobol sequence is a clever way to generate points inside a many-dimensional cube so that the points cover the cube **evenly** — much more evenly than purely random points. They were invented by I. M. Sobol' in 1967. Today they are used to:

- Price financial options (Monte Carlo simulation under the Black-Scholes model and many extensions).
- Calibrate machine-learning models (hyperparameter search).
- Render realistic 3D images (path tracing in computer graphics).
- Solve high-dimensional integrals in physics, chemistry, and engineering.
- Run experiments in fields like Bayesian inference, robotics planning, and reinforcement learning.

If you use `scipy.stats.qmc.Sobol` in Python, or BoTorch in PyTorch, or QuantLib in C++, or any of dozens of other libraries, you are using a Sobol sequence. The quality of those sequences depends on a set of numbers called **direction numbers**. Better direction numbers → more even point coverage → smaller errors in your simulation.

The de facto worldwide standard for direction numbers since 2008 has been the construction by **Stephen Joe and Frances Y. Kuo**, two researchers at the University of New South Wales. Their direction numbers are what `scipy` ships with.

---

## What does this project do?

Two things.

### Thing one — a better set of direction numbers (for some uses)

Joe and Kuo did not just publish their direction numbers — they also published the criterion by which they had judged them. That criterion is a number called the **audit value** (Joe-Kuo Table 3.6). For their direction numbers, the audit value is 3196. Lower is better.

For eighteen years, nobody published a set of direction numbers with a lower audit value, verified, reproducible, and on exactly the same criterion as the original paper.

**This project did.** The set called `COMBO_3027` (in this repository, file `TOGORDO_COMBO_v1_RECORD.txt`) has an audit value of 3027 — about 5.3 % lower. You can verify this yourself in under a second on any laptop by running the Python script in this repository.

### Thing two — the surprising part

Here is the catch, and it is itself a useful discovery:

A lower audit value does **not** automatically mean smaller errors in your real simulations.

We checked this by running the new direction numbers and the original Joe-Kuo direction numbers through a standard battery of 18 test integrations (the **Genz suite**: six families of test functions, each in three dimensions). On these 18 tests, the new set wins 7 and loses 11.

That is not a uniform improvement. It is **better on some kinds of problem and worse on others**.

While discovering this, we also found a *second* set of direction numbers (in `togordo17_seed3_dump.txt`, audit value 3095) which has a *different* pattern of wins and losses — it wins on Gaussian-type integrands in high dimensions, where the first set loses.

That is the genuinely interesting finding: **the space of "good" Sobol direction numbers is not organised around a single optimum**. There are multiple distinct sets that are all better than Joe-Kuo (2008) on the original criterion, and each of them is good at a different kind of integration problem. A practitioner should choose direction numbers based on the kind of integrand they have, not by the audit number alone.

This refutes a quiet assumption in the QMC literature that audit-metric optimisation is a single-target problem.

---

## What should I do with this?

**If you write Monte Carlo simulations**: the three direction-number files in this repository (`TOGORDO_COMBO_v1_RECORD.txt`, `togordo17_seed3_dump.txt`, `new-joe-kuo-6.21201`) are plain text and easy to load. Pick by integrand type:

- Oscillatory or product-peak integrands, dimension ≥ 20 → `COMBO_3027`.
- Gaussian-type integrands, dimension ≥ 20 → `E17_seed4_3095`.
- Smooth integrands or low dimension → Joe-Kuo (2008) standard, which is the scipy default.

A worked example loading any of the three sets into `scipy.stats.qmc.Sobol` is below.

**If you are a researcher in QMC, quasi-Monte Carlo, computational finance, or computer graphics**: please cite this work if you use it, replicate it if you doubt it, and extend it if you can. The full paper (`PAPER_TERMINAL.md`), the methodology primer (`METHODOLOGY.md`), the operational arsenal (`COJONES_SABIOS_TERMINAL.md`), and the four key contributions in compressed form (`MAIN_DISCOVERIES.md`) are all in this repository. The C++ engines, the Python verifier, the Genz benchmark, the full campaign logs, and every dump are included so that any disagreement can be settled against the files.

**If you are a software-library maintainer**: the records in this repository are licensed MIT. Practitioner-relevant guidance is in `MAIN_DISCOVERIES.md`.

**If you are simply curious**: read on. The rest of this guide explains how all this works without mathematical notation.

---

## How did the project produce these results?

The recipe was:

1. **Cache trick**. A naive evaluation of the audit value scales quadratically in the number of dimensions. We built an incremental cache that updates the audit value linearly when a single direction-number slot is changed, with a rollback structure so failed trial moves cost almost nothing. Measured speedup: 65× on a single laptop core. This makes everything else affordable on a single Apple M2.

2. **Search engines**. Six independent C++ programmes (named, in the project's tradition, `TOGORDOEL...` followed by absurd Spanish phrases — the names are a discipline trick: silly names cannot survive mediocre engines) each implementing a different search strategy. Each engine combines:
   - An *exhaustive greedy* phase that tries every primitive polynomial against every dimension and keeps the best initial direction numbers it can find.
   - A *simulated annealing* phase with five parallel temperature chains and Metropolis-Hastings swaps between them.
   - A *polish* phase that systematically improves the best state by trying every local single-bit move.

3. **A productivity test before commitment**. Before letting any move type consume the simulated-annealing time budget, we sample 1000 random attempts of that move from the current state and measure how often they actually improve the audit value. If the lower quartile of the resulting deltas is ≥ 0, the move is *ornamental* from this state and gets zero mass. This is the antidote to relying on lore from previous projects.

4. **Quadruple verification of every record**. Before declaring a number, four independent checks must pass: rebuild from scratch and compare; serialise to a text dump, parse back, rebuild, compare; check every structural invariant; run a separate Python implementation on the dump. Any record that does not pass all four is not a record.

5. **Brutal honesty about negative results**. A fresh-eye audit performed in mid-project (a separate Claude instance, given the role of forensic auditor against the archives) caught a critical bug in one of the engines: a move type that had been *declared* in the code but actually *aliased* to a different move. The result was retracted, the engine was re-built with the move implemented as a genuinely separate move (Engine 18), and the move turned out to be unproductive in this regime — also a useful finding. The project's habit of inviting fresh-eye audits after major changes (the "D235" rule in our terminology) is documented as part of the methodology.

6. **A clean stop**. After six engines and a Genz benchmark with mixed results, the architect decided the project had produced its substantive contributions and closed it. Subsequent findings, if any, will be written into `NEW_DISCOVERIES.md` in this same repository.

---

## Worked example: using the records in Python

The three direction-number sets in this repository are plain text. Here is how to load any of them into `scipy.stats.qmc.Sobol` and use it.

```python
import numpy as np
from scipy.stats import qmc

def parse_direction_numbers(path):
    """
    Parses any of the three direction-number files in this repository:
      - new-joe-kuo-6.21201          (Joe-Kuo 2008 baseline)
      - TOGORDO_COMBO_v1_RECORD.txt  (this project's record, audit = 3027)
      - togordo17_seed3_dump.txt     (alternative basin,    audit = 3095)
    Returns a list of (s, a, m_init) per dimension starting at dim 2.
    """
    specs = []
    with open(path, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#') or line.startswith('d '):
                continue
            # Both formats: leading "d=" or just plain "d s a m_1 ..."
            cleaned = line.replace('=', ' ').replace(':', ' ').replace(',', ' ')
            parts = cleaned.split()
            try:
                if parts[0] == 'd':
                    d = int(parts[1]); s = int(parts[3]); a = int(parts[5])
                    m = [int(x) for x in parts[7:7+s]]
                else:
                    d = int(parts[0]); s = int(parts[1]); a = int(parts[2])
                    m = [int(x) for x in parts[3:3+s]]
                if len(m) != s:
                    continue
                specs.append((d, s, a, m))
            except (ValueError, IndexError):
                continue
    return specs

# Load the project record
specs = parse_direction_numbers('TOGORDO_COMBO_v1_RECORD.txt')
print(f"Loaded {len(specs)} dimensions, first row: {specs[0]}")

# For use in scipy: build the V matrices the way scipy.stats.qmc.Sobol expects.
# See scipy.stats.qmc documentation or the file verify_F2_independent.py in
# this repository for a complete reference implementation of the Joe-Kuo
# recurrence (function _compute_direction_numbers).
```

For a complete, self-contained example that builds and uses a Sobol generator from any of the three files, see `verify_F2_independent.py` in this repository — it parses the file, builds the direction matrices, generates the points, and computes the audit value, and is heavily commented.

---

## Acknowledgements and people

The project was conducted by Rafael Amichis Luengo, a psychologist by formal training who learnt the relevant mathematics autodidactically over several months. The code was written in collaboration with Claude (Anthropic), in a working pattern with three distinct roles: Architect (Rafa, who arbitrated all decisions and ran all jobs on his Apple M2), Constructor (Claude, who wrote C++ engine code and Python scripts), and Auditor (a separate Claude instance, brought in fresh at key transitions to audit the work against the archives). The triple-role discipline and the practice of inviting fresh-eye audits after major changes are themselves documented as methodological contributions.

The work builds on Joe & Kuo (2008), Sobol' (1967), Niederreiter (1987-1988), Owen (1995), Faure & Lemieux (2016), and the standard Genz test suite.

---

## License and citation

This project is released under the MIT license. If you use it in a paper, please cite it; a machine-readable citation file (`CITATION.cff`) is included.

---

*Madrid, May 2026 — Rafael Amichis Luengo.*
