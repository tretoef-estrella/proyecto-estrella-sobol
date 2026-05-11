// ============================================================================
// togordoeldesheredado.cpp — Engine 17 single-file standalone
//
// Engine: TOGORDOELDESHEREDADO  (the disinherited — no warm-load, no frozen)
// Constructor v53 (Cojones Rectos), 9 May 2026
// Architect: Rafael Amichis Luengo
//
// Substrate: F_2 (canonical Joe-Kuo Table 3.6 audit metric)
// Warm start: JK_2008 baseline (audit 3196) — NOT COMBO_3027
// Frozen slots: NONE — all 36 audit dims (2..37) are flexible from start
// Multi-seed: N=4 independent cold-starts in serial single-binary
//
// THESIS (per LEEME_PRIMERO_ENGINE_17_ENTRY_POINT §3): Engines 13-16
// exhausted the subspace REACHABLE from COMBO_3027 warm-start genealogy.
// COMBO_3027 saturated within its basin. To break 3027 requires a
// DIFFERENT basin reached via cold-start cross-genealogy with no
// inherited FORENSIC frozen assumption.
//
// SCIENTIFIC OUTCOMES:
//   - Best of N seeds < 3027  → cross-genealogy descent achieved, RECORD.
//   - Best of N seeds = 3027  → COMBO basin re-discovered from cold. Strong
//                                  evidence COMBO is structurally preferred
//                                  attractor of canonical F_2 audit subspace.
//   - Best of N seeds > 3027  → cold-start finds different (worse) basin.
//                                  Publishable F_2 basin landscape data.
//
// NOTE: This is the FIRST engine in project history that does NOT warm-load
// from a prior F_2 cluster. F47 cross-substrate-cold law (D227) APPLIES here
// — JK_2008 baseline is not a "saturated cluster" but a published reference
// (s, a, m_init) configuration. Pre-SA exhaustive greedy expected to descend
// ~80-150 pts (audit 3196 → ~3050-3120) before SA Phase 2.
//
// PHASES (per seed):
//   §1 — Pre-SA F47 EXHAUSTIVE GREEDY from JK_2008. 36 flex dims (no frozen).
//        Pass 1: 30 m_init trials per (d, s, a) over POLY{1..9}.
//        Pass 2: top-24 pressure pairs + FORENSIC pairs (hypothesis, not
//                mandatory) × 30 trials.
//        Pass 3: top-20 pressure triples + FORENSIC triples × 8 trials.
//        Iterate Pass 1→2→3 until cycle Δ=0 (max 5 cycles).
//   §2 — SA-PT 5 chains cold-shifted T_local {0.05, 0.10, 0.20, 0.50, 1.00}.
//        Move set: B/R/P1/Q1 (locales) + W (grade promote/demote) + Move S
//        (slot-swap, 36-dim space, 630 pairs).
//        Mass {B:15, R:15, P1:35, Q1:15, W:10, S:10}. Refined via MIX_RECAL.
//        PT swap MH authentic period 1500. Tabu hash-100. Stagnation 8000.
//        AUTO-EXTEND laxo. 200k iters base.
//   §3 — Mini-greedy local rescue (top-3 pressure × bit-flip + a-replace +
//        grade promote/demote + Move S exhaustive vs all 35 others).
//   §4 — Polish 50-pass post-SA: bit-flip + a-replace + Move S exhaustive.
//   §5 — D190 quadruple verify (steps 1-3 in-process, step 4 deferred to
//        verify_F2_independent.py).
//
// Across N=4 seeds: track best_of_seeds, save dump from best.
//
// All Δ via AUDIT_CACHE.h v2 incremental (D172 binding). Audit cap discipline:
// INT32_MAX in greedy/polish (net-Δ semantics), metropolis_cap_from_T(T) in
// SA Metropolis (Opt5 active). D231 binding.
//
// iters_since_best discipline (D232): increment on EVERY proposal not
// improving best — rejects, tabu, apply-failures all increment. Single
// increment site inside step function.
//
// Compile:
//   cd ~/Downloads && g++ -O3 -march=native -std=c++17 -funroll-loops -Wall -Wextra togordoeldesheredado.cpp -o togordo17
// Run:
//   caffeinate -dims script -q ~/Downloads/togordo17_log.txt ./togordo17
// ============================================================================

#include "JK_BUILDER.h"
#include "ESTRELLA_GF2_KERNEL.h"
#include "AUDIT_CACHE.h"

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <random>
#include <cmath>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <climits>
#include <tuple>

using namespace estrella;

// ============================================================================
// Constants — Engine 17: all 36 audit dims flexible (NO frozen)
// ============================================================================

constexpr int N_FLEX = LAST_AUDIT_DIM - FIRST_AUDIT_DIM + 1;  // 36 dims
// FLEX_SLOTS = [2, 3, 4, ..., 37] — all audit dims, no exclusion
static int FLEX_SLOTS[N_FLEX];
static void init_flex_slots() {
    for (int i = 0; i < N_FLEX; ++i) FLEX_SLOTS[i] = FIRST_AUDIT_DIM + i;
}

// SA constants
constexpr int    N_CHAINS              = 5;
constexpr double T_LOCAL[N_CHAINS]     = { 0.05, 0.10, 0.20, 0.50, 1.00 };
constexpr int    PT_SWAP_PERIOD        = 1500;
constexpr int    TABU_CAP              = 100;
constexpr int    MIX_RECAL_WINDOW      = 20000;
constexpr int    STAGNATION_TRIGGER    = 8000;
constexpr int    AUTO_EXTEND_WINDOW    = 50000;
constexpr long   MAX_TOTAL_ITERS       = 1500000;
constexpr long   PHASE2_BASE_ITERS     = 200000;

// Multi-seed campaign size
constexpr int    N_SEEDS               = 4;

// Pre-SA params
constexpr int PASS1_TRIALS_PER_AC      = 30;
constexpr int PASS2_TOP_K_PRESSURE     = 24;
constexpr int PASS2_DUAL_TRIALS_PER_S  = 30;
constexpr int PASS3_TOP_K_TRIPLES      = 20;
constexpr int PASS3_TRIPLE_TRIALS      = 8;

// SA mass (sums to 100): B=15, R=15, P1=35, Q1=15, W=10, S=10
enum SAMove : int { MV_B = 0, MV_R = 1, MV_P1 = 2, MV_Q1 = 3, MV_W = 4, MV_S = 5 };
constexpr int N_SA_MOVES = 6;
constexpr int DEFAULT_MASS[N_SA_MOVES] = { 15, 15, 35, 15, 10, 10 };
constexpr int MASS_FLOOR = 5;
constexpr int MASS_CEIL  = 60;

// ============================================================================
// POLY pools s ∈ {1..6} — canonical small grades (verbatim Joe-Kuo)
// ============================================================================
constexpr uint16_t POLY1_A[] = { 0 };
constexpr uint16_t POLY2_A[] = { 1 };
constexpr uint16_t POLY3_A[] = { 1, 2 };
constexpr uint16_t POLY4_A[] = { 1, 4 };
constexpr uint16_t POLY5_A[] = { 2, 4, 7, 11, 13, 14 };
constexpr uint16_t POLY6_A[] = { 1, 13, 16, 19, 22, 25 };

