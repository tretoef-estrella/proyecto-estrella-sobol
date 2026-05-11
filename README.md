# Proyecto Estrella · Sobol Campaign

**First ratified improvement over Joe-Kuo (2008) Table 3.6 audit metric in 18 years, with empirical refutation of the single-basin-optimum assumption in F₂ Sobol direction numbers.**

**Author**: Rafael Amichis Luengo (Madrid) · **Date**: May 2026 · **Status**: closed project, open dataset · **License**: MIT

---

## TL;DR

Sobol' quasi-Monte Carlo sequences are the most widely deployed low-discrepancy sequences in industry: scipy, BoTorch, MATLAB QRNG, NAG, QuantLib, computer graphics, machine learning, computational finance. Their quality depends on a set of **direction numbers**, and the de facto standard for the last 18 years has been the Joe & Kuo (2008) construction.

This project delivers two contributions:

1. **A reproducible record on the Joe-Kuo Table 3.6 audit metric**: `COMBO_3027` (audit = 3027), a **−5.3 % improvement** over the published Joe-Kuo (2008) baseline (audit = 3196), verified byte-exact via an independent Python re-implementation of the audit recurrence.

2. **A finding that refutes a standard implicit assumption in QMC literature**: the F₂ audit subspace contains *multiple* sub-baseline basins with **functionally specialised Genz integration error profiles**, not a single audit-optimum. A practitioner should pick direction numbers by integrand family, not by audit number alone (see [`MAIN_DISCOVERIES.md`](MAIN_DISCOVERIES.md)).

The full optimisation pipeline (6 C++ search engines, incremental audit cache with rollback, parallel-tempering simulated annealing, empirical productivity probes, quadruple-verify protocol) and every dump and log used to produce the record are included in this repository so anybody can reproduce, audit, and extend.

---

## The record at a glance

| Construction | Audit (Joe-Kuo Table 3.6) | Δ vs JK_2008 | Genz wins vs JK (6 fns × 3 dims) | Specialisation |
|---|---:|---:|---:|---|
| **JK_2008** (Joe & Kuo, 2008) | 3196 | — | (baseline) | continuous, corner-peak |
| **COMBO_3027** (this work, project record) | **3027** | **−5.3 %** | **7 / 18** | oscillatory, product-peak (dim ≥ 20) |
| **E17_seed4_3095** (this work, alternative basin) | 3095 | −3.16 % | 6 / 18 | **Gaussian dim ≥ 20** |

All three constructions are included as plain-text dumps (`*_RECORD.txt`) and can be re-verified in seconds by any third party with the supplied Python verifier.

---

## Verify the record in two minutes

Requirements: Python 3.9+, `numpy`, `scipy`, `qmcpy 2.2+`.

```bash
# Clone
git clone https://github.com/<your-username>/proyecto-estrella-sobol
cd proyecto-estrella-sobol

# Verify the project record (audit = 3027)
python3 verify_F2_independent.py TOGORDO_COMBO_v1_RECORD.txt
# → Expected: audit = 3027

# Verify the Joe-Kuo (2008) baseline (audit = 3196)
python3 verify_F2_independent.py new-joe-kuo-6.21201
# → Expected: audit = 3196

# Genz integration error benchmark (~5 minutes, single thread)
python3 genz_benchmark_COMBO_vs_JK.py TOGORDO_COMBO_v1_RECORD.txt new-joe-kuo-6.21201
# → Expected: COMBO_3027 wins 7/18 cells, family-specialised
```

That's it — three commands, byte-exact verification, no special hardware.

---

## How to use this work as a practitioner

If your application has **oscillatory or product-peak integrands** (Fourier-domain methods, multiplicative-payoff financial pricing, physical simulations with rapid modes) and you work in dimension ≥ 20, **use `TOGORDO_COMBO_v1_RECORD.txt`** as your direction numbers in place of `new-joe-kuo-6.21201`.

If your application has **Gaussian-type integrands in dim ≥ 20** (Bayesian posterior expectations, certain ML hyperparameter problems), **use `togordo17_seed3_dump.txt`** (audit = 3095, alternative basin).

If your application has **smooth, continuous, or corner-peak integrands**, the canonical Joe-Kuo (2008) construction is still the best choice on this benchmark.

See [`GUIDE_FOR_EVERYONE.md`](GUIDE_FOR_EVERYONE.md) for a plain-language tour of the project, including a worked example of loading any of the three direction-number sets into `scipy.stats.qmc.Sobol`.

---

## Repository contents

All files live in the root. No nested folders.

