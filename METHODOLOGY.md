# METHODOLOGY

**A short, standalone primer on the methodology used in Proyecto Estrella.** This file is for readers who want the methods without the project history. The full history is in `PAPER_TERMINAL.md`; the operational arsenal with empirical annotations is in `COJONES_SABIOS_TERMINAL.md`.

---

## 1. The problem

Given:
- A primitive polynomial degree `s` (1 ≤ s ≤ 9) per Sobol dimension `d`,
- An integer `a` encoding the intermediate coefficients of that polynomial,
- An initial direction sequence `m_1, m_2, ..., m_s` (each odd, with `m_k < 2^k`),

the Joe-Kuo (2008) recurrence extends `(m_k)` to a full direction-number sequence used in Sobol generation. For each pair of dimensions `(j, d)` and each block size `m`, a *t-value* `t(j, d, m)` is defined via the rank deficiency of a particular binary matrix derived from the direction numbers.

**The audit metric** of Joe-Kuo Table 3.6 is

```
audit(slot) = Σ_{d = 2..37} Σ_{m = 5..25} max_{j < d} t(j, d, m).
```

Lower is better. The published Joe-Kuo (2008) construction achieves audit = 3196. This project achieves audit = 3027 (`COMBO_3027_RECORD.txt`).

---

## 2. Search architecture

The search is a discrete combinatorial optimisation over the slot space `(s, a, m_init)`-per-dim, with a multi-cell aggregate objective. The architecture is:

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
- **Random Joe-Kuo-valid init**: each dim independently sampled with a biased grade distribution (`{3, 5, 5, 6, 6, 7, 7, 8, 8, 9}`), a random valid `a` for that grade, and a random valid `m_init`. Typical initial audit: 4000–8000. Used by Engine 18 Sub-B.

No warm-load from a prior project dump is allowed in the post-Engine-16 discipline; see `PAPER_TERMINAL.md` §224 (the "learn from logs, do not warm-load" distinction).

---

## 4. Pre-SA F47 cross-substrate-NEW exhaustive greedy

**Three nested passes**, iterated until a full cycle produces Δ = 0:

- **Pass 1 (singles)**: for each flexible dim `d` and each candidate `(s, a)` in the polynomial pool, sample up to 30 random `m_init` values, keep the best, commit if it improves the current audit.
- **Pass 2 (pressure pairs)**: rank flex dims by per-dim `PAIR_T_max` pressure; take the top 24, generate all pairs; add the FORENSIC empirically-correlated pairs as a non-mandatory hypothesis; for each pair try 30 random `(s_i, m_i, s_j, m_j)` joint replacements.
- **Pass 3 (pressure triples)**: top-10 pressure triples plus FORENSIC triples; 8 joint random replacements per triple.

The "F47" label refers to the empirical finding that this pass produces real descent (~−20 to −95 audit points cumulative) when the substrate is genuinely cold — i.e. starting from JK_2008 or from a different substrate, not from a warm-loaded prior cluster. The law was ratified four times across substrates F₄, F₈, F₁₆, F₃₂ in earlier project eras, and a fifth time in F₂ Engine 17 (Pre-SA Δ ∈ [−22, −26] across 4 seeds).

---

## 5. Probe §C — empirical productivity test before mass allocation

**Before SA mass is committed to any structural move**, sample 1000 random attempts of that move from the current basin and measure the distribution of audit-Δ:

- if `p25(Δ) ≥ 0` → the move is **ornamental** from this basin; assign SA mass = 0.
- if `p25(Δ) < 0` → the move is **productive**; assign SA mass scaled by `|p25|`.

This is the antidote to *etiquetas sin entregar* — labels without delivery. Probe §C lets the runtime data, not the prior project lore, decide which moves carry mass.