inline int n_poly_for_s(int s) {
    switch (s) {
        case 1: return 1; case 2: return 1; case 3: return 2; case 4: return 2;
        case 5: return 6; case 6: return 6;
        case 7: return N_POLY7; case 8: return N_POLY8; case 9: return N_POLY9;
        default: return 0;
    }
}

inline uint32_t poly_a_at(int s, int idx) {
    switch (s) {
        case 1: return POLY1_A[idx]; case 2: return POLY2_A[idx];
        case 3: return POLY3_A[idx]; case 4: return POLY4_A[idx];
        case 5: return POLY5_A[idx]; case 6: return POLY6_A[idx];
        case 7: return (uint32_t)POLY7[idx].a;
        case 8: return (uint32_t)POLY8[idx].a;
        case 9: return (uint32_t)POLY9[idx].a;
        default: return 0;
    }
}

inline void random_m_init(std::mt19937& rng, int s, uint32_t m_out[SMAX]) {
    for (int k = 0; k < SMAX; ++k) m_out[k] = 0;
    if (s < 1) return;
    m_out[0] = 1;
    for (int k = 1; k < s; ++k) {
        uint32_t hi_half = 1u << k;
        uint32_t v = (rng() % hi_half) * 2u + 1u;
        m_out[k] = v;
    }
}

inline SlotState build_random_slot(std::mt19937& rng, int s) {
    SlotState slot;
    slot.s = (uint8_t)s;
    int n_pool = n_poly_for_s(s);
    if (n_pool > 0) slot.a = poly_a_at(s, (int)(rng() % n_pool));
    else            slot.a = 0;
    random_m_init(rng, s, slot.m);
    return slot;
}

// ============================================================================
// Pressure-directed slot picker (over 36 dims, no frozen exclusion)
// ============================================================================
static int pick_flex_pressure(const AuditState& st, std::mt19937& rng,
                              int exclude = -1) {
    double w[N_FLEX]; double total = 0.0;
    for (int i = 0; i < N_FLEX; ++i) {
        int d = FLEX_SLOTS[i];
        if (d == exclude) { w[i] = 0.0; continue; }
        double s = 0.0;
        for (int mi = 0; mi < NM; ++mi) s += (double)st.PAIR_T_max[d][mi];
        if (s < 1e-9) s = 1e-9;
        w[i] = s; total += s;
    }
    if (total <= 0.0) {
        for (int i = 0; i < N_FLEX; ++i) if (FLEX_SLOTS[i] != exclude) return FLEX_SLOTS[i];
        return FLEX_SLOTS[0];
    }
    double u = std::generate_canonical<double, 32>(rng) * total;
    double acc = 0.0;
    for (int i = 0; i < N_FLEX; ++i) {
        acc += w[i];
        if (u <= acc) return FLEX_SLOTS[i];
    }
    for (int i = N_FLEX - 1; i >= 0; --i) {
        if (FLEX_SLOTS[i] != exclude && w[i] > 0.0) return FLEX_SLOTS[i];
    }
    return FLEX_SLOTS[0];
}

// ============================================================================
// Single-slot apply
// ============================================================================
struct SingleOutcome {
    bool applied;
    int delta;
    CacheDiff diff;
};

static SingleOutcome apply_single(AuditState& st, int d,
                                   const SlotState& new_slot, int audit_cap) {
    SingleOutcome out{}; out.applied = false; out.delta = INT32_MAX;
    if (!validate_slot_invariants(new_slot)) return out;
    int ab = st.total_audit;
    out.diff.clear();
    out.diff.d_changed = d;
    out.diff.old_slot = st.slot[d];
    st.slot[d] = new_slot;
    bool ok = incremental_after_slot(st, d, audit_cap, out.diff);
    if (!ok) { revert_with_diff(st, out.diff); return out; }
    out.delta = st.total_audit - ab;
    out.applied = true;
    return out;
}

// ============================================================================
// Move S — slot swap (i, j): exchange (s, a, m) between two flex slots
// ============================================================================
struct MoveSOutcome {
    bool applied;
    int delta;
    int d_lo, d_hi;
    CacheDiff diff_lo, diff_hi;
};

static MoveSOutcome apply_move_s(AuditState& st, int d1, int d2, int audit_cap) {
    MoveSOutcome out{}; out.applied = false; out.delta = INT32_MAX;
    if (d1 == d2) return out;
    int d_lo = std::min(d1, d2);
    int d_hi = std::max(d1, d2);
    out.d_lo = d_lo; out.d_hi = d_hi;

    SlotState save_lo = st.slot[d_lo];
    SlotState save_hi = st.slot[d_hi];
    if (!validate_slot_invariants(save_hi)) return out;
    if (!validate_slot_invariants(save_lo)) return out;

    int audit_before = st.total_audit;

    out.diff_lo.clear();
    out.diff_lo.d_changed = d_lo;
    out.diff_lo.old_slot = save_lo;
    st.slot[d_lo] = save_hi;
    bool ok1 = incremental_after_slot(st, d_lo, audit_cap, out.diff_lo);
    if (!ok1) { revert_with_diff(st, out.diff_lo); return out; }

    out.diff_hi.clear();
    out.diff_hi.d_changed = d_hi;
    out.diff_hi.old_slot = save_hi;
    st.slot[d_hi] = save_lo;
    bool ok2 = incremental_after_slot(st, d_hi, audit_cap, out.diff_hi);
    if (!ok2) {
        revert_with_diff(st, out.diff_hi);
        revert_with_diff(st, out.diff_lo);
        return out;
    }
    out.delta = st.total_audit - audit_before;
    out.applied = true;
    return out;
}

static void revert_move_s(AuditState& st, const MoveSOutcome& mv) {
    if (!mv.applied) return;
    revert_with_diff(st, mv.diff_hi);
    revert_with_diff(st, mv.diff_lo);
}

// ============================================================================
// Move W — grade promote/demote at slot d: s_new = s_old ± 1
// ============================================================================
static SingleOutcome move_W(AuditState& st, std::mt19937& rng, int audit_cap) {
    int d = pick_flex_pressure(st, rng);
    int s_old = (int)st.slot[d].s;
    SingleOutcome out{}; out.applied = false; out.delta = INT32_MAX;
    int s_new;
    if (s_old <= 1)      s_new = s_old + 1;
    else if (s_old >= 9) s_new = s_old - 1;
    else                 s_new = s_old + ((rng() & 1u) ? 1 : -1);
    if (s_new < 1 || s_new > 9) return out;
    SlotState slot = build_random_slot(rng, s_new);
    return apply_single(st, d, slot, audit_cap);
}

// ============================================================================
// Move B / R / P1 / Q1 (locales)
// ============================================================================
static SingleOutcome move_B(AuditState& st, std::mt19937& rng, int audit_cap) {
    int d = pick_flex_pressure(st, rng);
    int s = (int)st.slot[d].s;
    SingleOutcome out{}; out.applied = false; out.delta = INT32_MAX;
    if (s < 2) return out;
    int k = 1 + (int)(rng() % (s - 1));
    uint32_t hi = 1u << (k + 1);
    int bit = 1 + (int)(rng() % k);
    SlotState slot = st.slot[d];
    uint32_t new_mk = slot.m[k] ^ (1u << bit);
    if (new_mk == 0 || new_mk >= hi || (new_mk & 1u) == 0) return out;
    slot.m[k] = new_mk;
    return apply_single(st, d, slot, audit_cap);
}