**Records (the direction numbers themselves):**
- `TOGORDO_COMBO_v1_RECORD.txt` — the project record (audit = 3027), oscillatory / product-peak specialist · **included in this repository**
- `togordo17_seed3_dump.txt` — alternative basin (audit = 3095), Gaussian dim ≥ 20 specialist · **see SUBMIT_INSTRUCTIONS.md — must be uploaded by the author from the M2 run output**
- `new-joe-kuo-6_21201.md` — Joe-Kuo (2008) baseline reference (audit = 3196) · **included in this repository** (markdown wrapper with the first 37 dimensions; the full 21201-dim official file can be downloaded from https://web.maths.unsw.edu.au/~fkuo/sobol/ as `new-joe-kuo-6.21201`)

**Verifiers (reproduce or check):**
- `verify_F2_independent.py` — D190 step 4 redundancy verifier (Python, clean-room re-implementation)
- `genz_benchmark_COMBO_vs_JK.py` — Genz benchmark, six integrand families, three dimensions, COMBO vs any baseline

**C++ kernel (the engine):**
- `ESTRELLA_GF2_KERNEL.h` — canonical F₂ audit metric kernel
- `AUDIT_CACHE.h` — incremental cache with `CacheDiff` rollback (65× speedup, single-thread M2)
- `JK_BUILDER.h` — Joe-Kuo-spec slot construction

**Search engines (final closure cycle):**
- `togordoeldesheredado.cpp` — Engine 17 (4-seed cold-start cross-genealogy campaign; produced the 3095 alternative basin)
- `togordoelsupergordoinfiel.cpp` — Engine 18 (Move P1 chained recovery + multi-init: 2 seeds JK_2008 init + 2 seeds random JK-valid init)

Engines 13–16 from the early F₂ closure cycle are documented in `PAPER_TERMINAL.md` §217–§222 (the basin-saturation evidence under the COMBO genealogy). Their source code is not included as the final closure engines (17, 18) supersede them in arsenal, discipline, and methodology; readers wishing to replicate Engines 13–16 will find their full architectural specification in `PAPER_TERMINAL.md`.

**Documents:**
- `README.md` — this file
- `MAIN_DISCOVERIES.md` — the four scientific contributions, concise and citable
- `NEW_DISCOVERIES.md` — open journal for findings post project-closure (empty at release)
- `PAPER_TERMINAL.md` — full project paper, lossless, ~3700 lines, every engine documented
- `COJONES_SABIOS_TERMINAL.md` — operational arsenal: 12 Sobol-native levers with empirical-applicability annotations
- `METHODOLOGY.md` — a short standalone primer on the methodology (D190 verify, D172 cache, probe §C, etc.) for readers who only want the methods, not the project history
- `GUIDE_FOR_EVERYONE.md` — plain-language tour
- `CITATION.cff` — machine-readable citation
- `LICENSE` — MIT

**Analytical reference data:**
- `BOUND_COMPUTE_v1_log.txt` — analytical floors on the audit subspace (with caveat that the independence assumption is structurally false; see `MAIN_DISCOVERIES.md`)
- `PISO_TEORICO_v1_log.txt` — absolute / real floor measurements
- `RESULTADOS_FORENSIC.txt` — FORENSIC v1 structural priors (frozen slots, couplings, sextet `{21, 23, 31, 32, 33, 34}`)
- `primitive_polynomials_deg8.md`, `primitive_polynomials_deg9.md` — primitive polynomial reference data

**Engine campaign logs (raw text, reproducible):**
- `togordo13_v2_log.txt`, `togordo14_log.txt`, `togordo15_log.txt`, `togordo16_log.txt` — F₂ early closure cycle (Engines 13–16, all closed with audit-Δ = 0 over the COMBO genealogy)
- `togordo17_log.txt` — Engine 17 full campaign log (4 seeds, best seed 4 = 3095, ~33h M2)
- *(Engine 18 log not included: the campaign was terminated mid-run by the Architect after seed 3 closure when the substantive findings — F86 basin diversity and Move P1 ornamental in F₂ — were already established; see `PAPER_TERMINAL.md` §226.2 for the closure rationale.)*

---

## What this repository is, and what it isn't

**It is**: a complete, reproducible, byte-exact record of one of the rare improvements over the Joe-Kuo (2008) Sobol direction numbers on their own audit metric, together with the empirical evidence that the F₂ audit subspace is structured into multiple basins with distinct integration-error profiles — refuting an implicit assumption in QMC literature about the meaning of audit-metric optimisation.

**It is not**: a uniform replacement for Joe-Kuo (2008). The Genz benchmark in this repository makes the limits explicit: the project record beats Joe-Kuo on 7 of 18 Genz cells, not all of them. Honest practitioner advice is in `MAIN_DISCOVERIES.md` and `GUIDE_FOR_EVERYONE.md`.

---

## Citation

If this work is useful to you, please cite it:

```bibtex
@misc{amichis2026estrella,
  author       = {Amichis Luengo, Rafael},
  title        = {Proyecto Estrella: Empirical characterization of basin
                  diversity in the F\_2 Joe-Kuo Table 3.6 audit subspace,
                  with functional-family-specialised Genz performance profiles},
  year         = {2026},
  month        = may,
  howpublished = {GitHub repository},
  url          = {https://github.com/<your-username>/proyecto-estrella-sobol},
  note         = {Records: COMBO\_3027 (audit 3027, -5.3\% vs JK\_2008);
                  E17\_seed4\_3095 (audit 3095, Gaussian dim$\geq$20 specialist)}
}
```

A machine-readable `CITATION.cff` is included.

---

## Acknowledgements

- **Joe & Kuo (2008)** — original construction and reference direction numbers (`new-joe-kuo-6.21201`).
- **Niederreiter (1987-1988), Owen (1995), Faure & Lemieux (2016)** — foundational work informing the Sobol-native operational arsenal documented in `COJONES_SABIOS_TERMINAL.md`.
- **Anthropic / Claude** — AI collaboration in the Architect–Constructor–Auditor working pattern. The Architect (Rafa) arbitrated all decisions, set all hypotheses, and ran all M2 jobs. Claude instances served as Constructor (C++ engine code, scripts) and Auditor (forensic disenta against archives). Triple-role discipline documented in the paper.

---

## Contact

For questions, corrections, or to share results derived from this work, please open a GitHub issue. Substantive disagreements and reproducibility reports are particularly welcome.
 email: tretoef@gmail.com
---

*Madrid, May 2026 — Cojones rectos.*