Engine 18 worked example: Move P1 (chained 1-flip P1 → P2 → P3 cross-cell adjacent), probed from a JK_2008 init basin, returned `p25 = +6, p50 = +13, 1.4 % productive` — ornamental. The mass calibration set P1 to 0 automatically, freeing that mass for B/R/Q1. From a random-init basin (audit ≈ 4772), the same Move P1 probe returned `p25 = -3, 30.6 % productive` — productive in that regime. Move productivity is basin-dependent, not constant.

---

## 6. SA-PT — Simulated Annealing with Parallel Tempering

Five chains at local temperatures `T_local ∈ {0.05, 0.10, 0.20, 0.50, 1.00}`. Every 1500 iterations a random adjacent pair attempts a Metropolis-Hastings swap with acceptance probability

```
P_swap = min(1, exp((β_i - β_j) · (E_i - E_j)))
```

where `β = 1/T` and `E` is the current audit. Within each chain, a single SA step samples a move type from the current mass distribution, applies it, evaluates the audit delta via the incremental cache, and accepts with Metropolis probability `min(1, exp(-Δ / T))`.

The dispatcher samples among six move types: B (single bit flip), R (random odd resample of one `m_k`), P1 (chained 1-flip P1 → P2 → P3, **as a genuinely separate move in Engine 18**, not aliased to B), Q1 (slot re-randomise at fixed `s`), W (grade promote/demote), S (slot swap between two dims).

Every 20000 iterations, **MIX_RECAL** re-fits each chain's mass distribution to the empirical productivity ratio `(events + 1) / (proposals + 10)` per move, with a floor of 5 (or 0 if probe §C ruled the move ornamental) and a ceiling of 60.

A **tabu ring buffer** of 100 state fingerprints prevents revisits within each chain.

Stagnation trigger: 8000 iterations without improving the chain's best triggers a **mini-greedy rescue** (top-3 pressure slots, bit-flip exhaustive, a-replace exhaustive, Move S exhaustive against all other dims).

AUTO-EXTEND: if the iteration budget is reached while improvement is still recent (within 50000 iters), extend by another 50000.

---

## 7. Incremental audit cache (D172) with rollback

The audit is stored as two cached tables per dim:

- `PAIR_T[d][m]` — the per-cell t-values for pairs `(j, d)` for each `m`,
- `PAIR_T_max[d][m]` — the running `max_{j < d} t(j, d, m)`,

and a `total_audit` running sum. Whenever a single slot `d` is changed, only `PAIR_T[d][·]`, `PAIR_T[d'][·]` for `d' > d`, and the corresponding maxes need updating. The update is **bounded in time independent of d** for slots where the new `(s, a, m)` is the same length as the old.

A `CacheDiff` structure records every cell write during an incremental update, so a failed Metropolis acceptance can roll back to the previous state in O(diff size), not by re-auditing.

Measured speedup: **65× on a single thread (Apple M2)** versus a full `O(d²)` re-audit of every move.

---

## 8. Polish

After SA terminates, the best state runs through a deterministic polish loop, up to 50 passes:

- For each dim, every legal single bit-flip in `m_k` for `k ∈ [1, s-1]`, kept if Δ < 0.
- For each dim, every alternative `a` in the polynomial pool, kept if Δ < 0.
- For every pair of dims `(d_i, d_j)`, a Move S slot swap, kept if Δ < 0.
- For top-pressure dims, additional Move P1 chained attempts.

The polish typically yields another −3 to −15 audit points on top of the SA best. It is deterministic and exhaustive over the listed local neighbourhoods.

---

## 9. D190 quadruple verify protocol

A record claim requires all four of the following to pass:

1. **FINAL_VERIFY**: rebuild a fresh `AuditState` from the runtime slots from scratch and compare `total_audit` byte-exact with the runtime value.
2. **ROUNDTRIP**: serialise the slots to a plain-text dump, parse them back into a fresh slot array, rebuild and audit, compare byte-exact.
3. **STRUCTURAL**: validate slot invariants on every dim (`m_1 = 1`, each `m_k` odd, `m_k < 2^k`).
4. **INDEPENDENT (currently: REDUNDANCY)**: run the Python verifier `verify_F2_independent.py` on the dump and compare the audit value.