static SingleOutcome move_R(AuditState& st, std::mt19937& rng, int audit_cap) {
    int d = pick_flex_pressure(st, rng);
    int s = (int)st.slot[d].s;
    SingleOutcome out{}; out.applied = false; out.delta = INT32_MAX;
    if (s < 2) return out;
    int k = 1 + (int)(rng() % (s - 1));
    uint32_t hi_half = 1u << k;
    SlotState slot = st.slot[d];
    slot.m[k] = (rng() % hi_half) * 2u + 1u;
    return apply_single(st, d, slot, audit_cap);
}

static SingleOutcome move_Q1(AuditState& st, std::mt19937& rng, int audit_cap) {
    int d = pick_flex_pressure(st, rng);
    int s = (int)st.slot[d].s;
    SingleOutcome out{}; out.applied = false; out.delta = INT32_MAX;
    if (s < 1) return out;
    SlotState slot = build_random_slot(rng, s);
    return apply_single(st, d, slot, audit_cap);
}

// ============================================================================
// Tabu fingerprint
// ============================================================================
inline uint64_t fingerprint_state(const AuditState& st) {
    uint64_t h = 0xcbf29ce484222325ULL;
    auto mix = [&](uint64_t x){ h ^= x; h *= 0x100000001b3ULL; };
    for (int d = FIRST_AUDIT_DIM; d <= LAST_AUDIT_DIM; ++d) {
        const SlotState& s = st.slot[d];
        mix((uint64_t)s.s); mix((uint64_t)s.a);
        for (int k = 0; k < SMAX; ++k) mix((uint64_t)s.m[k]);
    }
    return h;
}

struct TabuRingBuffer {
    uint64_t buf[TABU_CAP]; int head; int size;
    TabuRingBuffer() : head(0), size(0) { for (int i = 0; i < TABU_CAP; ++i) buf[i] = 0; }
    bool contains(uint64_t fp) const {
        for (int i = 0; i < size; ++i) if (buf[i] == fp) return true;
        return false;
    }
    void push(uint64_t fp) {
        buf[head] = fp; head = (head + 1) % TABU_CAP;
        if (size < TABU_CAP) ++size;
    }
};

// ============================================================================
// Chain state
// ============================================================================
struct ChainState {
    AuditState   audit;
    int          best_audit;
    AuditState   best_state;
    int          iters_since_best;
    long         total_proposals;
    long         total_accepts;
    long         accepts_last_1000;
    long         proposals_last_1000;
    TabuRingBuffer tabu;
    long         proposals_by_move[N_SA_MOVES];
    long         events_by_move[N_SA_MOVES];
    int          mass[N_SA_MOVES];
};

static void init_chain(ChainState& c, const AuditState& warm) {
    c.audit = warm;
    c.best_audit = warm.total_audit;
    c.best_state = warm;
    c.iters_since_best = 0;
    c.total_proposals = c.total_accepts = 0;
    c.accepts_last_1000 = c.proposals_last_1000 = 0;
    c.tabu = TabuRingBuffer{};
    for (int i = 0; i < N_SA_MOVES; ++i) {
        c.proposals_by_move[i] = 0;
        c.events_by_move[i] = 0;
        c.mass[i] = DEFAULT_MASS[i];
    }
}

static int sample_move(const ChainState& c, std::mt19937& rng) {
    double bias[N_SA_MOVES]; double total = 0.0;
    for (int i = 0; i < N_SA_MOVES; ++i) { bias[i] = (double)c.mass[i]; total += bias[i]; }
    if (total <= 0.0) return 0;
    double u = std::generate_canonical<double, 32>(rng) * total;
    double cum = 0.0;
    for (int i = 0; i < N_SA_MOVES; ++i) {
        cum += bias[i];
        if (u <= cum) return i;
    }
    return 0;
}

static void mix_recal(ChainState& c) {
    double prod[N_SA_MOVES]; double total = 0.0;
    for (int i = 0; i < N_SA_MOVES; ++i) {
        prod[i] = (double)(c.events_by_move[i] + 1) /
                  (double)(c.proposals_by_move[i] + 10);
        total += prod[i];
    }
    if (total <= 0.0) return;
    int sum = 0;
    for (int i = 0; i < N_SA_MOVES; ++i) {
        double frac = prod[i] / total;
        int new_mass = (int)std::lround(frac * 100.0);
        if (new_mass < MASS_FLOOR) new_mass = MASS_FLOOR;
        if (new_mass > MASS_CEIL ) new_mass = MASS_CEIL;
        c.mass[i] = new_mass; sum += new_mass;
    }
    if (sum != 100 && sum > 0) {
        int imax = 0;
        for (int i = 1; i < N_SA_MOVES; ++i) if (c.mass[i] > c.mass[imax]) imax = i;
        c.mass[imax] += (100 - sum);
        if (c.mass[imax] < MASS_FLOOR) c.mass[imax] = MASS_FLOOR;
    }
    for (int i = 0; i < N_SA_MOVES; ++i) {
        c.proposals_by_move[i] = 0;
        c.events_by_move[i] = 0;
    }
}

inline bool metropolis_accept(int delta, double T, std::mt19937& rng) {
    if (delta <= 0) return true;
    double prob = std::exp(-(double)delta / T);
    return std::generate_canonical<double, 32>(rng) < prob;
}

inline int metropolis_cap_from_T(double T) {
    int cap = (int)std::ceil(T * 12.0);
    if (cap < 4) cap = 4;
    return cap;
}

static bool try_pt_swap(ChainState& ci, ChainState& cj,
                        double Ti, double Tj, std::mt19937& rng) {
    int Ei = ci.audit.total_audit;
    int Ej = cj.audit.total_audit;
    double bi = 1.0/Ti, bj = 1.0/Tj;
    double arg = (bi - bj) * (double)(Ei - Ej);
    double prob = std::min(1.0, std::exp(arg));
    if (std::generate_canonical<double, 32>(rng) < prob) {
        std::swap(ci.audit, cj.audit);
        if (ci.audit.total_audit < ci.best_audit) {
            ci.best_audit = ci.audit.total_audit; ci.best_state = ci.audit;
            ci.iters_since_best = 0;
        }
        if (cj.audit.total_audit < cj.best_audit) {
            cj.best_audit = cj.audit.total_audit; cj.best_state = cj.audit;
            cj.iters_since_best = 0;
        }
        return true;
    }
    return false;
}

