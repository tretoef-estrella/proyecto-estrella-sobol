# METHODOLOGY

**A short, standalone primer on the methodology used in Proyecto Estrella.** This file is for readers who want the methods without the project history. The full history is in `PAPER_TERMINAL.md`; the operational arsenal with empirical annotations is in `COJONES_SABIOS_TERMINAL.md`.

All numerical parameters quoted below are the actual compile-time constants of the closure-cycle engine (`togordoeldesheredado.cpp`), not illustrative values.

---

## 1. The problem

Given:
- A primitive polynomial of degree `s` (1 ≤ s ≤ 9) per Sobol dimension `d`,
- An integer `a` encoding the intermediate coefficients of that polynomial,
- An initial direction sequence `m_1, m_2, ..., m_s` (each odd, with `m_k < 2^k`),

the Joe-Kuo (2008) recurrence extends `(m_k)` to a full direction-number sequence used in Sobol generation. For each pair of dimensions `(j, d)` and each block size `m`, a *t-value* `t(j, d, m)` is defined via the rank deficiency of a particular binary matrix derived from the direction numbers.

**The audit metric** of Joe-Kuo Table 3.6 is

```
audit(slot) = Σ_{d = 2..37} Σ_{m = 5..25} max_{j < d} t(j, d, m).
```

Lower is better. The published Joe-Kuo (2008) construction achieves audit = 3196. This project achieves audit = 3027 (`TOGORDO_COMBO_v1_RECORD.txt`).

**A note on the field.** This metric is defined over GF(2), the binary field, which is the field every standard QMC library uses. The project also ran the same machinery over the larger fields GF(4), GF(8), GF(16), GF(32), GF(64) — and found that records there, though real within those fields, do not transfer back to GF(2) (the GF(64) record of 414 lifts to a GF(2) value of 4541, worse than the baseline). That cross-field non-monotonicity is a project finding, not a methodological assumption; see `MAIN_DISCOVERIES.md` Discovery 3. The methods below are described for the GF(2) attack, which is the practitioner-relevant one. The same architecture was used in every field; only the kernel arithmetic changed.

---

## 2. Search architecture

The search is a discrete combinatorial optimisation over the `(s, a, m_init)`-per-dim slot space, with a multi-cell aggregate objective. The architecture is:

```
   ┌──────────────────────────────────────────────────────────┐
   │ INIT  →  Pre-SA F47 exhaustive greedy  →  Probe §C        │
   │                ↓                                           │
   │  SA-PT 5 chains  ←──  mass calibration from probe          │
   │       ↓                                                    │
   │  Mini-greedy rescue (on stagnation)                        │
   │       ↓                                                    │
   │  Polish 50-pass  →  D190 quadruple verify                  │
   └──────────────────────────────────────────────────────────┘
```

Each phase is documented below.

---

## 3. Init

Two sources are supported:

- **Joe-Kuo cold-start**: load the JK_2008 baseline (audit = 3196) as starting state. Used by Engines 13–17.
- **Random Joe-Kuo-valid init**: each dim independently sampled with a biased grade distribution, a random valid `a` for that grade, and a random valid `m_init`. Typical initial audit: 4000–8000. Used by Engine 18 Sub-B.

No warm-load from a prior project dump is allowed in the post-Engine-16 discipline; see `PAPER_TERMINAL.md` §224 (the "learn from logs, do not warm-load" distinction). This matters: a warm-load would inherit a basin rather than explore for one, and several findings below depend on each engine starting genuinely cold.

---

## 4. Pre-SA F47 cross-substrate exhaustive greedy

**Three nested passes**, iterated until a full cycle produces Δ = 0:

- **Pass 1 (singles)**: for each flexible dim `d` and each candidate `(s, a)` in the polynomial pool, sample up to 30 random `m_init` values, keep the best, commit if it improves the current audit.
- **Pass 2 (pressure pairs)**: rank flex dims by per-dim pressure; take the top 24, generate all pairs, add the FORENSIC empirically-correlated pairs as a non-mandatory hypothesis; for each pair try 30 random joint replacements.
- **Pass 3 (pressure triples)**: top-10 pressure triples plus FORENSIC triples; 8 joint random replacements per triple.

The "F47" label refers to the empirical finding that this pass produces real descent (≈ −20 to −95 audit points cumulative) when the substrate is genuinely cold — starting from JK_2008 or from a different field, not from a warm-loaded prior cluster. The law was ratified four times across fields GF(4), GF(8), GF(16), GF(32) in earlier project eras, and a fifth time in GF(2) Engine 17 (Pre-SA Δ ∈ [−22, −26] across 4 seeds).

---

## 5. Probe §C — empirical productivity test before mass allocation

**Before SA mass is committed to any structural move**, sample 1000 random attempts of that move from the current basin and measure the distribution of audit-Δ:

