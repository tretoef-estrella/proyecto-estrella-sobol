# MAIN_DISCOVERIES

**Proyecto Estrella · Sobol Campaign · May 2026 · Rafael Amichis Luengo, Madrid**

This document distils the four substantive contributions of the project. Each finding is stated precisely, supported by reproducible evidence in this repository, and accompanied by a note on what it does *not* claim.

---

## Discovery 1 — A ratified record on the Joe-Kuo Table 3.6 audit metric

**Statement.** The Sobol direction number set `COMBO_3027` (Engine sequence ending 19 April 2026, lineage `TOGORDO_COMBO_v1` post-FORENSIC) achieves an audit value of **3027** on the canonical Joe-Kuo (2008) Table 3.6 metric (dims 2..37, m ∈ [5, 25]), versus **3196** for the published Joe-Kuo (2008) construction `new-joe-kuo-6.21201`. The reduction is **−169 absolute, −5.29 % relative**.

**Why this matters.** The Joe-Kuo (2008) direction numbers are the de facto standard in modern QMC software (scipy, BoTorch, MATLAB, NAG, QuantLib). Their own audit metric is the criterion they reported and optimised in the original paper. No ratified improvement on this metric has been published in the eighteen years between Joe-Kuo (2008) and this work.

**Evidence.**
- Dump: `TOGORDO_COMBO_v1_RECORD.txt` (plain text, 37 dimensions, `d s a m_1 .. m_s` format).
- Verification: `python3 verify_F2_independent.py TOGORDO_COMBO_v1_RECORD.txt` returns `audit = 3027`, byte-exact, in under one second.
- Baseline cross-check: `python3 verify_F2_independent.py new-joe-kuo-6.21201` returns `audit = 3196` on the official Joe-Kuo file.
- Cumulative empirical search: ≈ 6 × 10⁶ greedy and SA proposals across six engines (Engines 13–18, source code included), saturating the basin around 3027 with audit-Δ = 0 over the canonical search subspace.

**Caveat.** D190 verification step 4 (the independent Python re-implementation) shares the Joe-Kuo specification of the m_k recurrence and t-value rank predicate with the C++ kernel; the column packing differs (LSB-first vs MSB-first) but this is a bit-reversal, not a mathematical independence. The current verifier therefore catches implementation errors, not specification errors. A genuinely independent verifier using `qmcpy` is recommended pre-publication and is the natural first task for any continuation of this work.

---

## Discovery 2 — Basin diversity with functionally specialised Genz profiles

**Statement.** The F₂ audit subspace contains **multiple sub-baseline basins with distinct Genz integration-error profiles by integrand family**. No single F₂ basin is uniformly better than Joe-Kuo (2008) across the standard six Genz functions in dimensions 10, 20, 30. This refutes a *single-basin-optimum* assumption commonly implicit in the QMC literature, where audit-metric optimisation is treated as a unique target.

**Concrete instance.** Two distinct sub-baseline basins were found and benchmarked:

| Construction | Audit | Genz wins vs JK (6 fns × 3 dims) | Strongest family |
|---|---:|---:|---|
| `COMBO_3027` | 3027 | 7 / 18 | Oscillatory + Product-peak (dim ≥ 20) |
| `E17_seed4_3095` | 3095 | 6 / 18 | **Gaussian dim ≥ 20** |
| `JK_2008` | 3196 | (baseline) | Continuous, Corner-peak |

The two project-discovered basins are different both in audit (3027 vs 3095) and in their family of best Genz performance. `COMBO_3027` wins Oscillatory cleanly across all three dims and Product-peak in dim ≥ 20; `E17_seed4_3095` wins Gaussian in dim 20 and dim 30 where `COMBO_3027` loses.

**Why this matters.**
1. **Empirical refutation** of the implicit assumption that audit-metric reduction tracks Genz-error reduction uniformly. The relationship is family-specific.
2. **Practitioner consequence**: direction numbers should be selected by integrand family, not by audit number. The repository ships three sets (`COMBO_3027`, `E17_seed4_3095`, `new-joe-kuo-6.21201`) with documented family specialisations.
3. **Theoretical consequence**: any future search over Sobol direction numbers should benchmark each basin against the practitioner-relevant metric (e.g. Genz), not rely on audit improvement as a sufficient indicator.