// ============================================================================
// SA single step (D232 binding: iters_since_best on EVERY proposal)
// ============================================================================
static bool sa_step(ChainState& c, std::mt19937& rng, int chain_idx) {
    int idx = sample_move(c, rng);
    double T = T_LOCAL[chain_idx];
    int cap = metropolis_cap_from_T(T);

    int delta = 0;
    bool applied = false;
    MoveSOutcome S_out{};
    SingleOutcome single_out{};
    bool is_S = (idx == MV_S);

    if (is_S) {
        int d1 = pick_flex_pressure(c.audit, rng, -1);
        int d2 = pick_flex_pressure(c.audit, rng, d1);
        S_out = apply_move_s(c.audit, d1, d2, cap);
        applied = S_out.applied; delta = S_out.delta;
    } else if (idx == MV_B || idx == MV_P1) {
        single_out = move_B(c.audit, rng, cap);
        applied = single_out.applied; delta = single_out.delta;
    } else if (idx == MV_R) {
        single_out = move_R(c.audit, rng, cap);
        applied = single_out.applied; delta = single_out.delta;
    } else if (idx == MV_W) {
        single_out = move_W(c.audit, rng, cap);
        applied = single_out.applied; delta = single_out.delta;
    } else { // MV_Q1
        single_out = move_Q1(c.audit, rng, cap);
        applied = single_out.applied; delta = single_out.delta;
    }

    ++c.total_proposals; ++c.proposals_last_1000;
    ++c.proposals_by_move[idx];
    if (!applied) { ++c.iters_since_best; return false; }

    uint64_t fp = fingerprint_state(c.audit);
    if (c.tabu.contains(fp)) {
        if (is_S) revert_move_s(c.audit, S_out);
        else revert_with_diff(c.audit, single_out.diff);
        ++c.iters_since_best;
        return false;
    }
    if (delta <= -1) ++c.events_by_move[idx];

    if (!metropolis_accept(delta, T, rng)) {
        if (is_S) revert_move_s(c.audit, S_out);
        else revert_with_diff(c.audit, single_out.diff);
        ++c.iters_since_best;
        return false;
    }

    c.tabu.push(fp);
    ++c.total_accepts; ++c.accepts_last_1000;
    if (c.audit.total_audit < c.best_audit) {
        c.best_audit = c.audit.total_audit;
        c.best_state = c.audit;
        c.iters_since_best = 0;
    } else {
        ++c.iters_since_best;
    }
    return true;
}

// ============================================================================
// Mini-greedy local rescue (top-3 pressure × bit-flip + a-replace + S)
// ============================================================================
static int mini_greedy_rescue(ChainState& c) {
    double w[N_FLEX];
    for (int i = 0; i < N_FLEX; ++i) {
        int d = FLEX_SLOTS[i]; double s = 0.0;
        for (int mi = 0; mi < NM; ++mi) s += (double)c.audit.PAIR_T_max[d][mi];
        w[i] = s;
    }
    int order[N_FLEX]; for (int i = 0; i < N_FLEX; ++i) order[i] = i;
    std::sort(order, order + N_FLEX, [&](int a, int b){ return w[a] > w[b]; });
    int targets[3] = { FLEX_SLOTS[order[0]], FLEX_SLOTS[order[1]], FLEX_SLOTS[order[2]] };

    int audit_before = c.audit.total_audit;

    // bit-flip + a-replace exhaustive on top-3
    for (int dt = 0; dt < 3; ++dt) {
        int d = targets[dt];
        int s = (int)c.audit.slot[d].s;
        if (s < 2) continue;
        for (int k = 1; k < s; ++k) {
            uint32_t hi = 1u << (k + 1);
            for (int bit = 1; bit <= k; ++bit) {
                SlotState slot = c.audit.slot[d];
                uint32_t new_mk = slot.m[k] ^ (1u << bit);
                if (new_mk == 0 || new_mk >= hi || (new_mk & 1u) == 0) continue;
                slot.m[k] = new_mk;
                if (!validate_slot_invariants(slot)) continue;
                CacheDiff diff;
                diff.clear(); diff.d_changed = d; diff.old_slot = c.audit.slot[d];
                int ab = c.audit.total_audit;
                c.audit.slot[d] = slot;
                bool ok = incremental_after_slot(c.audit, d, INT32_MAX, diff);
                if (!ok) { revert_with_diff(c.audit, diff); continue; }
                if (c.audit.total_audit - ab >= 0) revert_with_diff(c.audit, diff);
            }
        }
        int n_pool = n_poly_for_s(s);
        for (int p = 0; p < n_pool; ++p) {
            uint32_t a_val = poly_a_at(s, p);
            if (a_val == c.audit.slot[d].a) continue;
            SlotState slot = c.audit.slot[d];
            slot.a = a_val;
            if (!validate_slot_invariants(slot)) continue;
            CacheDiff diff;
            diff.clear(); diff.d_changed = d; diff.old_slot = c.audit.slot[d];
            int ab = c.audit.total_audit;
            c.audit.slot[d] = slot;
            bool ok = incremental_after_slot(c.audit, d, INT32_MAX, diff);
            if (!ok) { revert_with_diff(c.audit, diff); continue; }
            if (c.audit.total_audit - ab >= 0) revert_with_diff(c.audit, diff);
        }
    }

    // Move S exhaustive between each target and ALL other flex slots
    for (int dt = 0; dt < 3; ++dt) {
        int d_target = targets[dt];
        for (int i = 0; i < N_FLEX; ++i) {
            int d_other = FLEX_SLOTS[i];
            if (d_other == d_target) continue;
            MoveSOutcome out = apply_move_s(c.audit, d_target, d_other, INT32_MAX);
            if (!out.applied) continue;
            if (out.delta >= 0) revert_move_s(c.audit, out);
        }
    }

    if (c.audit.total_audit < c.best_audit) {
        c.best_audit = c.audit.total_audit;
        c.best_state = c.audit;
    }
    c.iters_since_best = 0;
    return c.audit.total_audit - audit_before;
}

// ============================================================================
// FORENSIC pairs/triples — used as PRESSURE HYPOTHESIS, not mandatory cluster
// ============================================================================
static std::vector<std::pair<int,int>> forensic_pairs() {
    return {
        {33,34}, {32,33}, {32,34}, {31,33}, {31,34},
        {21,23}, {21,32}, {21,33}, {21,34},
        {23,32}, {23,33}, {23,34}
    };
}

// ============================================================================
// Pre-SA Phase 1 — F47 cross-substrate-NEW exhaustive greedy from JK_2008
// ============================================================================
static int pre_sa_pass1(AuditState& st, std::mt19937& rng) {
    int initial = st.total_audit;
    int total_subpasses = 0;
    bool any_global;
    do {
        any_global = false;
        for (int i = 0; i < N_FLEX; ++i) {
            int d = FLEX_SLOTS[i];
            // Try grades 1..9 for this slot (cold-start: any grade is candidate)
            for (int s = 1; s <= 9; ++s) {
                int n_pool = n_poly_for_s(s);
                for (int p = 0; p < n_pool; ++p) {
                    int trials = (s <= 4) ? 1 : PASS1_TRIALS_PER_AC;
                    SlotState best_cand{}; int best_delta = 0; bool found = false;
                    for (int trial = 0; trial < trials; ++trial) {
                        SlotState cand;
                        cand.s = (uint8_t)s;
                        cand.a = poly_a_at(s, p);
                        random_m_init(rng, s, cand.m);
                        if (!validate_slot_invariants(cand)) continue;
                        CacheDiff diff;
                        diff.clear(); diff.d_changed = d; diff.old_slot = st.slot[d];
                        int ab = st.total_audit;
                        st.slot[d] = cand;
                        bool ok = incremental_after_slot(st, d, INT32_MAX, diff);
                        if (!ok) { revert_with_diff(st, diff); continue; }
                        int delta = st.total_audit - ab;
                        if (delta < best_delta) {
                            best_cand = cand; best_delta = delta; found = true;
                        }
                        revert_with_diff(st, diff);
                    }
                    if (found) {
                        CacheDiff apply_diff;
                        apply_diff.clear();
                        apply_diff.d_changed = d;
                        apply_diff.old_slot = st.slot[d];
                        st.slot[d] = best_cand;
                        bool ok = incremental_after_slot(st, d, INT32_MAX, apply_diff);
                        if (ok) any_global = true;
                        else revert_with_diff(st, apply_diff);
                    }
                }
            }
        }
        ++total_subpasses;
    } while (any_global && total_subpasses < 20);

    int gain = initial - st.total_audit;
    printf("    [F47_NEW] Pass 1 cumulative Δ = %d  (audit %d -> %d, %d sub-passes)\n",
           -gain, initial, st.total_audit, total_subpasses); fflush(stdout);
    return gain;
}