- if `p25(Δ) ≥ 0` → the move is **ornamental** from this basin; assign SA mass = 0.
- if `p25(Δ) < 0` → the move is **productive**; assign SA mass scaled by `|p25|`.

This is the antidote to *labels without delivery* — invoking a move because lore says it once worked. Probe §C lets live data from the current basin, not prior-project lore, decide which moves carry mass.

Engine 18 worked example: Move P1 (chained 1-flip P1 → P2 → P3 cross-cell adjacent), probed from a JK_2008 init basin, returned `p25 = +6, p50 = +13, 1.4 % productive` — ornamental — and the calibration set P1 to mass 0 automatically, freeing that mass for B/R/Q1. From a random-init basin (audit ≈ 4772), the *same* Move P1 probe returned `p25 = −3, 30.6 % productive` — productive in that regime. **Move productivity is a function of the current basin, not a constant.**

---

## 6. SA-PT — Simulated Annealing with Parallel Tempering

Five chains at local temperatures `T_local ∈ {0.05, 0.10, 0.20, 0.50, 1.00}` (`N_CHAINS = 5`, `T_LOCAL` constant). Every `PT_SWAP_PERIOD = 1500` iterations a random adjacent pair attempts a Metropolis-Hastings swap with acceptance probability

```
P_swap = min(1, exp((β_i - β_j) · (E_i - E_j)))
```

where `β = 1/T` and `E` is the current audit. Within each chain, a single SA step samples a move type from the current mass distribution, applies it, evaluates the audit delta via the incremental cache, and accepts with Metropolis probability `min(1, exp(-Δ / T))`.

The dispatcher samples among six move types with base mass `{B:15, R:15, P1:35, Q1:15, W:10, S:10}` (refined at runtime by MIX_RECAL): B (single bit flip), R (random odd resample of one `m_k`), P1 (chained 1-flip P1 → P2 → P3, **a genuinely separate move in Engine 18**, not aliased to B), Q1 (slot re-randomise at fixed `s`), W (grade promote/demote), S (slot swap between two dims, over the 36-dim flex space).

Every `MIX_RECAL_WINDOW = 20000` iterations, **MIX_RECAL** re-fits each chain's mass distribution to the empirical productivity ratio `(events + 1) / (proposals + 10)` per move, with a floor (or 0 if probe §C ruled the move ornamental) and a ceiling.

A **tabu ring buffer** of `TABU_CAP = 100` state fingerprints prevents revisits within each chain.

Stagnation trigger: `STAGNATION_TRIGGER = 8000` iterations without improving the chain's best triggers a **mini-greedy rescue** (top pressure slots, bit-flip exhaustive, a-replace exhaustive, Move S exhaustive against all other dims).

AUTO-EXTEND: base budget `PHASE2_BASE_ITERS = 200000`; if the budget is reached while improvement is still recent (within `AUTO_EXTEND_WINDOW = 50000` iters), extend, up to `MAX_TOTAL_ITERS = 1500000`.

---

## 7. Incremental audit cache (D172) with rollback

The audit is stored as cached tables per dim:

- `PAIR_T[d][m]` — the per-cell t-values for pairs `(j, d)` for each `m`,
- `MAX_T[d][m]` — the running `max_{j < d} t(j, d, m)`,

and a `total_audit` running sum. Whenever a single slot `d` is changed, only `PAIR_T[d][·]`, the entries `PAIR_T[d'][·]` for `d' > d`, and the corresponding maxes need updating, with three-case logic (new ≥ old max / old < old max / old was the culprit). For a single-slot change where the new and old `(s, a, m)` are the same length, the update is **bounded essentially independently of the dimension count** rather than the `O(d²)` of full re-audit.

A `CacheDiff` structure records every cell write during an incremental update, so a failed Metropolis acceptance can roll back in time proportional to the diff size, not by re-auditing.

Measured speedup: **65× on a single thread (Apple M2)**, ratified across all six closure-cycle engines, and sanity-tested with a separate verification pass (`sanity_v2_log.txt`).

---

## 8. Polish

After SA terminates, the best state runs through a deterministic polish loop, up to 50 passes:

- For each dim, every legal single bit-flip in `m_k` for `k ∈ [1, s-1]`, kept if Δ < 0.
- For each dim, every alternative `a` in the polynomial pool, kept if Δ < 0.
- For every pair of dims `(d_i, d_j)`, a Move S slot swap, kept if Δ < 0.
- For top-pressure dims, additional Move P1 chained attempts.

Polish typically yields another −3 to −15 audit points on top of the SA best (in some engines as little as +1; it is deterministic and exhaustive over the listed local neighbourhoods, so the gain reflects how saturated the SA result already was).

---

## 9. D190 quadruple verify protocol