**Evidence.**
- Genz benchmark script: `genz_benchmark_COMBO_vs_JK.py` (6 functions × 3 dims × 30 reps × N = 2¹⁶ points, reference truth via MC at N = 2²², 30 distinct random integrand parameter sets per cell).
- Run on `COMBO_3027` vs `JK_2008`: 7/18 wins, full per-cell table reproducible in ≈ 5 minutes on an M2.
- Run on `togordo17_seed3_dump.txt` (`E17_seed4_3095`) vs `JK_2008`: 6/18 wins, *different* family pattern.

**Caveat.** This is established on the six standard Genz families in dimensions 10, 20, 30, with N = 2¹⁶. Other test suites (e.g. Asian options, BSDE solvers, deep-learning hyperparameter selection) may produce different rankings. The methodological point — *audit and integration error are not monotonically related, even within F₂* — is robust; the precise practitioner guidance is family-specific.

---

## Discovery 3 — Tridirectional ratification of audit-error non-monotonicity

**Statement.** The Joe-Kuo Table 3.6 audit metric and practical integration-error performance are **non-monotonically related in three distinct senses**, each established by an independent empirical observation in this project:

1. **Cross-substrate (Fₚ ↔ F₂)**. A near-optimal Fₚ audit configuration, when lifted to its corresponding F₂ representation, yields an F₂ audit value worse than `JK_2008`. Empirical instance: a sub-500 F₆₄ audit configuration (`TRINCANERO 414`) lifted to F₂ produces audit = 4541, i.e. +42 % over `JK_2008`. The Fₚ optimisation track does not transfer to F₂.

2. **F₂ audit ↔ F₂ Genz**. The improved F₂ audit value `COMBO_3027` reduces Genz integration error on only 7 of 18 standard cells (six Genz families × three dimensions). A 5.3 % audit improvement does not imply a uniform Genz improvement.

3. **F₂ basin-A vs F₂ basin-B**. Two distinct F₂ basins (`COMBO_3027`, `E17_seed4_3095`) with different audit values (3027 vs 3095) display different Genz win-loss patterns by integrand family. Audit-better is not Genz-better even within F₂.

**Operational corollary**. The Joe-Kuo Table 3.6 audit metric is a **weak proxy** for practitioner performance: it has segments of validity (Oscillatory and Product-peak in higher dimensions) but is not a uniform indicator. Any future record claim on this metric should be accompanied by an integration-error benchmark.

**Evidence.** All three observations are reproducible from files included in this repository: Fₚ tracks are documented in `PAPER_TERMINAL.md` §F-history; the COMBO vs JK Genz run and the E17 vs JK Genz run both use `genz_benchmark_COMBO_vs_JK.py`.

---

## Discovery 4 — Methodology that is publishable independently of the records

The optimisation pipeline developed for this project is itself a contribution. Five components are reusable and may be of interest beyond the specific Joe-Kuo Table 3.6 case:

**D172 — Incremental audit cache with rollback (`AUDIT_CACHE.h`).** A pair-table caching scheme (`PAIR_T` and `MAX_T` per dim) with a `CacheDiff` rollback structure enables single-slot move evaluation in O(d) instead of the O(d²) of full re-audit. Measured speedup: **65× on a single thread (Apple M2)**, ratified across all six engines, sanity-tested with a separate verification pass (`sanity_v2_log.txt`).

**D190 — Quadruple verification protocol.** Every record claim is checked four ways: (1) cold rebuild and total audit from the packed slot state, (2) round-trip via dump file (serialise, parse, rebuild, re-audit), (3) structural invariant check on every slot, (4) independent Python re-implementation. The combination catches implementation, parsing, and serialisation bugs that any single check would miss. Documented in `PAPER_TERMINAL.md` §190.