static int pre_sa_pass2(AuditState& st, std::mt19937& rng) {
    int initial = st.total_audit;
    double w[N_FLEX];
    for (int i = 0; i < N_FLEX; ++i) {
        int d = FLEX_SLOTS[i]; double s = 0.0;
        for (int mi = 0; mi < NM; ++mi) s += (double)st.PAIR_T_max[d][mi];
        if (s < 1e-9) s = 1e-9;
        w[i] = s;
    }
    int order[N_FLEX]; for (int i = 0; i < N_FLEX; ++i) order[i] = i;
    std::sort(order, order + N_FLEX, [&](int a, int b){ return w[a] > w[b]; });
    int K = std::min(PASS2_TOP_K_PRESSURE, N_FLEX);

    std::vector<std::pair<int,int>> pairs;
    for (int a = 0; a < K; ++a) for (int b = a+1; b < K; ++b) {
        int da = FLEX_SLOTS[order[a]], db = FLEX_SLOTS[order[b]];
        if (da > db) std::swap(da, db);
        pairs.push_back({da, db});
    }
    auto fp = forensic_pairs();
    for (auto& pr : fp) {
        int a = pr.first, b = pr.second;
        if (a > b) std::swap(a, b);
        bool dup = false;
        for (auto& ex : pairs) if (ex.first == a && ex.second == b) { dup = true; break; }
        if (!dup) pairs.push_back({a, b});
    }

    int total_subpasses = 0;
    bool any_global;
    do {
        any_global = false;
        for (auto& pr : pairs) {
            int d1 = pr.first, d2 = pr.second;
            for (int s1 = 7; s1 <= 9; ++s1) {
                for (int s2 = 7; s2 <= 9; ++s2) {
                    SlotState best_n1{}, best_n2{}; int best_delta = 0; bool found = false;
                    for (int trial = 0; trial < PASS2_DUAL_TRIALS_PER_S; ++trial) {
                        SlotState n1 = build_random_slot(rng, s1);
                        SlotState n2 = build_random_slot(rng, s2);
                        if (!validate_slot_invariants(n1) || !validate_slot_invariants(n2)) continue;
                        int ab = st.total_audit;
                        CacheDiff dd1, dd2;
                        dd1.clear(); dd1.d_changed = d1; dd1.old_slot = st.slot[d1];
                        st.slot[d1] = n1;
                        bool ok1 = incremental_after_slot(st, d1, INT32_MAX, dd1);
                        if (!ok1) { revert_with_diff(st, dd1); continue; }
                        dd2.clear(); dd2.d_changed = d2; dd2.old_slot = st.slot[d2];
                        st.slot[d2] = n2;
                        bool ok2 = incremental_after_slot(st, d2, INT32_MAX, dd2);
                        if (!ok2) { revert_with_diff(st, dd2); revert_with_diff(st, dd1); continue; }
                        int delta = st.total_audit - ab;
                        if (delta < best_delta) {
                            best_n1 = n1; best_n2 = n2; best_delta = delta; found = true;
                        }
                        revert_with_diff(st, dd2); revert_with_diff(st, dd1);
                    }
                    if (found) {
                        CacheDiff dd1, dd2;
                        dd1.clear(); dd1.d_changed = d1; dd1.old_slot = st.slot[d1];
                        st.slot[d1] = best_n1;
                        bool ok1 = incremental_after_slot(st, d1, INT32_MAX, dd1);
                        if (!ok1) { revert_with_diff(st, dd1); continue; }
                        dd2.clear(); dd2.d_changed = d2; dd2.old_slot = st.slot[d2];
                        st.slot[d2] = best_n2;
                        bool ok2 = incremental_after_slot(st, d2, INT32_MAX, dd2);
                        if (!ok2) { revert_with_diff(st, dd2); revert_with_diff(st, dd1); continue; }
                        any_global = true;
                    }
                }
            }
        }
        ++total_subpasses;
    } while (any_global && total_subpasses < 8);

    int gain = initial - st.total_audit;
    printf("    [F47_NEW] Pass 2 cumulative Δ = %d  (audit %d -> %d, %d sub-passes)\n",
           -gain, initial, st.total_audit, total_subpasses); fflush(stdout);
    return gain;
}