A record claim requires all four to pass:

1. **FINAL_VERIFY**: rebuild a fresh `AuditState` from the runtime slots from scratch and compare `total_audit` byte-exact with the runtime value.
2. **ROUNDTRIP**: serialise the slots to a plain-text dump, parse them back into a fresh slot array, rebuild and audit, compare byte-exact.
3. **STRUCTURAL**: validate slot invariants on every dim (`m_1 = 1`, each `m_k` odd, `m_k < 2^k`).
4. **REDUNDANCY (Python re-implementation)**: run `verify_F2_independent.py` on the dump and compare the audit value.

Step 4 currently catches implementation errors (parser, off-by-one, indexing) but shares the Joe-Kuo specification of the `m_k` recurrence and t-value with the C++ kernel — the column packing differs (LSB-first vs MSB-first), which is a bit-reversal, not mathematical independence. It is therefore honestly a **redundancy verifier**, not an independent one. A genuinely independent verifier using `qmcpy` cross-validation is the natural next item and the recommended first task for any continuation; the distinction is documented in `MAIN_DISCOVERIES.md` and `COJONES_SABIOS_TERMINAL.md` §K6.

---

## 10. Move types — definitions

| Move | Action | Cells touched |
|---|---|---|
| **B** | single bit-flip in one `m_k` of one dim | 1 |
| **R** | resample `m_k` (random odd, `< 2^k`) in one dim, single `k` | 1 |
| **P1** (real, Engine 18) | chained: B at random cell `k_1`, then if Δ<0 try `k_1+1`, then if Δ<0 try `k_1+2` | 1 to 3 |
| **Q1** | full random resample of `(a, m_init)` at preserved `s` | 1 dim |
| **W** | grade promote/demote (`s → s±1`) with fresh `(a, m_init)` | 1 dim |
| **S** | slot swap: exchange `(s, a, m_init)` between two dims | 2 dims |

Moves from the larger-field eras (K, L, U, T3) were tested earlier and either ratified as cross-field-broken or ornamental in GF(2); see `COJONES_SABIOS_TERMINAL.md` §10 annotations.

---

## 11. Pressure-directed slot selection (F78)

Slots are not sampled uniformly when picking a target for moves B, R, P1, Q1, W. Each flex slot `d` has a *pressure* score

```
pressure(d) = Σ_m MAX_T[d][m]
```

i.e. the sum of its per-cell t-value maxes — its actual contribution to the objective. Selection is weighted by pressure, so high-contribution slots are touched preferentially. This is finding F78, ratified in earlier eras and binding via D223: **uniform slot selection where pressure data is available is a project anti-pattern**, because it spends the same effort on slots that cannot move the objective as on those that can.

---

## 12. Genz benchmark

`genz_benchmark_COMBO_vs_JK.py` runs the standard Genz suite:

- **Six integrand families**: Oscillatory, Product peak, Corner peak, Gaussian, Continuous, Discontinuous.
- **Three dimensions**: 10, 20, 30.
- **Per cell**: 30 replicate runs with independent random Genz parameters, each at N = 2¹⁶ points; reference truth via Monte Carlo at N = 2²².
- **Output**: per-cell mean absolute error for each of the two direction-number sets compared, plus a verdict on whether audit improvement translated to Genz error reduction.

Runtime: ≈ 5 minutes on Apple M2. This benchmark is mandatory before the word "improvement" is attached to any audit record (the rule that closed the larger-field track).

---

## 13. Adapting the methodology elsewhere

The architecture is generic over discrete-search problems with these properties:

- The state is a vector of per-component descriptors.
- The objective is a multi-cell aggregate (a sum-of-maxes over a structured grid).
- Single-component changes can be evaluated incrementally with bounded work and bounded rollback.
- Independent verification is possible by a second implementation.

Domain knowledge — the Joe-Kuo recurrence, the t-value definition, the dimensionality, the pressure metric — is encapsulated in three header files (`ESTRELLA_GF2_KERNEL.h`, `AUDIT_CACHE.h`, `JK_BUILDER.h`) and the Python verifier. The SA-PT loop, MIX_RECAL, probe §C, mini-greedy rescue, polish, and the D190 protocol are domain-agnostic and reusable. The single most transferable lesson is probe §C: **measure a move's productivity from the basin you are actually in before you spend any budget on it** — the larger-field detour is the cautionary tale of what happens when a metric is trusted without that check.

---

*This primer covers the methodology only. For the project's empirical history (the larger-field climb, six GF(2) engines, the retractions, the fresh-eye audit, the closure decision) see `PAPER_TERMINAL.md`. For the operational arsenal with regime-by-regime annotations see `COJONES_SABIOS_TERMINAL.md`. For the scientific contributions in compressed form see `MAIN_DISCOVERIES.md`.*