Step 4 currently catches implementation errors (parser, off-by-one, indexing) but shares the Joe-Kuo specification of the m_k recurrence and t-value with the C++ kernel. A genuinely independent verifier using `qmcpy` is the natural next item; the current verifier is documented as a "redundancy verifier" in `MAIN_DISCOVERIES.md` and in `COJONES_SABIOS_TERMINAL.md` §K6.

---

## 10. Move types — definitions

For reference, the move types used in this project:

| Move | Action | Cells touched |
|---|---|---|
| **B** | single bit-flip in one `m_k` of one dim | 1 |
| **R** | resample `m_k` (random odd, `< 2^k`) in one dim, single `k` | 1 |
| **P1** (real, Engine 18) | chained: B at random cell `k_1`, then if Δ<0 try B at cell `k_1+1`, then if Δ<0 try `k_1+2` | 1 to 3 |
| **Q1** | full random resample of `(a, m_init)` at preserved `s` | 1 dim |
| **W** | grade promote/demote (`s → s±1`) with fresh `(a, m_init)` | 1 dim |
| **S** | slot swap: exchange `(s, a, m_init)` between two dims | 2 dims |

Moves K, L, U, T3 from Fₚ eras were tested earlier and either ratified as cross-field-broken or ornamental in F₂ (see `COJONES_SABIOS_TERMINAL.md` §10 annotations).

---

## 11. Pressure-directed slot selection (F78)

Slots are not sampled uniformly when picking a target for moves B, R, P1, Q1, W. Each flex slot `d` has a *pressure* score

```
pressure(d) = Σ_m PAIR_T_max[d][m]
```

i.e. the sum of its per-cell t-value maxes. Selection is weighted by pressure, so high-contribution slots are touched preferentially. This is the empirical finding F78 ratified in earlier eras and binding via D223. Uniform slot selection where pressure data is available is a project anti-pattern.

---

## 12. Genz benchmark

`genz_benchmark_COMBO_vs_JK.py` runs the standard Genz suite:

- **Six integrand families**: Oscillatory, Product peak, Corner peak, Gaussian, Continuous, Discontinuous.
- **Three dimensions**: 10, 20, 30.
- **Per cell**: 30 replicate runs with independent random Genz parameters `(a, u)`, each at N = 2¹⁶ points; reference truth via Monte Carlo at N = 2²².
- **Output**: per-cell mean absolute error for each of the two direction-number sets being compared, plus a verdict on the F75 dual-form question (does audit improvement translate to Genz error reduction?).

Runtime: ≈ 5 minutes on Apple M2.

---

## 13. Adapting the methodology elsewhere

The architecture is generic over discrete-search problems with the following properties:

- The state is a vector of per-component descriptors.
- The objective is a multi-cell aggregate (a sum-of-maxes over a structured grid).
- Single-component changes can be evaluated incrementally with bounded work and bounded rollback.
- Independent verification is possible by a second implementation.

Specific domain knowledge — the Joe-Kuo recurrence, the t-value definition, the dimensionality, the pressure metric — is encapsulated in three header files (`ESTRELLA_GF2_KERNEL.h`, `AUDIT_CACHE.h`, `JK_BUILDER.h`) and the Python verifier. The SA-PT loop, MIX_RECAL, probe §C, mini-greedy rescue, polish, and D190 protocol are agnostic and reusable.

---

*This primer covers the methodology only. For the project's empirical history (six engines, retractions, fresh-eye audit, closure decision) see `PAPER_TERMINAL.md`. For the operational arsenal with regime-by-regime annotations see `COJONES_SABIOS_TERMINAL.md`. For the four scientific contributions in compressed form see `MAIN_DISCOVERIES.md`.*