static int pre_sa_pass3(AuditState& st, std::mt19937& rng) {
    int initial = st.total_audit;
    double w[N_FLEX];
    for (int i = 0; i < N_FLEX; ++i) {
        int d = FLEX_SLOTS[i]; double s = 0.0;
        for (int mi = 0; mi < NM; ++mi) s += (double)st.PAIR_T_max[d][mi];
        w[i] = s;
    }
    int order[N_FLEX]; for (int i = 0; i < N_FLEX; ++i) order[i] = i;
    std::sort(order, order + N_FLEX, [&](int a, int b){ return w[a] > w[b]; });
    int K = std::min(10, N_FLEX);

    std::vector<std::tuple<int,int,int>> triples;
    for (int a = 0; a < K && (int)triples.size() < PASS3_TOP_K_TRIPLES; ++a)
    for (int b = a+1; b < K && (int)triples.size() < PASS3_TOP_K_TRIPLES; ++b)
    for (int cc = b+1; cc < K && (int)triples.size() < PASS3_TOP_K_TRIPLES; ++cc) {
        int da = FLEX_SLOTS[order[a]], db = FLEX_SLOTS[order[b]], dc = FLEX_SLOTS[order[cc]];
        int v[3] = {da, db, dc}; std::sort(v, v+3);
        triples.push_back({v[0], v[1], v[2]});
    }
    triples.push_back({21, 33, 34});
    triples.push_back({31, 33, 34});

    for (auto& tr : triples) {
        int d1, d2, d3; std::tie(d1, d2, d3) = tr;
        SlotState best_n1{}, best_n2{}, best_n3{}; int best_delta = 0; bool found = false;
        for (int trial = 0; trial < PASS3_TRIPLE_TRIALS; ++trial) {
            int s1 = 7 + (int)(rng() % 3);
            int s2 = 7 + (int)(rng() % 3);
            int s3 = 7 + (int)(rng() % 3);
            SlotState n1 = build_random_slot(rng, s1);
            SlotState n2 = build_random_slot(rng, s2);
            SlotState n3 = build_random_slot(rng, s3);
            if (!validate_slot_invariants(n1) || !validate_slot_invariants(n2)
                || !validate_slot_invariants(n3)) continue;
            int ab = st.total_audit;
            CacheDiff D1, D2, D3;
            D1.clear(); D1.d_changed = d1; D1.old_slot = st.slot[d1];
            st.slot[d1] = n1;
            bool ok1 = incremental_after_slot(st, d1, INT32_MAX, D1);
            if (!ok1) { revert_with_diff(st, D1); continue; }
            D2.clear(); D2.d_changed = d2; D2.old_slot = st.slot[d2];
            st.slot[d2] = n2;
            bool ok2 = incremental_after_slot(st, d2, INT32_MAX, D2);
            if (!ok2) { revert_with_diff(st, D2); revert_with_diff(st, D1); continue; }
            D3.clear(); D3.d_changed = d3; D3.old_slot = st.slot[d3];
            st.slot[d3] = n3;
            bool ok3 = incremental_after_slot(st, d3, INT32_MAX, D3);
            if (!ok3) { revert_with_diff(st, D3); revert_with_diff(st, D2); revert_with_diff(st, D1); continue; }
            int delta = st.total_audit - ab;
            if (delta < best_delta) {
                best_n1 = n1; best_n2 = n2; best_n3 = n3; best_delta = delta; found = true;
            }
            revert_with_diff(st, D3); revert_with_diff(st, D2); revert_with_diff(st, D1);
        }
        if (found) {
            CacheDiff D1, D2, D3;
            D1.clear(); D1.d_changed = d1; D1.old_slot = st.slot[d1];
            st.slot[d1] = best_n1;
            bool ok1 = incremental_after_slot(st, d1, INT32_MAX, D1);
            if (!ok1) { revert_with_diff(st, D1); continue; }
            D2.clear(); D2.d_changed = d2; D2.old_slot = st.slot[d2];
            st.slot[d2] = best_n2;
            bool ok2 = incremental_after_slot(st, d2, INT32_MAX, D2);
            if (!ok2) { revert_with_diff(st, D2); revert_with_diff(st, D1); continue; }
            D3.clear(); D3.d_changed = d3; D3.old_slot = st.slot[d3];
            st.slot[d3] = best_n3;
            bool ok3 = incremental_after_slot(st, d3, INT32_MAX, D3);
            if (!ok3) { revert_with_diff(st, D3); revert_with_diff(st, D2); revert_with_diff(st, D1); continue; }
        }
    }
    int gain = initial - st.total_audit;
    printf("    [F47_NEW] Pass 3 cumulative Δ = %d  (audit %d -> %d)\n",
           -gain, initial, st.total_audit); fflush(stdout);
    return gain;
}

static int pre_sa_phase1(AuditState& st, std::mt19937& rng) {
    int initial_global = st.total_audit;
    int cycle = 0;
    while (cycle < 5) {
        ++cycle;
        printf("    [F47_NEW] === Cycle %d ===\n", cycle); fflush(stdout);
        int g1 = pre_sa_pass1(st, rng);
        int g2 = pre_sa_pass2(st, rng);
        int g3 = pre_sa_pass3(st, rng);
        if (g1 == 0 && g2 == 0 && g3 == 0) break;
    }
    int total_gain = initial_global - st.total_audit;
    printf("    [F47_NEW] Phase 1 total Δ = %d  (final audit = %d, %d cycles)\n",
           -total_gain, st.total_audit, cycle); fflush(stdout);
    return total_gain;
}

// ============================================================================
// Polish 50-pass: bit-flip + a-replace + Move S exhaustive on ALL 36 dims
// ============================================================================
static int polish_state(AuditState& st, int max_passes = 50) {
    int best = st.total_audit;
    for (int pass = 0; pass < max_passes; ++pass) {
        bool any_improve = false;

        for (int d = FIRST_AUDIT_DIM; d <= LAST_AUDIT_DIM; ++d) {
            int s = (int)st.slot[d].s;
            if (s < 2) continue;
            for (int k = 1; k < s; ++k) {
                uint32_t hi = 1u << (k + 1);
                for (int bit = 1; bit <= k; ++bit) {
                    SlotState slot = st.slot[d];
                    uint32_t new_mk = slot.m[k] ^ (1u << bit);
                    if (new_mk == 0 || new_mk >= hi || (new_mk & 1u) == 0) continue;
                    slot.m[k] = new_mk;
                    if (!validate_slot_invariants(slot)) continue;
                    CacheDiff diff;
                    diff.clear(); diff.d_changed = d; diff.old_slot = st.slot[d];
                    int ab = st.total_audit;
                    st.slot[d] = slot;
                    bool ok = incremental_after_slot(st, d, INT32_MAX, diff);
                    if (!ok) { revert_with_diff(st, diff); continue; }
                    if (st.total_audit - ab < 0) { any_improve = true; best = st.total_audit; }
                    else revert_with_diff(st, diff);
                }
            }
        }

        for (int d = FIRST_AUDIT_DIM; d <= LAST_AUDIT_DIM; ++d) {
            int s = (int)st.slot[d].s;
            int n_pool = n_poly_for_s(s);
            for (int p = 0; p < n_pool; ++p) {
                uint32_t a_val = poly_a_at(s, p);
                if (a_val == st.slot[d].a) continue;
                SlotState slot = st.slot[d];
                slot.a = a_val;
                if (!validate_slot_invariants(slot)) continue;
                CacheDiff diff;
                diff.clear(); diff.d_changed = d; diff.old_slot = st.slot[d];
                int ab = st.total_audit;
                st.slot[d] = slot;
                bool ok = incremental_after_slot(st, d, INT32_MAX, diff);
                if (!ok) { revert_with_diff(st, diff); continue; }
                if (st.total_audit - ab < 0) { any_improve = true; best = st.total_audit; }
                else revert_with_diff(st, diff);
            }
        }

        for (int di = FIRST_AUDIT_DIM; di <= LAST_AUDIT_DIM; ++di) {
            for (int dj = di + 1; dj <= LAST_AUDIT_DIM; ++dj) {
                MoveSOutcome out = apply_move_s(st, di, dj, INT32_MAX);
                if (!out.applied) continue;
                if (out.delta < 0) { any_improve = true; best = st.total_audit; }
                else revert_move_s(st, out);
            }
        }

        if (!any_improve) break;
    }
    return best;
}

// ============================================================================
// SA-PT main loop
// ============================================================================
struct SAResult {
    int best_audit;
    AuditState best_state;
    long total_iters;
    long total_proposals;
    long total_accepts;
    int num_pt_swaps;
    int num_rescues;
};

