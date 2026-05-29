// JK_BUILDER.h — Joe-Kuo 2008 spec witnesses + primitive polynomial pools + dump I/O
//
// Engine 13: ELCHUMINEROASESINEROSALELOCODELGRANERO — Phase 1 Kernel
// Constructor v53 (Cojones Rectos), 8 May 2026
// Phase 1 file 1 of 5. Provides:
//   - JK_2008 spec for dims 2..37 (audit witness 3196).
//   - COMBO_3027 spec for dims 2..37 (audit witness 3027, project ratified warm-start).
//   - Primitive polynomial pools POLY7[18], POLY8[16], POLY9[48].
//   - Dump format parser/serializer compatible with TOGORDO_COMBO_v1_RECORD.txt.
//
// Sources (numbers from files only, D44/D81 binding):
//   - new-joe-kuo-6_21201.md §3 (JK 2008 dims 2..37).
//   - TOGORDO_COMBO_v1_RECORD.txt (COMBO_3027 19 April 2026 ratified).
//   - primitive_polynomials_deg8.md (16 deg-8 primitives).
//   - primitive_polynomials_deg9.md (48 deg-9 primitives).
//
// No incremental cache here. No audit computation here. This file provides
// canonical inputs to the kernel + cache + sanity instrument.

#ifndef ESTRELLA_JK_BUILDER_H
#define ESTRELLA_JK_BUILDER_H

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