**Sobol-native operational arsenal — twelve levers with empirical annotations (`COJONES_SABIOS_TERMINAL.md`).** Each lever (cache D172, Pre-SA F47 cross-substrate-NEW, multi-chain ladder F44, parallel tempering, cross-chain non-self-referential coupling, pressure-directed slot selection F78, runtime mass recalibration MIX_RECAL, runtime decision matrix, triple-rescue cascades D189, tabu caps per chain, AUTO-EXTEND lax, post-SA polish D184, probe-before-commit) is documented with its empirical pedigree and applicability annotations, including which levers proved regime-dependent (Move P1 chained: productive in Fₚ warm basins, ornamental in F₂ cold cross-genealogy).

**Probe §C — empirical productivity test before mass allocation.** Before committing an SA move type to a fixed proposal mass, sample 1000 random attempts of that move from the current basin, measure the 25th-percentile delta. If p25 ≥ 0 the move is ornamental from that basin and should not receive SA mass. Implemented in Engine 18 (`togordoelsupergordoinfiel.cpp`); empirical instance: Move P1 (chained 1-flip) probe from `JK_2008` init returned p25 = +6, p50 = +13, 1.4 % productive — ornamental — and was correctly given mass 0 by the runtime calibration.

**Hardened pre-launch checklist (`PAPER_TERMINAL.md` §225.7).** A five-element checklist gating any new engine: pre-registered quantitative hypothesis with refutation criterion, probe §C results for any new or repurposed move, explicit cold-start source (no warm-load from prior dumps), causal chain {observation → modification → prediction}, pre-committed compute budget and kill criterion. Adopted after a fresh-eye audit caught a critical implementation bug in Engine 17 (a chained-move alias) that had escaped the original author.

These five components are reusable for any discrete-search problem with a multi-cell aggregate metric and incremental-update structure, not only for Sobol direction numbers.

---

## What was *not* achieved (honest scope)

- **Sub-3027** was not achieved over the canonical Joe-Kuo Table 3.6 subspace after six engines (Engines 13–18). The best cross-genealogy cold-start basin found is 3095, which is +68 over the project record. The 3027 basin is therefore the empirically dominant attractor within the search subspace explored.
- **Sub-2500** was an early aspirational target that was retracted before project closure as not empirically grounded (see `PAPER_TERMINAL.md` §225, retractions C2.1 and C4.1).
- **Uniform practitioner improvement over Joe-Kuo (2008)** is not claimed. The Genz benchmark is mixed; the practitioner contribution is *specialised*, not uniform.
- **A genuinely independent verifier** (using `qmcpy` or an alternative t-value algorithm) is not yet implemented. The current Python verifier catches implementation bugs but shares the Joe-Kuo specification with the C++ kernel.
- **The standard `BOUND_COMPUTE_v1` archive** records "record = 3044" in its line 70 because it pre-dates the final 3027 result; gaps in this repository are recalculated against 3027 but the original archive file is preserved for traceability.

---

## How to verify each claim

Every claim above is reproducible from files in this repository. The shortest path:

```bash
# Discovery 1 (the record)
python3 verify_F2_independent.py TOGORDO_COMBO_v1_RECORD.txt
python3 verify_F2_independent.py new-joe-kuo-6.21201

# Discovery 2 + 3 (basin diversity, audit↔Genz non-monotonicity)
python3 genz_benchmark_COMBO_vs_JK.py TOGORDO_COMBO_v1_RECORD.txt new-joe-kuo-6.21201
python3 genz_benchmark_COMBO_vs_JK.py togordo17_seed3_dump.txt new-joe-kuo-6.21201

# Discovery 4 (methodology): read COJONES_SABIOS_TERMINAL.md and METHODOLOGY.md;
# the C++ pipeline is in *.cpp and *.h, fully self-contained.
```

For deeper inspection: read `PAPER_TERMINAL.md` (full lossless history) and `COJONES_SABIOS_TERMINAL.md` (operational arsenal). The campaign logs (`togordo*_log.txt`) contain every run's raw output.

---

*Madrid, May 2026.*