static SAResult run_sa_pt(const AuditState& warm, std::mt19937& rng,
                          long max_iters_base, int seed_idx) {
    ChainState chains[N_CHAINS];
    for (int c = 0; c < N_CHAINS; ++c) init_chain(chains[c], warm);

    int global_best = warm.total_audit;
    AuditState global_best_state = warm;
    long iter = 0;
    long iters_since_global_best = 0;
    long max_iters = max_iters_base;
    int num_pt_swaps = 0, num_rescues = 0;

    auto t_start = std::chrono::steady_clock::now();
    long last_log_iter = 0;

    while (iter < max_iters && iter < MAX_TOTAL_ITERS) {
        for (int c = 0; c < N_CHAINS; ++c) {
            sa_step(chains[c], rng, c);
            if (chains[c].best_audit < global_best) {
                global_best = chains[c].best_audit;
                global_best_state = chains[c].best_state;
                iters_since_global_best = 0;
            }
        }

        if (iter > 0 && iter % 1000 == 0) {
            for (int c = 0; c < N_CHAINS; ++c) {
                chains[c].accepts_last_1000 = 0;
                chains[c].proposals_last_1000 = 0;
            }
        }

        if (iter > 0 && iter % PT_SWAP_PERIOD == 0) {
            int i = (int)(rng() % (N_CHAINS - 1));
            if (try_pt_swap(chains[i], chains[i+1], T_LOCAL[i], T_LOCAL[i+1], rng)) {
                ++num_pt_swaps;
                if (chains[i].best_audit < global_best) {
                    global_best = chains[i].best_audit;
                    global_best_state = chains[i].best_state;
                    iters_since_global_best = 0;
                }
                if (chains[i+1].best_audit < global_best) {
                    global_best = chains[i+1].best_audit;
                    global_best_state = chains[i+1].best_state;
                    iters_since_global_best = 0;
                }
            }
        }

        if (iter > 0 && iter % MIX_RECAL_WINDOW == 0) {
            for (int c = 0; c < N_CHAINS; ++c) mix_recal(chains[c]);
        }

        for (int c = 0; c < N_CHAINS; ++c) {
            if (chains[c].iters_since_best >= STAGNATION_TRIGGER) {
                int rd = mini_greedy_rescue(chains[c]);
                ++num_rescues;
                if (rd < 0 && chains[c].best_audit < global_best) {
                    global_best = chains[c].best_audit;
                    global_best_state = chains[c].best_state;
                    iters_since_global_best = 0;
                }
            }
        }

        if (iter - last_log_iter >= 10000) {
            auto t_now = std::chrono::steady_clock::now();
            double sec = std::chrono::duration<double>(t_now - t_start).count();
            printf("    [SA seed %d] iter=%ld global_best=%d rescues=%d pt_swaps=%d %.0fs\n",
                   seed_idx, iter, global_best, num_rescues, num_pt_swaps, sec);
            for (int c = 0; c < N_CHAINS; ++c) {
                printf("      chain %d T=%.3f cur=%d best=%d acc=%ld/%ld mass[B/R/P1/Q1/W/S]=%d/%d/%d/%d/%d/%d\n",
                       c, T_LOCAL[c], chains[c].audit.total_audit, chains[c].best_audit,
                       chains[c].total_accepts, chains[c].total_proposals,
                       chains[c].mass[0], chains[c].mass[1], chains[c].mass[2],
                       chains[c].mass[3], chains[c].mass[4], chains[c].mass[5]);
            }
            fflush(stdout);
            last_log_iter = iter;
        }

        ++iters_since_global_best;
        if (iter == max_iters - 1) {
            if (iters_since_global_best < AUTO_EXTEND_WINDOW &&
                max_iters + AUTO_EXTEND_WINDOW <= MAX_TOTAL_ITERS) {
                max_iters += AUTO_EXTEND_WINDOW;
                printf("    [AUTO-EXTEND seed %d] %ld -> %ld\n",
                       seed_idx, iter+1, max_iters);
                fflush(stdout);
            }
        }

        ++iter;
    }

    SAResult r{};
    r.best_audit = global_best;
    r.best_state = global_best_state;
    r.total_iters = iter;
    long tp = 0, ta = 0;
    for (int c = 0; c < N_CHAINS; ++c) { tp += chains[c].total_proposals; ta += chains[c].total_accepts; }
    r.total_proposals = tp; r.total_accepts = ta;
    r.num_pt_swaps = num_pt_swaps;
    r.num_rescues = num_rescues;
    return r;
}

// ============================================================================
// D190 verify (steps 1-3)
// ============================================================================
static bool d190_step1(const AuditState& runtime_st) {
    AuditState fresh; std::memset(&fresh, 0, sizeof(fresh));
    std::memcpy(fresh.slot, runtime_st.slot, sizeof(fresh.slot));
    if (!validate_all_slots(fresh.slot)) {
        printf("  [D190 step 1] STRUCTURAL FAIL\n"); return false;
    }
    full_audit_rebuild(fresh);
    if (fresh.total_audit != runtime_st.total_audit) {
        printf("  [D190 step 1] FAIL runtime=%d cold=%d\n",
               runtime_st.total_audit, fresh.total_audit); return false;
    }
    printf("  [D190 step 1] FINAL_VERIFY PASS (%d)\n", fresh.total_audit);
    return true;
}

static bool d190_step2(const AuditState& runtime_st, const std::string& path) {
    std::ofstream out(path);
    if (!out) return false;
    out << "# Engine 17 dump\n# d s a m_1 m_2 ... m_s\n";
    for (int d = FIRST_AUDIT_DIM; d <= LAST_AUDIT_DIM; ++d) {
        const SlotState& s = runtime_st.slot[d];
        out << d << " " << (int)s.s << " " << s.a;
        for (int k = 0; k < (int)s.s; ++k) out << " " << s.m[k];
        out << "\n";
    }
    out.close();

    SlotState parsed[MAXDIM]; std::memset(parsed, 0, sizeof(parsed));
    std::ifstream in(path);
    if (!in) return false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        int d, s_in; uint32_t a_in;
        iss >> d >> s_in >> a_in;
        if (d < FIRST_AUDIT_DIM || d > LAST_AUDIT_DIM) continue;
        SlotState& slot = parsed[d];
        slot.s = (uint8_t)s_in; slot.a = a_in;
        for (int k = 0; k < SMAX; ++k) slot.m[k] = 0;
        for (int k = 0; k < s_in; ++k) {
            uint32_t mv; if (!(iss >> mv)) break;
            slot.m[k] = mv;
        }
    }
    parsed[1] = runtime_st.slot[1];
    if (!validate_all_slots(parsed)) {
        printf("  [D190 step 2] FAIL parsed invalid\n"); return false;
    }
    AuditState rebuilt; std::memset(&rebuilt, 0, sizeof(rebuilt));
    std::memcpy(rebuilt.slot, parsed, sizeof(rebuilt.slot));
    full_audit_rebuild(rebuilt);
    if (rebuilt.total_audit != runtime_st.total_audit) {
        printf("  [D190 step 2] FAIL runtime=%d parsed=%d\n",
               runtime_st.total_audit, rebuilt.total_audit); return false;
    }
    printf("  [D190 step 2] ROUNDTRIP PASS (%s)\n", path.c_str());
    return true;
}

static bool d190_step3(const AuditState& st) {
    if (!validate_all_slots(st.slot)) {
        printf("  [D190 step 3] FAIL invariants\n"); return false;
    }
    printf("  [D190 step 3] STRUCTURAL PASS (frozen-check OMITTED — Engine 17 cold cross-genealogy)\n");
    return true;
}

// ============================================================================
// Single seed run
// ============================================================================
struct SeedResult {
    int    seed_idx;
    int    final_audit;
    int    pre_sa_audit;
    int    sa_audit;
    int    polished_audit;
    bool   d190_ok;
    long   total_iters;
    int    num_rescues;
    int    num_pt_swaps;
    AuditState best_state;
};