namespace estrella {

// ============================================================================
// §1 — Constants matching KERNEL_ASESINO_F2_DESIGN_v4.md §2
// ============================================================================

constexpr int MAXBITS = 32;     // 32 cols/rows of generator matrix (canonical)
                                // Audit uses m in [MLO, MHI] = [5, 25]; rows 26..31
                                // never contribute to rank checks but stored for canonical
                                // alignment with TOGORDOBIENGORDO_v2 reference.
constexpr int MAXDIM  = 38;     // dims 2..37 + index alignment
constexpr int MLO     = 5;      // audit lower resolution
constexpr int MHI     = 25;     // audit upper resolution
constexpr int NM      = MHI - MLO + 1;  // 21 m-values per dim
constexpr int SMAX    = 9;      // max primitive degree (Engine 13: SMAX=9 for D9x2)
constexpr int MAXDN   = 32;     // direction numbers per dim

constexpr int FIRST_AUDIT_DIM = 2;
constexpr int LAST_AUDIT_DIM  = 37;
constexpr int N_AUDIT_DIMS    = LAST_AUDIT_DIM - FIRST_AUDIT_DIM + 1;  // 36

// ============================================================================
// §2 — SlotState (forward-declared canonical struct, full def in KERNEL.h)
// ============================================================================
//
// We declare it here so JK_BUILDER can construct/return SlotState arrays.
// Full canonical definition lives in ESTRELLA_GF2_KERNEL.h. They must match.

struct SlotState {
    uint8_t  s;           // primitive degree, 1..SMAX
    uint32_t a;           // primitive intermediate coefficients
    uint32_t m[SMAX];     // direction-number initials (m[0]=m_1=1, m[k]<2^(k+1), odd)
                          // Canonicalization (CACHE_INCREMENTAL §K4): m[k]=0 for k>=s
};

// ============================================================================
// §3 — Primitive polynomial pools
// ============================================================================
//
// Per primitive_polynomials_deg{7,8,9}.md.
// Each entry is a (s, a) pair encoding x^s + a_{s-1}x^{s-1} + ... + a_1 x + 1
// where the binary expansion of `a` (s-1 bits, MSB=a_{s-1}) gives middle coeffs.

struct PrimPoly {
    uint8_t s;
    uint16_t a;
};

// POLY7[18]: deg-7 primitives currently used by JK 2008 dims 20..37.
// Derived from the 18 distinct (s=7, a) pairs in new-joe-kuo-6_21201.md §3 dims 20..37.
constexpr PrimPoly POLY7[] = {
    {7,   1}, {7,   4}, {7,   7}, {7,   8}, {7,  14}, {7,  19},
    {7,  21}, {7,  28}, {7,  31}, {7,  32}, {7,  37}, {7,  41},
    {7,  42}, {7,  50}, {7,  55}, {7,  56}, {7,  59}, {7,  62}
};
constexpr int N_POLY7 = sizeof(POLY7) / sizeof(POLY7[0]);
static_assert(N_POLY7 == 18, "POLY7 must have 18 entries");

// POLY8[16]: deg-8 primitives per primitive_polynomials_deg8.md §3.
constexpr PrimPoly POLY8[] = {
    {8,  14}, {8,  21}, {8,  22}, {8,  38}, {8,  47}, {8,  49},
    {8,  50}, {8,  52}, {8,  56}, {8,  67}, {8,  70}, {8,  84},
    {8,  97}, {8, 103}, {8, 115}, {8, 122}
};
constexpr int N_POLY8 = sizeof(POLY8) / sizeof(POLY8[0]);
static_assert(N_POLY8 == 16, "POLY8 must have 16 entries");

// POLY9[48]: deg-9 primitives per primitive_polynomials_deg9.md §3.
// Engine 13 NEW: never tested in F_2 era. Move D9x2 candidate (probe-before-commit).
constexpr PrimPoly POLY9[] = {
    {9,   8}, {9,  13}, {9,  16}, {9,  22}, {9,  25}, {9,  44},
    {9,  47}, {9,  52}, {9,  55}, {9,  59}, {9,  62}, {9,  67},
    {9,  74}, {9,  81}, {9,  82}, {9,  87}, {9,  91}, {9,  94},
    {9, 103}, {9, 104}, {9, 109}, {9, 122}, {9, 124}, {9, 137},
    {9, 138}, {9, 143}, {9, 145}, {9, 152}, {9, 157}, {9, 167},
    {9, 173}, {9, 176}, {9, 181}, {9, 182}, {9, 185}, {9, 191},
    {9, 194}, {9, 199}, {9, 218}, {9, 220}, {9, 227}, {9, 229},
    {9, 230}, {9, 234}, {9, 236}, {9, 241}, {9, 244}, {9, 253}
};
constexpr int N_POLY9 = sizeof(POLY9) / sizeof(POLY9[0]);
static_assert(N_POLY9 == 48, "POLY9 must have 48 entries");

// ============================================================================
// §4 — JK_2008 spec for audit witness 3196
// ============================================================================
//
// Source: new-joe-kuo-6_21201.md §3 lines 58-94, dims 2..37 verbatim.
// Embedded as constexpr literals (per design decision D-jk-builder-1):
// D44/D81 binding satisfied — values verifiable byte-exact against archive
// in code review. Parsing from file would introduce format-fragility.

struct JKDimSpec {
    uint8_t  s;
    uint16_t a;
    uint32_t m_init[SMAX];  // m_init[0..s-1] valid, m_init[k>=s] padded 0
};

constexpr JKDimSpec JK_2008[N_AUDIT_DIMS] = {
    // d=2:  s=1 a=0   m={1}
    { 1,   0, { 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
    // d=3:  s=2 a=1   m={1,3}
    { 2,   1, { 1, 3, 0, 0, 0, 0, 0, 0, 0 } },
    // d=4:  s=3 a=1   m={1,3,1}
    { 3,   1, { 1, 3, 1, 0, 0, 0, 0, 0, 0 } },
    // d=5:  s=3 a=2   m={1,1,1}
    { 3,   2, { 1, 1, 1, 0, 0, 0, 0, 0, 0 } },
    // d=6:  s=4 a=1   m={1,1,3,3}
    { 4,   1, { 1, 1, 3, 3, 0, 0, 0, 0, 0 } },
    // d=7:  s=4 a=4   m={1,3,5,13}
    { 4,   4, { 1, 3, 5, 13, 0, 0, 0, 0, 0 } },
    // d=8:  s=5 a=2   m={1,1,5,5,17}
    { 5,   2, { 1, 1, 5, 5, 17, 0, 0, 0, 0 } },
    // d=9:  s=5 a=4   m={1,1,5,5,5}
    { 5,   4, { 1, 1, 5, 5, 5, 0, 0, 0, 0 } },
    // d=10: s=5 a=7   m={1,1,7,11,19}
    { 5,   7, { 1, 1, 7, 11, 19, 0, 0, 0, 0 } },
    // d=11: s=5 a=11  m={1,1,5,1,1}
    { 5,  11, { 1, 1, 5, 1, 1, 0, 0, 0, 0 } },
    // d=12: s=5 a=13  m={1,1,1,3,11}
    { 5,  13, { 1, 1, 1, 3, 11, 0, 0, 0, 0 } },
    // d=13: s=5 a=14  m={1,3,5,5,31}
    { 5,  14, { 1, 3, 5, 5, 31, 0, 0, 0, 0 } },
    // d=14: s=6 a=1   m={1,3,3,9,7,49}
    { 6,   1, { 1, 3, 3, 9, 7, 49, 0, 0, 0 } },
    // d=15: s=6 a=13  m={1,1,1,15,21,21}
    { 6,  13, { 1, 1, 1, 15, 21, 21, 0, 0, 0 } },
    // d=16: s=6 a=16  m={1,3,1,13,27,49}
    { 6,  16, { 1, 3, 1, 13, 27, 49, 0, 0, 0 } },
    // d=17: s=6 a=19  m={1,1,1,15,7,5}
    { 6,  19, { 1, 1, 1, 15, 7, 5, 0, 0, 0 } },
    // d=18: s=6 a=22  m={1,3,1,15,13,25}
    { 6,  22, { 1, 3, 1, 15, 13, 25, 0, 0, 0 } },
    // d=19: s=6 a=25  m={1,1,5,5,19,61}
    { 6,  25, { 1, 1, 5, 5, 19, 61, 0, 0, 0 } },
    // d=20: s=7 a=1   m={1,3,7,11,23,15,103}
    { 7,   1, { 1, 3, 7, 11, 23, 15, 103, 0, 0 } },
    // d=21: s=7 a=4   m={1,3,7,13,13,15,69}
    { 7,   4, { 1, 3, 7, 13, 13, 15, 69, 0, 0 } },
    // d=22: s=7 a=7   m={1,1,3,13,7,35,63}
    { 7,   7, { 1, 1, 3, 13, 7, 35, 63, 0, 0 } },
    // d=23: s=7 a=8   m={1,3,5,9,1,25,53}
    { 7,   8, { 1, 3, 5, 9, 1, 25, 53, 0, 0 } },
    // d=24: s=7 a=14  m={1,3,1,13,9,35,107}
    { 7,  14, { 1, 3, 1, 13, 9, 35, 107, 0, 0 } },
    // d=25: s=7 a=19  m={1,3,1,5,27,61,31}
    { 7,  19, { 1, 3, 1, 5, 27, 61, 31, 0, 0 } },
    // d=26: s=7 a=21  m={1,1,5,11,19,41,61}
    { 7,  21, { 1, 1, 5, 11, 19, 41, 61, 0, 0 } },
    // d=27: s=7 a=28  m={1,3,5,3,3,13,69}
    { 7,  28, { 1, 3, 5, 3, 3, 13, 69, 0, 0 } },
    // d=28: s=7 a=31  m={1,1,7,13,1,19,1}
    { 7,  31, { 1, 1, 7, 13, 1, 19, 1, 0, 0 } },
    // d=29: s=7 a=32  m={1,3,7,5,13,19,59}
    { 7,  32, { 1, 3, 7, 5, 13, 19, 59, 0, 0 } },
    // d=30: s=7 a=37  m={1,1,3,9,25,29,41}
    { 7,  37, { 1, 1, 3, 9, 25, 29, 41, 0, 0 } },
    // d=31: s=7 a=41  m={1,3,5,13,23,1,55}
    { 7,  41, { 1, 3, 5, 13, 23, 1, 55, 0, 0 } },
    // d=32: s=7 a=42  m={1,3,7,3,13,59,17}
    { 7,  42, { 1, 3, 7, 3, 13, 59, 17, 0, 0 } },
    // d=33: s=7 a=50  m={1,3,1,3,5,53,69}
    { 7,  50, { 1, 3, 1, 3, 5, 53, 69, 0, 0 } },
    // d=34: s=7 a=55  m={1,1,5,5,23,33,13}
    { 7,  55, { 1, 1, 5, 5, 23, 33, 13, 0, 0 } },
    // d=35: s=7 a=56  m={1,1,7,7,1,61,123}
    { 7,  56, { 1, 1, 7, 7, 1, 61, 123, 0, 0 } },
    // d=36: s=7 a=59  m={1,1,7,9,13,61,49}
    { 7,  59, { 1, 1, 7, 9, 13, 61, 49, 0, 0 } },
    // d=37: s=7 a=62  m={1,3,3,5,3,55,33}
    { 7,  62, { 1, 3, 3, 5, 3, 55, 33, 0, 0 } }
};

// ============================================================================
// §5 — COMBO_3027 spec for audit witness 3027
// ============================================================================
//
// Source: TOGORDO_COMBO_v1_RECORD.txt (19 April 2026, project F_2 record vigente).
// Embedded as constexpr literals same convention as JK_2008 §4.

constexpr JKDimSpec COMBO_3027[N_AUDIT_DIMS] = {
    // d=2:  s=1 a=0   m={1}
    { 1,   0, { 1, 0, 0, 0, 0, 0, 0, 0, 0 } },
    // d=3:  s=2 a=1   m={1,3}
    { 2,   1, { 1, 3, 0, 0, 0, 0, 0, 0, 0 } },
    // d=4:  s=3 a=1   m={1,3,1}
    { 3,   1, { 1, 3, 1, 0, 0, 0, 0, 0, 0 } },
    // d=5:  s=3 a=2   m={1,1,1}
    { 3,   2, { 1, 1, 1, 0, 0, 0, 0, 0, 0 } },
    // d=6:  s=4 a=1   m={1,1,3,13}
    { 4,   1, { 1, 1, 3, 13, 0, 0, 0, 0, 0 } },
    // d=7:  s=4 a=4   m={1,1,3,7}
    { 4,   4, { 1, 1, 3, 7, 0, 0, 0, 0, 0 } },
    // d=8:  s=5 a=7   m={1,3,5,1,29}
    { 5,   7, { 1, 3, 5, 1, 29, 0, 0, 0, 0 } },
    // d=9:  s=5 a=4   m={1,1,7,5,27}
    { 5,   4, { 1, 1, 7, 5, 27, 0, 0, 0, 0 } },
    // d=10: s=5 a=13  m={1,1,7,5,21}
    { 5,  13, { 1, 1, 7, 5, 21, 0, 0, 0, 0 } },
    // d=11: s=5 a=14  m={1,1,1,9,7}
    { 5,  14, { 1, 1, 1, 9, 7, 0, 0, 0, 0 } },
    // d=12: s=5 a=11  m={1,1,1,11,13}
    { 5,  11, { 1, 1, 1, 11, 13, 0, 0, 0, 0 } },
    // d=13: s=6 a=13  m={1,1,5,9,3,63}
    { 6,  13, { 1, 1, 5, 9, 3, 63, 0, 0, 0 } },
    // d=14: s=5 a=2   m={1,1,7,9,29}
    { 5,   2, { 1, 1, 7, 9, 29, 0, 0, 0, 0 } },
    // d=15: s=6 a=22  m={1,1,7,7,1,21}
    { 6,  22, { 1, 1, 7, 7, 1, 21, 0, 0, 0 } },
    // d=16: s=6 a=16  m={1,3,7,1,21,17}
    { 6,  16, { 1, 3, 7, 1, 21, 17, 0, 0, 0 } },
    // d=17: s=6 a=25  m={1,3,7,15,1,63}
    { 6,  25, { 1, 3, 7, 15, 1, 63, 0, 0, 0 } },
    // d=18: s=6 a=19  m={1,1,1,5,5,63}
    { 6,  19, { 1, 1, 1, 5, 5, 63, 0, 0, 0 } },
    // d=19: s=7 a=14  m={1,1,3,9,21,57,81}
    { 7,  14, { 1, 1, 3, 9, 21, 57, 81, 0, 0 } },
    // d=20: s=6 a=1   m={1,1,3,1,25,7}
    { 6,   1, { 1, 1, 3, 1, 25, 7, 0, 0, 0 } },
    // d=21: s=7 a=4   m={1,3,1,13,15,29,83}
    { 7,   4, { 1, 3, 1, 13, 15, 29, 83, 0, 0 } },
    // d=22: s=7 a=31  m={1,3,5,9,7,11,15}
    { 7,  31, { 1, 3, 5, 9, 7, 11, 15, 0, 0 } },
    // d=23: s=7 a=19  m={1,3,5,5,5,49,33}
    { 7,  19, { 1, 3, 5, 5, 5, 49, 33, 0, 0 } },
    // d=24: s=7 a=42  m={1,1,3,1,31,29,63}
    { 7,  42, { 1, 1, 3, 1, 31, 29, 63, 0, 0 } },
    // d=25: s=7 a=1   m={1,3,7,7,23,11,23}
    { 7,   1, { 1, 3, 7, 7, 23, 11, 23, 0, 0 } },
    // d=26: s=7 a=59  m={1,1,1,1,9,37,117}
    { 7,  59, { 1, 1, 1, 1, 9, 37, 117, 0, 0 } },
    // d=27: s=7 a=32  m={1,3,3,1,5,39,15}
    { 7,  32, { 1, 3, 3, 1, 5, 39, 15, 0, 0 } },
    // d=28: s=7 a=7   m={1,1,5,9,9,39,65}
    { 7,   7, { 1, 1, 5, 9, 9, 39, 65, 0, 0 } },
    // d=29: s=7 a=62  m={1,3,7,15,11,35,3}
    { 7,  62, { 1, 3, 7, 15, 11, 35, 3, 0, 0 } },
    // d=30: s=7 a=50  m={1,1,7,3,27,21,83}
    { 7,  50, { 1, 1, 7, 3, 27, 21, 83, 0, 0 } },
    // d=31: s=7 a=41  m={1,1,7,9,17,35,121}
    { 7,  41, { 1, 1, 7, 9, 17, 35, 121, 0, 0 } },
    // d=32: s=7 a=21  m={1,3,7,11,27,37,41}
    { 7,  21, { 1, 3, 7, 11, 27, 37, 41, 0, 0 } },
    // d=33: s=7 a=56  m={1,1,5,15,27,25,55}
    { 7,  56, { 1, 1, 5, 15, 27, 25, 55, 0, 0 } },
    // d=34: s=7 a=8   m={1,1,1,3,27,37,51}
    { 7,   8, { 1, 1, 1, 3, 27, 37, 51, 0, 0 } },
    // d=35: s=7 a=55  m={1,1,1,9,29,57,123}
    { 7,  55, { 1, 1, 1, 9, 29, 57, 123, 0, 0 } },
    // d=36: s=8 a=49  m={1,3,3,11,21,61,109,77}
    { 8,  49, { 1, 3, 3, 11, 21, 61, 109, 77, 0 } },
    // d=37: s=8 a=103 m={1,3,5,3,1,31,53,255}
    { 8, 103, { 1, 3, 5, 3, 1, 31, 53, 255, 0 } }
};

// ============================================================================
// §6 — Witness loaders
// ============================================================================
//
// Given a state array indexed by absolute dim d (where d in [0, MAXDIM)),
// fill slots[FIRST_AUDIT_DIM..LAST_AUDIT_DIM] from the spec. Slots outside
// the audit range (d=0, d=1) are zero-initialized.

inline void load_jk_2008(SlotState slots[MAXDIM]) {
    std::memset(slots, 0, sizeof(SlotState) * MAXDIM);
    for (int i = 0; i < N_AUDIT_DIMS; ++i) {
        int d = FIRST_AUDIT_DIM + i;
        const JKDimSpec& spec = JK_2008[i];
        slots[d].s = spec.s;
        slots[d].a = spec.a;
        for (int k = 0; k < SMAX; ++k) {
            slots[d].m[k] = spec.m_init[k];
        }
        // Canonicalization invariant: m[k>=s] = 0 (already 0 from spec, double-checked)
        for (int k = spec.s; k < SMAX; ++k) {
            slots[d].m[k] = 0;
        }
    }
}

inline void load_combo_3027(SlotState slots[MAXDIM]) {
    std::memset(slots, 0, sizeof(SlotState) * MAXDIM);
    for (int i = 0; i < N_AUDIT_DIMS; ++i) {
        int d = FIRST_AUDIT_DIM + i;
        const JKDimSpec& spec = COMBO_3027[i];
        slots[d].s = spec.s;
        slots[d].a = spec.a;
        for (int k = 0; k < SMAX; ++k) {
            slots[d].m[k] = spec.m_init[k];
        }
        for (int k = spec.s; k < SMAX; ++k) {
            slots[d].m[k] = 0;
        }
    }
}

// ============================================================================
// §7 — SlotState invariant validation (defensive)
// ============================================================================
//
// Verifies SlotState satisfies JK invariants per new-joe-kuo-6_21201.md §2:
//   - 1 <= s <= SMAX
//   - m[0] = 1 (m_1 = 1 always)
//   - m[k] is odd for k in [0, s)
//   - m[k] < 2^(k+1) for k in [0, s)
//   - m[k] = 0 for k in [s, SMAX) (canonicalization)
// Returns true if all invariants hold, false otherwise.

inline bool validate_slot_invariants(const SlotState& slot) {
    if (slot.s < 1 || slot.s > SMAX) return false;
    if (slot.m[0] != 1) return false;  // m_1 = 1 always
    for (int k = 0; k < (int)slot.s; ++k) {
        if ((slot.m[k] & 1u) == 0) return false;  // odd
        if (slot.m[k] >= (1u << (k + 1))) return false;  // m_{k+1} < 2^{k+1}
    }
    for (int k = (int)slot.s; k < SMAX; ++k) {
        if (slot.m[k] != 0) return false;  // canonicalization
    }
    return true;
}

inline bool validate_all_slots(const SlotState slots[MAXDIM]) {
    for (int d = FIRST_AUDIT_DIM; d <= LAST_AUDIT_DIM; ++d) {
        if (!validate_slot_invariants(slots[d])) return false;
    }
    return true;
}

// ============================================================================
// §8 — Dump format parser/serializer (anotación N: robust to actual format)
// ============================================================================
//
// Real format (TOGORDO_COMBO_v1_RECORD.txt observed):
//   "d= 2: s=1 a=  0 m: 1"
//   "d= 8: s=5 a=  7 m: 1 3 5 1 29"
//   "d=37: s=8 a=103 m: 1 3 5 3 1 31 53 255"
//
// Notes:
//   - 'd=' prefix, possible space pad inside number
//   - ':' separator after dim, before s
//   - 's=' prefix (no space inside)
//   - 'a=' prefix (possibly space-padded for alignment)
//   - 'm:' literal separator before m_init list
//   - whitespace tokenization between m values
//   - leading non-data lines (e.g. "THE AMICHIS CODE — ...", "Date: ...", "Audit: ...", blank)
//     are skipped if they don't match the dim-line pattern
//
// Parser uses tokenization on '=', ':', and whitespace; tolerant to padding.

namespace dump {

// Skip whitespace and optional padding around '=' and ':'
inline void skip_ws(const char*& p) {
    while (*p == ' ' || *p == '\t') ++p;
}

// Parse "key=value" pattern, skipping whitespace around '='. Returns true on success,
// stores parsed integer in *out_val. Advances *p past the value.
inline bool parse_key_eq_int(const char*& p, char key_char, uint32_t* out_val) {
    skip_ws(p);
    if (*p != key_char) return false;
    ++p;
    skip_ws(p);
    if (*p != '=') return false;
    ++p;
    skip_ws(p);
    char* endp = nullptr;
    long v = std::strtol(p, &endp, 10);
    if (endp == p) return false;
    if (v < 0) return false;
    *out_val = (uint32_t)v;
    p = endp;
    return true;
}

// Try to parse one dim line. Returns parsed dim index in *out_dim (0 if not a data line).
// Fills slot if valid. Returns true on successful parse, false if line is not a dim line
// (caller should skip and continue reading).
inline bool try_parse_dim_line(const std::string& line, int* out_dim, SlotState* out_slot) {
    const char* p = line.c_str();
    skip_ws(p);
    if (*p != 'd') return false;  // not a dim line

    uint32_t d_val = 0;
    if (!parse_key_eq_int(p, 'd', &d_val)) return false;
    if (d_val < (uint32_t)FIRST_AUDIT_DIM || d_val > (uint32_t)LAST_AUDIT_DIM) {
        // out-of-range dim — not a parse error, just skip this line
        return false;
    }

    skip_ws(p);
    if (*p != ':') return false;
    ++p;

    uint32_t s_val = 0;
    if (!parse_key_eq_int(p, 's', &s_val)) return false;
    if (s_val < 1 || s_val > (uint32_t)SMAX) return false;

    uint32_t a_val = 0;
    if (!parse_key_eq_int(p, 'a', &a_val)) return false;

    skip_ws(p);
    if (*p != 'm') return false;
    ++p;
    skip_ws(p);
    if (*p != ':') return false;
    ++p;

    // Read s integers as m_init
    SlotState slot;
    slot.s = (uint8_t)s_val;
    slot.a = a_val;
    for (int k = 0; k < SMAX; ++k) slot.m[k] = 0;
    for (uint32_t k = 0; k < s_val; ++k) {
        skip_ws(p);
        char* endp = nullptr;
        long mv = std::strtol(p, &endp, 10);
        if (endp == p) return false;
        if (mv <= 0) return false;
        slot.m[k] = (uint32_t)mv;
        p = endp;
    }

    if (!validate_slot_invariants(slot)) return false;

    *out_dim = (int)d_val;
    *out_slot = slot;
    return true;
}

// Parse a full dump file. Fills slots[FIRST_AUDIT_DIM..LAST_AUDIT_DIM].
// Returns true if all 36 audit dims were successfully parsed and validated.
// Non-dim lines (headers, blanks) are skipped silently.
// On any failure (file not found, missing dims, invalid invariants), returns false
// AND leaves slots[] zero-initialized so caller never sees uninitialized data.
inline bool parse_file(const std::string& filename, SlotState slots[MAXDIM]) {
    // Zero-init first so failure path leaves clean state (defensive).
    std::memset(slots, 0, sizeof(SlotState) * MAXDIM);

    std::ifstream in(filename);
    if (!in.is_open()) return false;

    bool seen[MAXDIM] = {false};

    std::string line;
    while (std::getline(in, line)) {
        int d = 0;
        SlotState slot;
        if (try_parse_dim_line(line, &d, &slot)) {
            if (d < FIRST_AUDIT_DIM || d > LAST_AUDIT_DIM) continue;
            slots[d] = slot;
            seen[d] = true;
        }
    }

    for (int d = FIRST_AUDIT_DIM; d <= LAST_AUDIT_DIM; ++d) {
        if (!seen[d]) {
            // Missing dim: zero-init again to ensure clean state on failure.
            std::memset(slots, 0, sizeof(SlotState) * MAXDIM);
            return false;
        }
    }
    if (!validate_all_slots(slots)) {
        std::memset(slots, 0, sizeof(SlotState) * MAXDIM);
        return false;
    }
    return true;
}

// Serialize slots to canonical dump format compatible with TOGORDO_COMBO_v1_RECORD.txt.
// Format: "d=NN: s=X a=YYY m: m_1 m_2 ... m_s\n" with right-aligned padding for d (2 chars)
// and a (3 chars), per observed format. m values space-separated, no trailing space.
//
// This serializer is the **canonical writer**. It is byte-exact compatible with itself
// (idempotent: serialize -> parse -> serialize gives byte-identical output, per
// KERNEL_ASESINO_F2_DESIGN_v4 §5.2 byte-exact round-trip assertion).
//
// NOTE: TOGORDO_COMBO_v1_RECORD.txt has a 4-line header before the dim lines. We do NOT
// reproduce that header in our canonical writer; the byte-exact round-trip assertion
// applies only to dim-lines section. If the original file's header is needed, caller
// can prepend it manually.

inline bool serialize_to_stream(std::ostream& out, const SlotState slots[MAXDIM]) {
    if (!validate_all_slots(slots)) return false;
    for (int d = FIRST_AUDIT_DIM; d <= LAST_AUDIT_DIM; ++d) {
        const SlotState& slot = slots[d];
        char dimbuf[8];
        std::snprintf(dimbuf, sizeof(dimbuf), "d=%2d", d);
        out << dimbuf << ": ";
        out << "s=" << (int)slot.s << " ";
        char abuf[16];
        std::snprintf(abuf, sizeof(abuf), "a=%3u", slot.a);
        out << abuf << " m:";
        for (int k = 0; k < (int)slot.s; ++k) {
            out << " " << slot.m[k];
        }
        out << "\n";
    }
    return true;
}

inline bool serialize_to_file(const std::string& filename, const SlotState slots[MAXDIM]) {
    std::ofstream out(filename);
    if (!out.is_open()) return false;
    return serialize_to_stream(out, slots);
}

inline bool serialize_to_string(std::string& out_str, const SlotState slots[MAXDIM]) {
    std::ostringstream oss;
    if (!serialize_to_stream(oss, slots)) return false;
    out_str = oss.str();
    return true;
}

}  // namespace dump

}  // namespace estrella

#endif  // ESTRELLA_JK_BUILDER_H