static SeedResult run_one_seed(int seed_idx, uint64_t seed_value) {
    SeedResult result{};
    result.seed_idx = seed_idx;

    auto t0 = std::chrono::steady_clock::now();
    std::mt19937 rng((uint32_t)seed_value ^ (uint32_t)(seed_value >> 32));

    printf("\n  ========== SEED %d / %d ==========\n", seed_idx + 1, N_SEEDS);
    printf("  RNG seed: 0x%016llX\n", (unsigned long long)seed_value);

    // Cold-start from JK_2008 baseline
    AuditState st; std::memset(&st, 0, sizeof(st));
    load_jk_2008(st.slot);
    if (!validate_all_slots(st.slot)) { printf("  ABORT: JK_2008 invariants\n"); return result; }
    full_audit_rebuild(st);
    if (st.total_audit != 3196) {
        printf("  ABORT: JK_2008 audit=%d (expected 3196)\n", st.total_audit);
        return result;
    }
    printf("  Cold-start: JK_2008 audit=3196 PASS\n");

    // Phase 1
    printf("  [Phase 1] Pre-SA F47 cross-substrate-NEW exhaustive (no frozen)...\n");
    fflush(stdout);
    auto tP1 = std::chrono::steady_clock::now();
    pre_sa_phase1(st, rng);
    double secP1 = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - tP1).count();
    result.pre_sa_audit = st.total_audit;
    printf("  Phase 1 audit=%d (%.0fs)\n", st.total_audit, secP1);

    // Phase 2 SA-PT
    printf("  [Phase 2] SA-PT 5 chains, 6-move dispatcher (B/R/P1/Q1/W/S)...\n");
    fflush(stdout);
    auto tP2 = std::chrono::steady_clock::now();
    SAResult sar = run_sa_pt(st, rng, PHASE2_BASE_ITERS, seed_idx);
    double secP2 = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - tP2).count();
    result.sa_audit = sar.best_audit;
    result.total_iters = sar.total_iters;
    result.num_rescues = sar.num_rescues;
    result.num_pt_swaps = sar.num_pt_swaps;
    printf("  Phase 2 best=%d (iters %ld, accepts %ld/%ld, PT %d, rescues %d, %.0fs)\n",
           sar.best_audit, sar.total_iters, sar.total_accepts,
           sar.total_proposals, sar.num_pt_swaps, sar.num_rescues, secP2);

    // Phase 5 Polish
    printf("  [Phase 5] Polish 50-pass (all 36 dims, no frozen)...\n");
    fflush(stdout);
    auto tP5 = std::chrono::steady_clock::now();
    AuditState st_pol = sar.best_state;
    int polished = polish_state(st_pol, 50);
    double secP5 = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - tP5).count();
    result.polished_audit = polished;
    printf("  Polish best=%d (Δ from SA=%d, %.0fs)\n",
           polished, polished - sar.best_audit, secP5);

    // Phase 6 D190
    char dump_path[64];
    std::snprintf(dump_path, sizeof(dump_path), "togordo17_seed%d_dump.txt", seed_idx);
    printf("  [Phase 6] D190 verify...\n"); fflush(stdout);
    bool s1 = d190_step1(st_pol);
    bool s2 = d190_step2(st_pol, dump_path);
    bool s3 = d190_step3(st_pol);
    result.d190_ok = s1 && s2 && s3;
    printf("  D190 (steps 1-3): %s\n", result.d190_ok ? "PASS" : "FAIL");

    result.final_audit = st_pol.total_audit;
    result.best_state = st_pol;

    double sec_total = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - t0).count();
    printf("  Seed %d finished: audit=%d (%.1f h)\n",
           seed_idx + 1, result.final_audit, sec_total / 3600.0);
    fflush(stdout);

    return result;
}

// ============================================================================
// MAIN — multi-seed campaign
// ============================================================================
int main() {
    auto t_global = std::chrono::steady_clock::now();

    init_flex_slots();

    printf("====================================================================\n");
    printf("  Engine 17: TOGORDOELDESHEREDADO  (cold-start cross-genealogy)\n");
    printf("  Constructor v53 · 9 May 2026 · F_2 · target sub-3027 from cold\n");
    printf("  Cold-start from JK_2008 (3196), NO frozen, %d seeds in serial\n", N_SEEDS);
    printf("====================================================================\n");

    uint64_t base_seed = (uint64_t)std::chrono::system_clock::now().time_since_epoch().count();
    base_seed ^= 0xDE5E1EDADA0E51DEULL;
    printf("  Base seed: 0x%016llX\n", (unsigned long long)base_seed);
    printf("  Total seeds: %d\n\n", N_SEEDS);

    SeedResult results[N_SEEDS];
    for (int s = 0; s < N_SEEDS; ++s) {
        uint64_t sv = base_seed ^ (((uint64_t)(s + 1)) * 0x9E3779B97F4A7C15ULL);
        results[s] = run_one_seed(s, sv);
    }

    // Summary
    int best_idx = 0;
    int best_audit = results[0].final_audit;
    for (int s = 1; s < N_SEEDS; ++s) {
        if (results[s].final_audit < best_audit) {
            best_audit = results[s].final_audit;
            best_idx = s;
        }
    }

    double sec_total = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - t_global).count();

    printf("\n====================================================================\n");
    printf("  Engine 17 CAMPAIGN COMPLETE — runtime %.2f h\n", sec_total / 3600.0);
    printf("====================================================================\n");
    printf("  Per-seed results:\n");
    printf("  | seed | pre_sa | sa    | polish | d190 |\n");
    printf("  |------|--------|-------|--------|------|\n");
    for (int s = 0; s < N_SEEDS; ++s) {
        printf("  |  %d   | %4d   | %4d  | %4d   | %s |\n",
               s + 1, results[s].pre_sa_audit, results[s].sa_audit,
               results[s].polished_audit, results[s].d190_ok ? "PASS" : "FAIL");
    }
    printf("\n  Best seed: %d with audit=%d (dump: togordo17_seed%d_dump.txt)\n",
           best_idx + 1, best_audit, best_idx);
    printf("  Δ vs JK_2008 (3196):  %d (%.2f%%)\n",
           best_audit - 3196, 100.0 * (best_audit - 3196) / 3196.0);
    printf("  Δ vs COMBO_3027:      %d\n", best_audit - 3027);

    if (best_audit < 3027) {
        printf("\n  *** RECORD: Engine 17 beats COMBO_3027 from cold cross-genealogy ***\n");
    } else if (best_audit == 3027) {
        printf("\n  COMBO basin re-discovered from cold — strong evidence COMBO is structural attractor.\n");
    } else {
        printf("\n  Cold-start finds different (worse) basin — F_2 landscape data.\n");
    }
    printf("  D190 step 4 (Python independent): DEFERRED. Run verify_F2_independent.py on best dump.\n");
    printf("====================================================================\n");

    printf("\nENGINE17_FINAL: best_audit=%d best_seed=%d delta_jk=%d delta_combo=%d\n",
           best_audit, best_idx + 1, best_audit - 3196, best_audit - 3027);
    return 0;
}
