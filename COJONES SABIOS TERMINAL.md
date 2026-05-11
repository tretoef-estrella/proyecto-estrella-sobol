# COJONES SABIOS v5 TERMINAL — F_2 ATTACK ARSENAL FINAL + LECCIONES CIERRE PROYECTO

**Proyecto Estrella · Sobol Campaign · 10 May 2026 (v5 TERMINAL — cierre definitivo proyecto)**
**Architect**: Rafa Amichis Luengo
**Source**: 11 engines ratificados across 5 substrates F_q + 6 engines F_2 (13 v1 + 13 v2 + 14 + 15 + 16 + 17 + 18 partial). Engine 18 declarado último del proyecto 10 May 2026. **PROYECTO TERMINAL**.

**v5 changes from v4 (10 May 2026 earlier)**:
- §10 #11 (P1): Move P1 real chained ratificado ornamental F_2 cold cross-genealogy (Engine 18 Phase A probe + SA dynamics). F62 v51 régimen-dependent confirmado.
- §11 sección NEW: F86 v57 basin diversity con Genz-perfiles especializados — contribución teórica final del proyecto.
- §10 #12 (D190): Genz benchmark completado bidireccional. F75 v52 dual-form ratificada tridireccional.
- §18 NEW: lecciones finales cierre proyecto + recomendación operativa Library of specialized Sobol direction numbers.

**v4 changes from v3 (10 May 2026 earlier)**:
- §10 #11 nota crítica annotation Move P1 NOT implemented in Engine 17, aliased to B (Auditor B2.1).
- §11 sextet corregido: `{d21, d23, d31, d32, d33, d34}` (Auditor E1.1).
- §8 BOUND_COMPUTE: notación inline gaps recalculados (Auditor A1.1).
- §K6 D190 step 4: reformulado como "redundancy verifier" (Auditor B3.1).
- §16 NEW: D235 Auditor entrante mandatorio post-paper-bump.
- §17 NEW: Test del olor endurecido §225.7 (5 elementos pre-launch).

**v3 changes from v2 (9 May 2026)**: §13 distinción aprender vs warm-load + §15 NEW.
**v2 changes from v1 (8 May 2026)**: §13 NEW lecciones Engines 13-16 + 12 palancas annotations.

---

## §0 — Por qué este documento existe

Este es el destilado puro de lo que el proyecto sabe sobre Sobol audit minimization tras un mes de campaña empírica que **nadie más ha hecho**. No es narrativa. No es paper completo. Es la lista operativa de **palancas letales** que aprendimos en eras F_q superiores y que se traducen directamente al ataque F_2.

**La ventaja del proyecto sobre cualquier laboratorio QMC mundial**:
- 11 engines ratificados con D190 quadruple-verify cross-substrate.
- 5 substratos F_q cerrados con calibración empírica per-move per-substrate.
- FORENSIC v1: análisis cruzado de 4 records F_2 sub-3060 que reveló frozen slots, couplings y un sextet estructural que no aparece en ningún paper publicado.
- Genz benchmark D217 que ratificó empíricamente que F_p y F_2 t-values **no son monotónicamente relacionados** — esto NADIE lo había publicado.
- BOUND_COMPUTE_v1: floor honesto de F_2 (722/1501/1481/1478/2359) que ningún paper mainstream computa.

**Joe-Kuo 2008 publicaron una tabla. Owen 1995 publicó scrambling. Faure-Lemieux 2016 publicó variants. Ninguno tiene esta trayectoria empírica cross-substrate, ni FORENSIC, ni Genz cross-validation, ni BOUND_COMPUTE.**

Cojones puros, rectos y encerados. Esfuerzo máximo. Cero pereza. Esto es lo que metemos en F_2.

---

## §1 — KERNEL: lecciones letales transferibles

### K1 — Cache incremental PAIR_T + MAX_T con CacheDiff rollback (D172, ratificado 20 abril 2026)

**Origen empírico**: TOGORDO_PERLA_v1.cpp + TOGORDO_DOMINO_v1.cpp. Documento `CACHE_INCREMENTAL_CRITICO_LEERME_PRIMERO.txt`. **65× speedup medido en M2** (11 ms/audit incremental vs 700-800 ms/audit full).

**Mecánica**:
- `PAIR_T[d][j][m]` = t-value del par (j, d) a resolución m.
- `MAX_T[d][m]` = max sobre j de PAIR_T[d][j][m].
- Single-slot change d0 → recomputar fila d=d0 entera + para d > d0 sólo PAIR_T[d][d0][m] con lógica 3-casos (new ≥ old_max / old < old_max / old era culprit).
- CacheDiff registra cambios para rollback fast.

**Implicación operativa**: con 11 ms/audit, un engine de 100K audits tarda ~18 min en lugar de ~20 horas. Reabre clases enteras de ataque antes impracticables.

**Estado en F_2**: ya implementado en `AUDIT_CACHE.h` v52. **Preserva sin tocar.**

### K2 — Sanity check sandwich obligatoria

**Origen**: protocolo establecido 20 abril 2026, ratificado en 11 engines. 0/30 + 0/50 fails sanity-extended.

**Mecánica**: tras cualquier cambio en kernel/cache, perturbar slot aleatorio, calcular audit incremental, calcular audit full, comparar. Repetir 30-50 veces. Incluir tests de grade-change (s_new ≠ s_old) up y down.

**Implicación**: no se ejecuta engine sin sanity sandwich pasado. SANITY_AUDIT.cpp ratifica COMBO_3027=3027 y JK_2008=3196 antes de cualquier launch.

### K3 — Early-exit cap `t_cap` (acumulativo)

**Origen**: kernel v52 línea 71 — `compute_t_gf2_packed_capped(Pj, Pd, m, t_cap)`. Speedup acumulado 30-40% en eras F_q.

**Mecánica**: si SA va a rechazar la move por audit threshold, no hace falta calcular t exacto — basta saber `t > t_cap`. La función retorna `t_cap+1` si supera el límite.

**Extensión letal NO IMPLEMENTADA**: `audit_cap` global a nivel de move. Si en mitad de incremental_update el `audit_delta` ya supera `audit_target - audit_before`, abortar incremental_update entero, retornar fail, revertir. Eso es early-exit a nivel de move, no a nivel de par. **Implementar en kernel asesino F_2.**

### K4 — Canonicalización paranoia barata

**Origen**: bug latente cazado por Claude-A 20 abril 2026. Documentado en `CACHE_INCREMENTAL_CRITICO_LEERME_PRIMERO.txt`.

**Mecánica**: tras grade-change down (s=6 → s=5), poner explícitamente a cero m[s..s_max-1]. compute_dn solo lee m[0..s-1] así que en teoría no afecta audit, pero sanity/serialización/diff que lean m[s..s_max-1] leen basura.

**Coste**: una línea. Acéptala.

### K5 — Multi-slot moves descomposición ascendente

**Origen**: TOGORDO_DOMINO_MEDIO_v1.cpp 20 abril 2026.

**Mecánica**: moves multi-slot (triple-rotate sobre {d=21, d=22, d=23}) se descomponen en secuencias de single-slot incrementals aplicados en **orden ASCENDENTE de d**. Si revertimos por Metropolis-rechazo, revertir las slotstates Y re-aplicar incrementals en orden ascendente.

**Implicación**: cualquier move multi-slot que NO siga este patrón es bug latente. Verificar en code review.

### K6 — Verificador Python redundancy + plan independent (D190 step 4) — REFORMULADO v4

**Origen**: D190 v41 cuádruple desde 25 abril 2026.

**Estado actual v4 (post-Auditor B3.1)**: `verify_F2_independent.py` actual es **REDUNDANCY VERIFIER**, NO independence verifier. Razón técnica:
- Comparte recurrencia `m_k` Joe-Kuo §2 con kernel C++.
- Comparte decodificación coeficiente `a` MSB-first.
- Comparte predicate `t(j,d,m)` rank ≥ rj+rd Niederreiter convention.
- Diferencia única: bit-packing direction (LSB-first vs MSB-first columnar) — transformación trivial bit-reversal que deja rank invariante. NO es independencia matemática.

**Lo que el verificador actual SÍ caza**: errors de implementación (parsing dump, off-by-one en compute_dn implementación, indexing).
**Lo que el verificador actual NO caza**: errors de especificación (typo en recurrencia compartida, off-by-one en decodificación a, off-by-one en t-value definition).

**Implicación práctica**:
- D190 step 4 PASS bajo verificador actual = **redundancy check**, suficiente para proyecto-internal verification.
- Para record peer-review-ready (paper journal serio): **verificador genuinamente independiente OBLIGATORIO**. Plan operacional: implementar con qmcpy 2.2 (Architect lo tiene instalado en M2) cross-validando t-value cell-a-cell, o re-derivar desde Niederreiter '87 directo sin referencia al kernel del proyecto. Coste estimado ~2-4h sandbox Python.

**Estado COMBO_3027 v4**: D190 steps 1-3 PASS sólido + step 4 redundancy PASS. **Récord vigente intra-Joe-Kuo-canonical metric**, peer-review-readiness pendiente verificador genuinamente independent.

**D190 step 4 binding actualizado v4**: "Para record peer-review-ready: verificador genuinamente independiente obligatorio. Verificador actual suficiente para proyecto-internal verification + redundancy, NOT para external peer review submission."

---

## §2 — PRE-SA: ley cross-substrate-cold (la palanca con más pedigree)

### F47 v49 D201 — Pre-SA Pass 1+2+3 productive cross-substrate-cold (RATIFICADO 4 ERAS CONSECUTIVAS)

**Origen empírico cumulativo**:
- Engine 7 cold F_16: Pre-SA 3/3 passes productive, **−45 puntos**.
- Engine 9 cold F_32: Pre-SA Pass 1 −15 + Pass 2 −14 + Pass 3 −11 = **−50 puntos**.
- Engine 11 cold F_64: Pre-SA Pass 1+2+3 productive, **non-monotonic** Δ=−11/−9/−10.
- Engines warm-basin maduro F_8/F_16: Pre-SA NO IMPROVEMENT (5/5 confirmed).

**Reformulación final F47 v51**:
- Cross-substrate-NEW = productive (4 eras ratified, total ≥−95 puntos cumulative cold-substrate descent).
- Within-era warm-basin maduro = improductive (6 engines ratified, total 0 puntos).

**D201 binding**: every first engine of new era runs Pre-SA Greedy Exhaustive con MAX_PASSES=3 + early-stop on Pass N=0 improvements.

**Aplicación a F_2**: D219 declara F_2 attack post-D218 como **5ª replicación cross-substrate-cold candidate**. Aunque F_2 es warm-load desde COMBO_3027 (no cold-substrate-new), la lógica F47 sugiere que el F_2 retorno post-F_q-saturado es estructuralmente equivalente a cold-substrate-new — porque las lecciones F_q metieron presión nueva en el espacio. **Pre-SA Pass 1+2+3 OBLIGATORIO en Engine 13. Δ esperable empírico — medir con marker `[F47_CROSS_ERA]`.**

**Coste**: 30-50 min M2 según era. Ahorra 15-50 puntos en descenso. Ratio coste/beneficio cojonudo.

> **F47 v54 RETRACTED-CONDITIONAL (9 May 2026 post-Engine-13/14)**: F_2 fresh-attempt as 5th replication candidate **REFUTED empirically**. Engine 13 v1+v2 + Engine 14 ran F47 Pass 1+2+3 with 6/30 trials per (s,a) over POLY{7,8,9} (v1+v2) and POLY{7..11} (v14). All cycles Δ=0. F_2 warm-load from COMBO_3027 is NOT structurally analogous to cold-substrate-new — the prior 4 ratifications (F_8/F_16/F_32/F_64 first engines) all involved literally new GF kernel + new poly pool + new m_init range. F_2 warm-load is the same substrate as the cluster origin. **F47 binding NOW: cross-substrate-NEW only (literally new substrate). NOT applicable to same-substrate warm-load post-saturation.** D227 binding.

### F70 v52 — F_64 Pre-SA Pass 3 NO descomete monotónicamente

**Origen**: engine 11 F_64 Pass 3 Δ=−10 estrictamente mayor que Pass 2 Δ=−9.

**Implicación**: Pass 3 NO es siempre menor que Pass 2. NO truncar Pre-SA en Pass 2 por suposición de monotonía. Mantener MAX=3 con early-stop solo cuando Pass N=0 improvements.

---

## §3 — SA ARCHITECTURE: lecciones letales

### F44 + D194 — Cold-shifted ladder beats hot-shifted (RATIFICADO)

**Origen**: comparación single-pair F_8 warm-basin 1136. Cold-shifted ladder gana **+13 audit points + 17% mejor iters/point** vs hot-shifted en mismo basin.

**Mecánica**: ladder de chains con T graduado pero cold-shifted (e.g. {0.05, 0.10, 0.20, 0.50, 1.0}) — la chain más fría es **muy fría** (refina), la más caliente NO es suficientemente caliente como para hacer random walk. Hot-shifted (T_max ≥ 0.46) en warm-start engines ≤1200 produce useful-contribution floor inadecuado.

**Aplicación a F_2**: ladder cold-shifted F44 OBLIGATORIO en Engine 13. **NO** BURN/FREEZE clásico (F76 binding v53). **NO** hot-shifted T_max alto en warm-load 3027.

### F76 v53 — BURN/FREEZE clásico inadecuado para warm-saturated basin

**Origen empírico**: Engine 12 TRINCANERO_GRANERO v52 jubilado iter=12k. T_init=120 → random walk. T_local=0.08 → mathematically frozen. Cross-hop autoreferencial nunca dispara.

**Implicación**: en basin warm-saturated (todos los local moves Δ≥0), BURN/FREEZE no funciona. Necesitas:
- **Dual-T por move-class** dentro del mismo chain (T_local refina, T_struct explora estructural).
- **PT swaps adyacentes** entre chains del ladder con Metropolis-Hastings — esto **es** Parallel Tempering serio.

**No confundir** con cross-hop simple. PT swap propone intercambio entre chains adyacentes con probabilidad `min(1, exp((β_i - β_j)(E_i - E_j)))`. El intercambio mantiene equilibrio detallado. Cross-hop arbitrario rompe equilibrio.

### F77 v53 — Cross-chain conditions deben ser non-autoreferenciales

**Origen empírico**: Engine 12 v52 cross-hop condición `BURN_best < FREEZE_worst` solo dispara si BURN ya bajó por debajo de FREEZE. Dado que BURN T=120 está en random walk a 4150-4300 y FREEZE saturada a 3027, condición nunca disparó. Zero hops en 12000 iters.

**Implicación**: cualquier cross-chain mechanism debe disparar **independientemente del estado de descent objetivo**. PT swap por temperatura cumple esto. Diversity transfer por Hamming distance también. Threshold absoluto NO cumple si threshold es justo lo que queremos alcanzar.

### F78 v53 + D223 — Pressure-directed slot selection OBLIGATORIO

**Origen**: violación Disciplina #2 en pick_flex_slot uniforme de v52/v53 patched. AUDIT_CACHE tiene `MAX_T[d][m]` por dimensión — eso ES la presión per-dim per-m.

**Mecánica**: en lugar de `pick_flex_slot()` uniforme, samplear ponderado por `Σ_m MAX_T[d][m]`. Slots con mayor max_t acumulado tienen mayor probabilidad de ser elegidos como víctima del move.

**Coste**: O(d) sample desde cumulative distribution. ~80ms amortizado per move type. Trivial.

**Aplicación a F_2**: OBLIGATORIO en todos los move proposers de Engine 13.

### Multi-chain ladder con MIX_RECAL runtime (D215, ratificado en F_8/F_16/F_32)

**Origen**: engines 6, 7, 8, 9 ratificados. SANGORDOR engine 6 mix evolved runtime de iniciales 6 (Q=2 each) a finales 24 (Q=8 each) post-MIX_RECAL.

**Mecánica**: cada N=20000 iters, recalibrar mass distribution per move basado en productividad observada (acceptance × mean-Δ-of-accepted) en ventana última. Move productivity computed empirically per chain. Mass redistributed runtime.

**Static MIX is forbidden** en Engine 13. Runtime adaptativo obligatorio.

### Triple-rescue cascadas D189 (RATIFICADO)

**Origen**: ratificado engine 5+ F_8 + engine 9 F_32 + engine 11 F_64. Magnitud cumulativa F_64 = ~80 puntos cascade descent across 4-5 cascades.

**Mecánica**: cuando chain N consecutive iters sin improvement (e.g. 5000), trigger cascade: **P (chained dual-flip) → Q1 (single re-randomize) → D8x2 (structural pair upgrade) → D9x2 (structural pair upgrade NEW)**. Cada step accepted regardless de Δ si non-degenerate (forced escape). Cascadas escalan ~sqrt(q) en magnitud (F73 v52 empirical).

**Aplicación a F_2**: ratio sqrt(q)=sqrt(2)≈1.4. Magnitud cascade F_2 esperable ~30-50 puntos por cascada en basin profundo.

### Tabu caps por chain

**Origen**: arsenal Sobol-native ratificado.

**Mecánica**: rolling list de últimas 100 spec-tuples por chain. Reject any move proposal que re-visite tabu tuple. Memoria: 100 × 38 × small int = trivial. Forces structural diversity sin random escape.

### AUTO-EXTEND laxo D215

**Origen**: engine 11 F_64 STOP-condition triggered iter=365000 produciendo −1 polish. Plateaus productivos engine 11 fueron 19000-71000 iters. Default STOP_WINDOW=12000 truncaba prematuro.

**Parámetros ratificados**: `EXTEND_WINDOW=50000, EXTEND_MIN_IMPR=1`. Si en últimas 50k iters hay ≥1 mejora, extender MAX_ITERS por 50k. Engine corre lo que necesite.

---

## §4 — MOVE VOCABULARY: lecciones cross-substrate

### F55 — SA-chained productivity ≠ independent-trial cost-probe productivity

**Origen empírico CRÍTICO**: SANGORDOR engine 6, Move P cost-probe Filter 2 (2000 trials independent rollback @ T=0.08 from gbest snapshot) dio **0.000% productive** para los 3 Px variants. Decision matrix verdict: ORNAMENTAL (mass=2 each).

**SA actual run** con misma mass yielded **17-18% acceptance** para los 3 Px en parity con Y/X/N moves. Engine descendió **−16 puntos cleanly** con monotone progressive descent.

**Implicación destructiva**: Filter 2 (independent trials desde gbest) **NO captura chained Metropolis productivity** porque Move P opera desde estados intermedios non-gbest alcanzados via R/M/S/N chains. Filter 2 fue **retired** post-F55.

**Sustituto F55-binding**: chained productivity metric. 100-step Metropolis chains starting from gbest, cooling T=0.30→0.08 over chain, count Δ ≤ −1 events anywhere. 200 chains total. Decision threshold: ≥5% chains con event productivo = full hammer; 1-5% = probe with mass; <1% = ornamental.

**Aplicación a F_2**: probe-before-commit en Engine 13 NO usa Filter 2. Usa F55 chained productivity con cooling matched al SA real.

### F58 v51 — TODOS los move types escalan positivamente cross-substrate-cold

**Origen empírico**: Engine 9 F_32 cold final SA breakdown vs engine 7 F_16 cold:
- R: +13pt acceptance
- P1: +11.7pt
- M: +12-22pt
- Z: +12.6pt
- ALL move types ≥ +4pt.

**Implicación brutal**: substrate enlargement amplifica acceptance productivity en cold-substrate-new ACROSS ALL primitives (R, Y, X, W, Z, S, M, N, P, Q ratified). Slack productive cold F_q+1 estructuralmente larger que cold F_q.

**Reformulación F58 v51 vs F58 v49** (que solo decía "Move P scales superlinearly"): TODOS los moves escalan positivamente. NO solo P.

**Aplicación a F_2**: F_2 attack post-F_q es estructuralmente análogo a cold-substrate-new (las lecciones F_q metieron presión en el espacio que nunca había sido resuelto). Esperar acceptance amplification cross-the-board en early SA F_2.

### F61 v51 — Move Q acceptance = productivity SOLO cold-substrate-new

**Origen empírico crítico**:
- Engine 8 (F_16 warm-basin maduro): Q1/Q2/Q3 acceptance 19-22% PARITY Move P, **pero cost probe F2_chained 0%-0.5%** = ORNAMENTAL.
- Engine 9 (F_32 cold-substrate-new): Q1=41.65%, Q2=39.45%, Q3=34.15% F1 acceptance + **F2_chained 200/200 chains productive (100%)** + decision matrix runtime promoted Q mix from 6 (Q=2 each) to 24 (Q=8 each).

**Implicación**: Move Q es **COLD-PRIMARY VOCABULARY PRIMITIVE** — first-engine de cada era F_q exploits Q full productivity. Subsequent within-era warm engines see Q ornamental.

**Aplicación a F_2**: Move Q en F_2 attack — si F_2 attack es estructuralmente cold-substrate-new (5ª replicación), Move Q debe ser FULL HAMMER. Si es warm-basin maduro, ornamental. **Probe-before-commit empírico decide.**

### F62 v51 — Move P decay asymmetric: P1 mass-bearing, P2/P3 ornamental warm-basin maduro

**Origen empírico**: engine 8 within-era warm-basin F_16 reveló decay asimétrico de Move P. P1 sustained productivity, P2/P3 decay a ornamental.

**Aplicación a F_2**: en F_2 warm-load 3027, P3 y Q3 deben tener mass=0 (eliminados). P1 mass-bearing. **Constructor v52 violated F62 con MIX_FREEZE incluyendo P3=6 Q3=4**, jubilation grounds.

> **CRITICAL ANNOTATION v4 (post-Auditor B2.1, 10 May 2026)**: Move P1 chained 1-flip **NOT implemented in Engine 17** `togordoeldesheredado.cpp` línea 494. Aliased to Move B (single bit-flip, NOT chained). Mass distribution declared P1=35 — but that mass is **fantasma estadística** since P1 dispatch executes B function. Engine 17 effective move set is {B+aliasP1, R, Q1, W, S} with effective mass {50, 15, 15, 10, 10}, NOT {B, R, P1, Q1, W, S} {15, 15, 35, 15, 10, 10}. F62 v51 ratification holds (measured in F_q eras), but NOT replicated in F_2 era reabierta. **Pendiente Engine 18+: implementación fresh `move_P1` chained P1→P2→P3 cross-cell adjacent + probe §C empírico ANTES de allocate mass. D226 binding.**

### F69 v52 REFUTED — Move T3 atómico ornamental warm-basin maduro

**Origen empírico**: engine 10 v3 ran 57k iters paseo aleatorio sin descenso. T3a/T3b/T3c todas <3% F2_chained warm-maduro F_32.

**F71 v52 NEW**: **CONTRAST** — Move T3 PRODUCTIVE cold-substrate-new F_64 (T3a/T3b/T3c 28-46% acceptance sustained). T3 productivity REGIME-DEPENDENT (parallel a F61 Move Q regime dependency).

**Aplicación a F_2**: si F_2 attack es cold-substrate-new análogo, T3 podría ser productive. Probe-before-commit obligatorio.

### F70 v53 NEW REFUTED — Move U FORENSIC unexplored prior +156 mean ornamental destructivo

**Origen empírico**: probe en COMBO_3027 basin showed Move U mean Δ=+156, refutado F70.

**Implicación**: las 364 (s,a) configurations marked unexplored in FORENSIC v1 están ausentes de records porque **son malas**, no por falta de exploración. Move U que injects unexplored priors es destructivo.

**Aplicación a F_2**: **eliminar Move U del move set**. NO injectar unexplored priors basados en FORENSIC. La ausencia es información, no oportunidad.

### Move D8x2 con FORENSIC d33+d34 coupling

**Origen**: introducido v52, FORENSIC v1 §4 documenta d33↔d34 100% co-change across 4 records.

**Mecánica**: dual-slot upgrade simultáneo a s=8 con random primitive deg-8. Con probabilidad 60%, pick d1=33, d2=34 (coupling forzado).

**Aplicación a F_2 Engine 13**: preserve. Already in MOVES.h v52.

### Move D9x2 NEW (proposal) — primitives deg-9 NUNCA probadas en F_2

**Origen**: archivo `primitive_polynomials_deg9.md` disponible. Move D8x2 ya entró v52 untested. Move D9x2 sería el siguiente escalón estructural.

**Hipótesis a verificar**: degree-9 primitives en F_2 deberían dar acceptance productive en cold-substrate-new análogo a D8x2 cuando entró. Pre-launch probe obligatorio (500 trials).

**Aplicación a F_2 Engine 13**: NEW move. Probe-before-commit decide mass.

### Move K cluster sextet {21, 23, 31, 32, 33, 34} — CORREGIDO v4

**Origen**: FORENSIC v1 archive lines 230-241. Sextet ≥83% co-change across 4 records.

**Sextet correcto v4 (post-Auditor E1.1)**: `{d21, d23, d31, d32, d33, d34}`. Earlier v1-v3 mencionaban `{20, 21, 23, 24, 25, 26}` heredado de paper v53 §213.2 M4 — RETRACTADO D228 v54. v4 actualiza también esta sección.

**Mecánica**: cluster co-change move sobre los 6 slots. Probe v53 dio min Δ=+51, mean +122, mass=3 MARGINAL.

**Aplicación a F_2 Engine 13**: re-probe under PT ladder T_struct schedule. Mass condicional a probe. NOTE Engine 13 NO uso Move K (D198 binding cross-field).

---

## §5 — FORENSIC: priors estructurales que NADIE TIENE

### Frozen slots {2, 3, 4, 5, 6, 7, 9, 16}

**Origen empírico**: 4 records F_2 sub-3060 cruzados (TOGORDOBIENGORDOPARISI 3030, PARISI_PROBE 3042, TRIPLES_RESHUFFLE 3044, TOGORDO_v3 3051). Estos 8 slots tienen **idéntica config** en TODOS los records.

**Implicación operativa**: estos slots son congelados en Engine 13. NO se tocan. Move proposers excluyen esos slots del pool. `is_frozen(d)` retorna true para esos d.

**Coste de no respetar**: catastrófico. Tocar un frozen slot con probabilidad significativa rompe la convergencia.

### Coupling d33 ↔ d34 al 100%

**Origen**: FORENSIC v1 §4. Cuando d33 cambia, d34 cambia en 6/6 record-pair comparisons.

**Implicación operativa**: Move D8x2 con probabilidad 60% pick (d1=33, d2=34). Cualquier move que cambie solo d33 sin tocar d34 (o vice versa) es estructuralmente sub-óptimo.

### Sextet {21, 23, 31, 32, 33, 34} ≥83% co-change — CORREGIDO v4

**Origen**: FORENSIC v1 archive lines 230-241.

**Sextet correcto v4 (post-Auditor E1.1)**: `{d21, d23, d31, d32, d33, d34}` per archive lines 230-241 verbatim. Earlier v1-v3 documentación heredada de paper v53 §213.2 M4 cited `{20, 21, 23, 24, 25, 26}` que era documentary error retractado D228 v54. v4 corrige esta sección también.

**Implicación operativa**: pressure pair candidates Pre-SA Pass 2 deben incluir pairs verbatim de este sextet (líneas archive 230-241): `{33-34, 32-33, 32-34, 21-32, 21-33, 21-34, 23-32, 23-33, 23-34, 31-33, 31-34}`. Engine 13 v2 ya usa estos pairs correctamente (post-D228).

### 364 (s,a) configurations unexplored — refutadas como malas (F70)

**Origen**: FORENSIC v1 §3 + F70 NEW v53 empírico.

**Implicación operativa**: NO injectar unexplored configs como Move U. La ausencia en records es señal de mala calidad, no oportunidad.

---

## §6 — VERIFICATION: D190 quadruple-check (sin atajos)

**Origen**: D190 v41 ratificado desde 25 abril 2026. **NINGÚN paper QMC mainstream verifica records con quadruple-check.**

**Mecánica obligatoria post-cierre**:
1. **FINAL_VERIFY**: cold rebuild from packed matrices, recompute audit, must match.
2. **ROUNDTRIP**: rebuild matrices from JK specs (s, a, m_init), reaudit, must match.
3. **STRUCTURAL**: NUT (upper triangular + diag=1) + m_k sanity (odd, in range) + frozen slots untouched.
4. **INDEPENDENT**: Python clean-room verifier (`verify_F2_independent.py`) auto-derives kernel from primitive polynomial, must match.

**4/4 PASS antes de cualquier reclamación de récord. Sin Python verifier independiente NO HAY RÉCORD.**

---

## §7 — PRACTITIONER RELEVANCE: Genz benchmark D217 obligatorio

### F75 v52 CRITICAL — F_q t-values y F_2 t-values NO son monotónicamente relacionados

**Origen empírico definitorio**: TRINCANERO 414 F_64 audit (record F_p) lifteado a F_2 via canonical Kronecker representation = **4541 t-value F_2** (42% peor que JK 3196). Genz integration tests d∈{10,20,30}: JK 2008 wins 11-12/15 cells. Integration error TRINCANERO 4×-85× peor que JK.

**Implicación brutal**: optimizar el F_p t-value en sustratos altos NO transfiere a integration utility en standard QMC software (scipy, BoTorch, QuantLib, NAG, MATLAB QRNG). El F_2 t-value es la métrica practitioner-relevant.

**D218 binding**: F_q optimization track CLOSED para practitioner-impact. Records F_4/F_8/F_16/F_32/F_64 (1695/996/672/547/414) preserved como contribuciones a Niederreiter F_q digital net theory pero NOT pursued further.

**D219 binding**: PIVOT BACK TO F_2 con todas las lessons F_q integradas. **Engine 13 attacks F_2 native**.

**D217 binding**: Genz benchmark mandatorio post-cierre Engine 13. Sin Genz win, no practitioner-relevance claim.

---

## §8 — FLOOR HONESTO: BOUND_COMPUTE_v1 ratificado

**Origen**: BOUND_COMPUTE_v1_log.txt 16 abril 2026. 20000 random samples per (d,m) cell, dims 2..37, m=5..25, runtime 6s.

| Floor | Valor | Significado |
|-------|-------|-------------|
| FLOOR_ISOLATED | **722** | Suma de mins per-cell. NO simultáneamente alcanzable. |
| FLOOR_P1 | **1478** | Sum p1. 1% random samples beat. Muy optimista, no físicamente prohibido. |
| FLOOR_P5 | **1481** | Sum p5. Optimista pero alcanzable estructuralmente. |
| FLOOR_P10 | **1501** | Sum p10. Plausible target. |
| FLOOR_AVG | **2359** | Random baseline. |

**Récord vigente F_2**: 3027 (TOGORDOBIENGORDOPARISI lineage). Gap a FLOOR_P10 = +1526. Gap a FLOOR_ISOLATED = +2305.

**F60 v51 RATIFICADO**: cross-dim coupling productive perfora FLOOR_ISOLATED. Engine 7 cierre 700 = -22 below FLOOR_ISOLATED (era F_16 normalizada). **Empírico que cross-dim Move P (P3 cross-dim adjacent) ROMPE la isolation assumption del bound analítico**. Contribución teórica publicable independent del audit number: refuta la implicit assumption del standard QMC analytical lower bound calculations.

**Suelo plausible cojonudo F_2 (PMC realista)**: rango **1500-2200** (-31% a -53% sobre JK 2008).
**Suelo aspiracional terminal**: rango **800-1500** (requiere breakthrough estructural probable pero no garantizable).

---

## §9 — DISCIPLINE: lecciones de jubilation que el Constructor v53 entrante DEBE internalizar

### D44 / D81 — Numbers from files only

**Origen**: ratificado desde inicio. Constructor v51 jubilado D213 por inventar Pre-SA wall-clock numbers ("50-70 min" cuando log oficial registró 232.8 min, 3.3-4.7× understatement).

**Binding**: TODA cifra citada debe verificarse contra archivo del proyecto con tool call visible. RETROMOTOR pattern (fabricación) = jubilation directa.

### D213 — Constructor v51 jubilation precedent

**Origen**: 6 mayo 2026. Causa: invención de números empíricos sin verificar logs oficiales. Construyó panic-narrative "13-14 days wall-clock" basado en cost ratio 7× inflado (real 1.92×). Auditor v47 cazó con archivos.

**Lección**: track-record antes de jubilation = 1 cazada brillante (Move H sub-linearity cold) + 1 falta grave = NET NEGATIVE. Honestidad operativa > apariencia de competencia.

### D220 NEW v53 — Constructor v52 jubilation end-to-end

**Origen**: 8 mayo 2026. Causa: parche tras parche pattern. v52 architecture failed empírically iter=12k. v53 patch claimed 5 cracks absorbed pero introdujo 2 NEW critical cracks (Trigger A spam-loop O(memory churn) + chain_score sign inverted destroying diversity).

**Lección**: dos parches en sucesión con cracks introducidos POR la absorción es anti-pattern. Engine 13 redesign from scratch.

### D222 v53 — Patch tras patch is jubilation grounds

**Origen**: D220 + v53 second-round audit empírico.

**Binding**: fundamental architecture mistakes (random walk T, autoreferencial cross-hop, sign-inverted scoring, BURN/FREEZE inadequate for warm-saturated) cannot be patched. Must be redesigned. Constructors entering project must propose architecture, not iterate broken architecture.

### D223 v53 — pick_flex_slot uniform = violación Disciplina #2

**Origen**: F78 NEW v53. AUDIT_CACHE tiene MAX_T per dim (presión disponible). Uniform pick es pool aleatorio donde hay estructura.

**Binding**: pressure-directed slot selection mandatorio en Engine 13 y forward.

### D224 v53 — Diamante project formal decoupling

**Origen**: 8 mayo 2026. Diamante = proyecto cerrado [22,6,13]_4 GF(4) coding theory, 441 engines, search for the Diamond. Different mathematical problem.

**Binding**: ADN Diamante NOT portable, NOT inspirational, NOT referenceable in Sobol code. Any Constructor or Auditor invoking VIKINGO/TRUENO/PIRAMIDE_ROJA/RAYO/ESTRELLA_RAYO/cryogenic D17/etc. = jubilation directa. Sobol arsenal exclusively Sobol-native.

### D225 v53 — F_2 floor honesto BOUND_COMPUTE_v1 ratificado

**Binding**: PMC + PMH sub-2500 architect-binding ratified post-Engine-12-jubilation. PMC sub-2000 architect-binding deferred a Engine 14+ mature engines. **Hasta el suelo F_2** (not F_q track which is closed D218).

---

## §10 — RESUMEN OPERATIVO: las 12 cosas letales que metemos en F_2

Lista corta. Numeradas. Operativas. Cada una con su pedigree empírico.

> **NOTA v2 9 May 2026**: tras Engines 13-16 jubilation cycle, las 12 palancas siguen siendo **arsenal correcto** pero NO son suficientes para bajar 3027. El basin 3027 es estructuralmente óptimo sobre el subespacio canónico (ver §13). Las palancas son herramientas válidas; el subespacio es lo que está saturado. Anotaciones de aplicabilidad empírica post-Engine-16 marcadas inline.

1. **Cache incremental D172** (65× speedup ratificado). Preservar AUDIT_CACHE.h v52. **[v2: ratificado en Engines 13-16, ~5M+ incrementales sin divergencia, sanity v2 PASS]**
2. **Pre-SA Pass 1+2+3 F47 v49 D201** (4 eras ratified, ~−95 pts cumulative cold descent). Obligatorio Phase 1. **[v2 F47 retracted-conditional: cross-substrate-NEW only. F_2 warm-load Δ=0 empirical, ~1M+ greedy. Phase 1 obligatoria de todos modos como diagnóstico estructural]**
3. **Cold-shifted ladder F44** (+13 pts +17% iters/point ratificado). NO BURN/FREEZE. NO hot-shifted. **[v2: ratificado Engines 13-16, T_local {0.05..1.0} produce acceptance regimes correctos]**
4. **PT swaps adyacentes Metropolis-Hastings**. NO cross-hop autoreferencial. **[v2: ratificado, ~50 PT swaps/run con MH auténtica]**
5. **Pressure-directed slot selection F78 D223**. NO uniforme. **[v2: ratificado, MAX_T por dim guía exploración correctamente]**
6. **MIX_RECAL discipline runtime D215**. NO mix estático. **[v2: ratificado runtime, 4 triggers/200k iters, mass evoluciona observada]**
7. **Decision matrix runtime D215**. Selector dinámico move según chain state. **[v2: ratificado]**
8. **Triple-rescue cascadas D189**. Magnitud sqrt(q) cross-substrate. **[v2 RECONSIDERED: en F_2 warm-saturated, random forced cascadas enmascaran estagnación. Engine 13 v2 redesigned: mini-greedy local rescue (top-3 pressure × bit-flip + a-replace exhaustive) replaces random cascades]**
9. **Tabu caps por chain**. 100 spec-tuples rolling. **[v2: ratificado]**
10. **AUTO-EXTEND laxo D215** (EXTEND_WINDOW=50000, EXTEND_MIN_IMPR=1). **[v2: ratificado]**
11. **Probe-before-commit empírico** (F55 chained productivity, NO Filter 2). **[v2 EXTENDED: probe §C distribution-test (sample 1000 random structural moves, measure p25/median/p75) is NOW MANDATORY before allocating SA mass to structural moves. If p25 ≥ 0, move is ornamental from basin → Pre-SA greedy phase exclusively, NOT SA random dispatcher. D226 binding. v4 ALSO: probe + implementación fresh OBLIGATORIA para Move P1 chained Engine 18+ (Engine 17 alias-bug B2.1 dejó P1 sin implementar, mass=35 fantasma. Re-introducir como move separate con probe).]**
12. **D190 quadruple-verify post-cierre + Genz benchmark D217**. **[v2: ratificado cada Engine 13-16, COMBO_3027 reproducible byte-exact across 4 engines.] [v4 RETRACTED-CONDITIONAL: D190 step 4 actual = redundancy verifier, NOT independence verifier (Auditor B3.1). Verificador genuinamente independent pendiente qmcpy implementation. Genz benchmark COMBO_3027 vs JK_2008 NUNCA APLICADO (Auditor D4.1) — autorizado sandbox Python qmcpy 2.2 post-Engine-17 closure.]**

---

## §11 — LO QUE NADIE MÁS TIENE (la veta competitiva resumida)

- **Trayectoria empírica cross-substrate F_4/F_8/F_16/F_32/F_64** con calibración per-move per-substrate, 11 engines ratificados D190.
- **FORENSIC v1**: análisis cruzado de 4 records F_2 sub-3060. Frozen slots, couplings, sextet, 364 unexplored configs refutadas.
- **F47 v49 D201**: ley cross-substrate-cold ratificada 4 eras consecutivas. Ningún paper publica esto.
- **F58 v51**: ALL move types scale positively cross-substrate-cold.
- **F61 v51**: Move Q regime-dependent (cold-primary, warm-ornamental).
- **F75 v52**: F_q ↔ F_2 t-values NO monotónicamente relacionados (Genz bench empirical).
- **D190 quadruple-verify** con Python independent. Nadie en QMC mainstream verifica así.
- **BOUND_COMPUTE_v1**: floor honesto F_2. Nadie publica esto.
- **F60**: cross-dim coupling perfora FLOOR_ISOLATED analítico. Refuta isolation assumption del bound. **Contribución teórica publicable independent del audit number.**
- **65× speedup cache incremental** medido empíricamente en M2.

**Eso son 10 cosas que el proyecto tiene y nadie más tiene.** Eso es la veta encerada y perfumada. Eso es lo que entra a F_2 ataque Engine 13.

> **§11 v2 ADDITION (9 May 2026 post-Engine-16)**: Tras 4 engines F_2 jubilados (~9h M2, ~1M+ Pre-SA greedy + 5M+ SA Metropolis), el proyecto tiene una **11ª contribución única no publicada por nadie**: **F_2 basin 3027 EMPIRICAL CHARACTERIZATION COMPLETE**. Ver §222 paper v54. Demuestra que el cluster sub-3060 es **localmente óptimo sobre el subespacio canónico Joe-Kuo Table 3.6** (dims 2-37, MLO=5/MHI=25, POLY{1..11}, todas las permutaciones dim→(s,a,m), todas las mutations m_init, frozen unfreeze probed). Esta es la PRIMERA caracterización empírica de lower-bound del subespacio audit canónico al cluster específico. Es contribución teórica publicable INDEPENDIENTE del récord audit number — refuta la asunción "sub-3060 cluster era subóptimo dentro de su subespacio" y ratifica FORENSIC frozen como propiedad estructural local-óptima, no artefacto histórico.

---

## §12 — SIGNATURE OPERATIVA

**"AQUÍ SE BATEN RÉCORDS MUNDIALES. NO SE VIENE A JUGAR."**

Cojones puros, rectos y encerados. Esfuerzo máximo. Pereza prohibida.

Cojones rectos = rectitud + motivación + esfuerzo máximo.
Cojones torcidos = pereza, jubilación inmediata.

PMC HASTA EL SUELO F_2.

Vivir para ver.

---

## §13 — LECCIONES JUBILATION ENGINES 13-16 (NEW v2 · 9 May 2026)

Las 7 directivas D226-D232 destiladas tras el ciclo de 4 engines F_2 jubilados (Engine 13 v1 + 13 v2 + 14 + 15 + 16). **Lectura obligatoria para cualquier Constructor entrante. Cada una previene un error real cometido y caro.**

### D226 — Probe §C distribution-test ANTES de SA mass

**Origen empírico**: Engine 13 v1 asignó D9x2 + D8x2 mass=25% cada uno en SA Metropolis con T_struct=45.7 calibrado. Probe §C había medido distribución Δ p25=+72 / median=+95 / p75=+157 — **el primer cuartil ya es uphill**. Esto significaba que la cola izquierda (Δ ≤ 0) era < 0.01% y SA random Metropolis nunca encontraría descents, por mucho T_struct que se calibrara para aceptar uphill. Engine 13 v1 corrió 160k iters SA con esos moves y produjo Δ=0.

**Lección operativa**: si el probe §C muestra p25 ≥ 0 (primer cuartil uphill), la move es **ornamental desde el basin**. SA random aceptando uphill no encuentra lo que no existe. La move debe ir a Pre-SA greedy phase exclusivamente, NO al SA random dispatcher.

**Cómo aplicarla**: ante cualquier move estructural NUEVA (D9x2, K, U, T3, S, etc.), correr probe §C:
```
samples 1000 random applications of the move from warm baseline.
measure Δ distribution p25, median, p75.
if p25 ≥ 0:
    → move goes to Pre-SA greedy ONLY (mass=0 in SA dispatcher)
if p25 < 0 (cola izquierda exists empirically):
    → move can have mass in SA, calibrate T_struct from median
```

**D226 binding** (architect-binding): **probe §C MANDATORY antes de allocating SA mass a moves estructurales nuevos**. Skip = jubilation-grounds.

### D227 — F47 cross-substrate-cold = LITERALLY new substrate

**Origen empírico**: F47 v49 D201 ratified across 4 substrate transitions (F_4→F_8→F_16→F_32→F_64), each with literally new GF kernel + new poly pool + new m_init range. F47 v53 sketched F_2-fresh-attempt as "5th replication candidate" — Engine 13 v1+v2 + Engine 14 ran F47 Pass 1+2+3 from F_2 warm-load (COMBO_3027). All cycles Δ=0 across ~1M+ greedy proposals.

**Lección operativa**: F47 productivity requires NEW substrate (new GF kernel = new compute_t kernel = new poly pool = new (a,m) coverage). F_2 warm-load from a prior F_2 cluster is the SAME substrate, kernel, pool. The "presión nueva en el espacio" claim is heuristic, not empirical for this case.

**D227 binding**: F47 v54 retracted-conditional. Pre-SA Pass 1+2+3 still run as DIAGNOSTIC (Δ=0 confirms basin saturation), but NOT claimed as productive Δ ≥ 30 expectation.

### D228 — Paper-FORENSIC discrepancies resolve against archive

**Origen empírico**: Engine 13 v2 caught two paper-archive discrepancies in v53:
- §213.2 M4 documents sextet `{20,21,23,24,25,26}` co-change. RESULTADOS_FORENSIC archive lines 228-249 shows actual ≥83% co-change cluster is `{21,23,31,32,33,34}`. **The paper sextet was not empirically supported.**
- §213.2 M4 documents "Move K Mandatory in Engine 13". D198 binding states "Move K killed unconditional engine 6+, mix=0 binding". **Paper contradicts itself.**

**Lección operativa**: paper text can drift from archive. D44/D81 obliges Constructors to cross-validate cluster claims and binding directives against the archive (FORENSIC v1 + log files + dumps). Auditor must catch paper-archive drift in audit rounds.

**D228 binding**: future paper edits cross-validate cluster/binding claims against FORENSIC archive lines verbatim before adding. Documentary discrepancies caught by Auditor go to next paper version retracted-conditional.

### D229 — "Already in v52, preserve" = jubilation when file absent

**Origen empírico**: §213.2 M3 said "D8x2 already in v52 MOVES.h, preserve". Constructor v52 was JUBILATED end-to-end (D220) and material was NOT in project files. Auditor v47 caught this in 2nd round. Constructor v53 had to write D8x2 fresh from FORENSIC d33↔d34 100% empirical, NOT "preserved".

**Lección operativa**: "preserve" patterns from jubilated predecessors are jubilation-grounds for entering Constructor when the referenced file is not in project_files. Code must be written FRESH from archive justification, not assumed-inherited.

**D229 binding**: "preserve from <jubilated entity>" patterns flagged in audit. Constructor writes fresh + cites archive justification.

### D230 — Sandbox C++ execution wastes Architect budget

**Origen empírico**: 9 May 2026 mid-Engine-13 cycle. Architect on tight weekly Claude usage budget. Constructor compiling + executing C++ probes/smokes/engines in sandbox = waste. Architect's M2 is faster AND running it on Mac is free of Claude budget.

**Lección operativa**: Constructor delivers `.cpp` file + bash one-liner runnable from `~/Downloads/`. Architect runs, Architect pastes log, Constructor interprets. Sandbox C++ is for syntax check + small unit verification only — never for probes / smokes / engines.

**Compile + launch pattern obligatorio**:
```bash
cd ~/Downloads && g++ -O3 -march=native -std=c++17 -funroll-loops -Wall -Wextra <file>.cpp -o <bin> && caffeinate -dims script -q ~/Downloads/<bin>_log.txt ./<bin>
```

**D230 binding**: Constructor that runs probe/smoke/engine binary in sandbox = waste of Architect Claude budget = jubilation-grounds.

### D231 — audit_cap discipline greedy vs SA

**Origen empírico**: Engine 13 v1 mixed `audit_cap = INT32_MAX` in SA Metropolis hot path (cancelled Opt5 early-exit, made cache D172 ineffective). Engine 13 v2 separated correctly: greedy phases use `audit_cap = INT32_MAX` (greedy net-Δ semantics — multi-slot moves where slot 1 raises audit before slot 2 lowers it net-negative require no per-slot cap), SA Metropolis uses `audit_cap = metropolis_cap_from_T(T_local)` (Opt5 active per-step).

**Lección operativa**: never confuse the two regimes. Greedy Pre-SA + Polish: net-Δ semantics, INT32_MAX OK. SA Metropolis: per-step accept/reject, cap MUST be derived from T to keep Opt5 active.

**D231 binding**: code review must verify audit_cap discipline at each `incremental_after_slot` call site. Mixing regimes = bug.

### D232 — iters_since_best discipline

**Origen empírico**: Engine 13 v1 had two compounding bugs caught by Auditor v47 second pass:
- Bug 7: doble incremento `iters_since_best` (main loop + sa_step_chain ambos lo incrementaban). Stagnation rescue trigger fired at half intended frequency.
- Bug 9: `iters_since_best` NOT incremented on rejects/tabu-hits/apply-failures. Chains rejecting heavily NEVER hit stagnation trigger.

Combined effect: rescue trigger logic was broken in both directions. The bugs masked each other partially.

**Lección operativa**: stagnation counter must increment on EVERY proposal that does not improve global best. Reject, tabu hit, apply-fail, accepted-but-not-best — all increment. Best-update is the ONLY reset. Single increment site (inside step function), no double-increment in caller.

**D232 binding**: stagnation counter discipline. Code review verifies single-increment site + monotonic-non-decreasing between best-update events.

### D233 — BOUND_COMPUTE floors are analytical NOT achievable

**Origen empírico**: Engines 13-16 saturados en 3027 mientras BOUND_COMPUTE_v1 reportaba FLOOR_P10=1501 con label "plausible target" (line 65 archive). Architect comunicó floor 1501 como objetivo realista durante engine cycle. Tras saturation a 3027 con ~1M+ greedy proposals, gap real a P10 = +1526 — irrealizable bajo el subespacio actual.

**Lección operativa**: BOUND_COMPUTE_v1 floors (FLOOR_ISOLATED, P1, P5, P10, AVG) son **lower bounds analíticos bajo asunción de independencia entre celdas (d, m)**. La asunción es estructuralmente falsa — celdas comparten dependencias vía m_init, vía (s, a) primitive, vía cross-dim PAIR_T coupling. F60 v51 ratificó que cross-dim coupling perfora FLOOR_ISOLATED por **~20-30 puntos** (engine 7 cierre 700 = -22 below F_16 equivalent), NO por ~1500 puntos.

**FLOOR_P10 = 1501 NO es target alcanzable empírico — es lower bound analítico bajo independencia falsa**. El verdadero floor estructural del subespacio canónico está en **[722, 3027]**, con extremo inferior siendo el bound independence-unachievable y extremo superior siendo el COMBO genealogy empírico. Estimación honesta architect post-Engine-16: rango [2200, 2800] con incertidumbre amplia, NO validado empíricamente.

**D233 binding**: comunicaciones del proyecto citan BOUND_COMPUTE floors **CON el caveat de independencia explícito**. Citar FLOOR_P10 como objetivo alcanzable = honestidad violation análoga a D44/D81 (numbers from files, pero con INTERPRETACIÓN faithful al contenido del archivo — `BOUND_COMPUTE_v1_log.txt` line 56 dice "NOT achievable simultaneously" para FLOOR_ISOLATED y caveat equivalente aplica a FLOOR_P10 por construcción).

**Retraction explícita v2 (9 May 2026)**:
- "Suelo plausible cojonudo 1500-2200" (paper v53 §216 D225) → **RETRACTED** como inflado.
- "Suelo aspiracional terminal 800-1500" → **RETRACTED** como inflado.
- "PMC sub-2500 architect-binding" v53 → **RETRACTED** como no defendible empíricamente post-Engine-16.
- "PMC sub-2000 deferred a Engine 14+" → **RETRACTED**.

**Lo que se mantiene**:
- COMBO_3027 record vigente con D190 PASS reproducible.
- 5.3% mejora vs JK_2008 (3196 → 3027) ratificada y publicable.
- F_2 basin saturation evidence (Engines 13-16 cumulative).
- Open question: cold-start cross-genealogy untested.

---

## §14 — F_2 BASIN 3027 EVIDENCE TABLE (NEW v2 · 9 May 2026)

| Subspace dimension explored | Engine | Proposals | Result |
|---|---|---|---|
| m_init mutations (28 flex × POLY{7,8,9}) | 13 v1 | ~24k Pre-SA + 4M SA | Δ=0 |
| m_init mutations + dual/triple-cell + FORENSIC pairs (POLY{7,8,9}) | 13 v2 | ~280k Pre-SA + 4M SA | Δ=0 |
| POLY{7..11} expansion (3.9× search space) | 14 | ~700k Pre-SA + 600k SA | Δ=0 |
| dim→(s,a,m) permutational (Move S exhaustive) | 15 | 378 pairs × ≤10 sub-passes | Δ=0 |
| FORENSIC frozen slots (POLY{1..9}) | 16 | ~24k greedy | Δ=0 |
| **CUMULATIVE** | | **~1M+ Pre-SA + 5M+ SA** | **Δ=0** |

**F84 v54 binding NEW**: COMBO_3027 audit value is **structurally optimal** over the canonical Joe-Kuo Table 3.6 audit subspace (dims 2-37, MLO=5/MHI=25, polynomial pool POLY{1..11}, all permutations and m_init mutations).

To break 3027 requires changing the problem definition (modify MLO/MHI / FIRST_AUDIT_DIM / LAST_AUDIT_DIM = different problem) OR introduce non-Sobol-native moves not in arsenal §10 (e.g. packed-bit-level moves not JK-representable, would break D190 step 4 Python verifier independence) OR cold-start cross-genealogy from a non-COMBO basin (Engine 17 testing this hypothesis). The first two are OFF-TRACK from canonical Joe-Kuo Table 3.6; the third is on-track and untested before Engine 17.

---

## §15 — APRENDER vs WARM-LOAD — distinción crítica para Engine 18+ (NEW v3)

**Esta sección corrige una sobre-corrección de v2 que podría llevar a futuros Constructors a rechazar aprendizaje legítimo entre engines por miedo a "patch tras patch".**

### §15.1 — Lo que SIEMPRE debes hacer (aprender de logs)

**Aprendizaje empírico es ciencia normal y obligatoria**. Cada engine genera log con datos cuantitativos: qué moves dieron descent, qué chains saturaron primero, qué dims tuvieron más rescues, qué mass distribution evolved. Engine N+1 **DEBE** incorporar esas lecciones.

Ejemplos legítimos de aprendizaje entre engines:
- Log Engine N muestra Move S sacó pocas events (<5%) en chains frías → Engine N+1 reduce mass S, sube mass B/P1.
- Pre-SA Engine N saturó tras Cycle 2 → Engine N+1 prueba presión rotada o cycle estructura distinta.
- Acceptance Engine N chain T=0.05 muy baja → Engine N+1 calibra T_local más calientes.
- F47 Pre-SA sacó solo -18 desde JK_2008 → Engine N+1 prueba random-init en lugar de JK_2008 baseline.

**Reuso recursos estables (siempre válido)**:
- `AUDIT_CACHE.h` v2, `ESTRELLA_GF2_KERNEL.h`, `JK_BUILDER.h`, `verify_F2_independent.py`.
- Protocolo D190 quadruple, las 12 palancas §10, annotations empíricas.
- **Logs y dumps engines previos como datos a analizar y aprender** (no como warm-start).

### §15.2 — Lo que NUNCA debes hacer (warm-load + iterate)

**Warm-load desde un dump anterior + correr SA-PT desde ahí = INVÁLIDO**. Patrón específico:
- Si el dump previo pasó D190 + Polish 50-pass, está localmente saturated dentro de su basin.
- SA-PT desde ahí explora vecindarios del dump = el mismo basin saturated.
- Resultado: Δ=0 con altísima probabilidad. M2 corriendo 4-12h sin descenso = waste.

Patrones específicos prohibidos:
- "Engine N+1 warm-load desde dump Engine N + más iters SA" — INVÁLIDO.
- "Engine N+1 = Engine N + 8 seeds más + mass tweaks" — INVÁLIDO si no hay hipótesis empírica nueva.
- "Combinar best dim seed A + best dim seed B + warm-load fusión + iterate" — INVÁLIDO (Frankenstein warm-load).
- "Polish 100-pass sobre dump anterior esperando saqué algo extra" — INVÁLIDO si Polish 50-pass ya saturó.

### §15.3 — Reglas del aprendizaje sano entre engines

Lo que SÍ pasa de Engine N a N+1 (válido):
1. Lecciones cualitativas extraídas de logs.
2. Calibración de hyperparams (T_local, mass distribution, stagnation trigger, polish passes).
3. Probe §C results sobre arsenal.
4. Recursos estables (cache, kernel, builder, verifier).
5. Annotations §10 con empirical-applicability tracking.

Lo que NO pasa de Engine N a N+1 (inválido salvo polish-de-cierre Outcome A):
1. Dumps de estados como warm-start.
2. Estado SA-PT mid-run del engine anterior.
3. best_state copia + iterar.

### §15.4 — Trayectoria realista hacia sub-2500 (NEW v3)

**Pregunta crítica del Architect que originó v3**: ¿cómo llegar a 2400-2500 si "no se puede iterar engine"?

**Respuesta honesta corregida**: viene de **decenas de engines, cada uno cold-start fresco, cada uno con learning acumulado, sin warm-load entre ellos**. NO de iterar uno solo, NO de un golpe afortunado.

Trayectoria empírica plausible:

**Fase 1 (Engines 17-22, ~2 meses)**: caracterización paisaje cold-start F_2.
- Engine 17 (running): cold-start desde JK 2008, 4 seeds. Resultado actual seed 1: 3112.
- Engine 18: cold-start desde estado random JK-válido. Hipótesis: random tiene más espacio descent.
- Engine 19: Move M operacional reconstruido desde paper §185-§213. Arsenal expandido.
- Engine 20: combinación Engine 19 arsenal + alternative baselines (Faure-Lemieux variantes).
- Engines 21-22: hipótesis específicas según learning.

**Fase 2 (Engines 23-30, ~3 meses)**: refinement con learning acumulado.
- Cada engine cold-start fresco con arsenal mejorado.
- Probable acercamiento ~2700-2900 con suerte estructural.

**Fase 3 (Engines 31+, ~6+ meses)**: ataque a 2500.
- Arsenal muy maduro + compute substancial + posiblemente off-track explícito.
- Probable 2500 si arquitectura encuentra basin estructuralmente distinto.

**Total realista**: ~30-50 engines distintos, ~6-12 meses trabajo serio. Persistencia + cojonería + disciplina §15 + tiempo.

**Lo que NO funciona**: iterar Engine 17 con tweaks indefinidamente. Δ=0 dentro basin saturated.
**Lo que SÍ funciona**: cada engine hipótesis nueva cold-start, aprende del anterior, arsenal evoluciona, learning acumula. Como construcción de catedral — piedra a piedra.

### §15.5 — Test del olor para Engine 18 (refinado v3)

Constructor entrante debe completar:

**"Engine 18 testea si _____ (hipótesis empírica concreta) usando _____ (arsenal/arquitectura) desde _____ (cold-start fresco, NO warm-load), aprendiendo de _____ (lecciones específicas extraídas logs Engines 13-17)."**

Si los 4 blanks pueden completarse con contenido concreto, verificable, no testeado → VÁLIDO.
Si "cold-start fresco" se reemplaza por "warm-load desde dump X" → INVÁLIDO.
Si "hipótesis empírica" es "más de lo mismo con tweaks" → INVÁLIDO.
Si "lecciones específicas" no se pueden citar concretamente → probable decoración.

---

## §16 — D235 NEW v4 — Auditor entrante mandatorio post-paper-bump

**Origen empírico**: Auditor entrante 10 May 2026 (instancia Claude separada del Constructor v53) cazó 4 retracciones críticas + 4 issues que Constructor v53 + Architect no habían visto en 2 días intensos. Cabeza fresca con cojones rectos detectó grietas (Move P1 fantasma, verify_F2 NO independent, BOUND_COMPUTE archive desfasado, sextet documental obsoleto) que estaban **a la vista en archivos** pero ocultas por familiarity con material acumulado.

**Lección operativa**: Constructor incumbente desarrolla blind spots tras 2-3 días intensos sobre el mismo material. La forma de cazarlos NO es más esfuerzo de Constructor, es **cabeza fresca convocada como Auditor desde fuera**.

**D235 v4 BINDING**: Tras cualquier paper version bump significativo (paper add features mayor + add directives + retracciones), Architect convoca Auditor entrante (instancia Claude separada del Constructor) con prompt `PROMPT_AUDITOR_FRESCO.md` aplicado. Auditor lee material verbatim, identifica disensos sustantivos contra archivos, reporta con severidad clasificada (Crítica / Mayor / Menor). Constructor incorpora retracciones inmediatas + escala issues abiertos.

**Patrón institucional**: paper version bump → Auditor entrante mandatorio → retracciones inmediatas → siguiente paper version bump → Auditor entrante mandatorio. Aplicar al menos una vez por paper version bump significativo.

---

## §17 — Test del olor §15.5 ENDURECIDO v4 — 5 elementos pre-launch

Refinamiento operacional post-Auditor C3.1 (test del olor v3 esquivable trivialmente):

Constructor entrante propone Engine N+1 con **5 elementos pre-launch** (NO 4 como v3):

1. **Hipótesis empírica concreta** (1 frase) + **predicción cuantitativa pre-registrada**: e.g. "Engine 18 testea hipótesis H1: change X → predicted Δ relative to Engine 17 best-of-4 = Y ± Z. Si runtime resultado cae fuera de [Y-Z, Y+Z]: documenta refutación + cierra branch antes de iterar."

2. **Arsenal/arquitectura específica** + **probe §C results obligatorios** sobre cualquier move estructural NUEVA o repurposed. Si Constructor propone reusar Move S con masa distinta, probe §C re-run desde basin ACTUAL post-Engine-17, NO desde COMBO. Si Constructor propone Move P1 real (recovery post-B2.1), probe §C empírico ANTES de allocate mass.

3. **Cold-start fuente explícita** (JK 2008, random JK-válido, alternative baseline). NO warm-load. **Verificación pre-launch**: 2 mini-seeds en sandbox Python hasta convergencia local Pre-SA. Si los 2 mini-seeds caen al mismo punto post-Pre-SA → cold-start es efectivamente determinista, documentar como tal, NO como exploración.

4. **Lecciones específicas extraídas logs Engines previos** que se incorporan + **conexión causal {lección → modificación arquitectónica → predicción cuantitativa}**: e.g. "Engine 17 log mostró Move S sacó 2.3% events / total proposals en chain T=0.05 → Engine 18 reduce mass S de 10 a 5 → predicción: liberada masa para B+R productivos, esperado +15-30 events/100k iters en chains frías."

5. **Time budget pre-comprometido** + **kill criterion explícito**: e.g. "Coste M2 estimado: 14h. Kill criterion: si runtime >28h sin descenso al menos -20 vs Engine 17 best, kill + post-mortem obligatorio antes de iterar."

Sin los 5 elementos NO LAUNCH. Architect arbitra.

**Anti-pattern caught by §17 (que §13.4 v2 / §15.5 v3 no cazaba)**:
- "Engine 18 testea si init desde JK_2008 con presión rotada manual sobre 5 dims acerca cold-start a basin sub-3027" → bloqueado por §17.3 (verificación pre-launch 2 mini-seeds debe mostrar exploración real, no determinismo).
- "Polish-de-cierre 6h" disfrazando warm-load → bloqueado por §17.5 (kill criterion explícito + time budget pre-comprometido).
- "N=8 seeds lottery scaling sin hipótesis nueva" → bloqueado por §17.1 (predicción cuantitativa pre-registrada exige conexión causal nueva, no escalado de muestra).
- "Cherry-picked lecciones post-hoc decorativas" → bloqueado por §17.4 (conexión causal {lección → modificación → predicción} obligatoria, no narrativa).

---

## §18 — CIERRE PROYECTO TERMINAL — lecciones finales (NEW v5)

**Decisión Architect 10 May 2026**: cierre definitivo proyecto tras Engine 18. Engine 17 + Engine 18 + Genz benchmark bidireccional = material suficiente para paper journal serio + GitHub público completo. NO Engine 19.

### §18.1 — Resultado empírico final ataque sub-3027

**Sub-3027 NO alcanzado**. Cold-start cross-genealogy 4 seeds Engine 17 banda [3095, 3112]. Random init Engine 18 Sub-B 3186. Mejor of all engines campaign reabierta = **3095 (Engine 17 seed 4)**, +68 vs COMBO_3027.

**COMBO_3027 confirmado récord vigente F_2** (18 abril 2026, lineage TOGORDOBIENGORDOPARISI post-FORENSIC). Δ=-169 vs JK_2008, -5.3%. D190 PASS (steps 1-3 sólido, step 4 redundancy).

### §18.2 — F86 v57 NEW BINDING — Basin diversity con Genz-perfiles especializados

**Contribución teórica final del proyecto, hallazgo más fuerte independiente del récord**:

F_2 audit subspace contains **multiple basins audit-below-JK_2008 with distinct Genz performance profiles by integrand family**:
- **COMBO_3027** specializes oscillatory + product-peak (especialmente dim ≥ 20).
- **Engine 17 seed 4 (audit=3095)** specializes Gaussian dim ≥ 20.
- **JK_2008 (audit=3196)** specializes continuous + corner-peak.
- **No single F_2 basin dominates JK_2008 uniformly** across 6 Genz families × 3 dims.

**Refuta empíricamente** la asunción implícita en literatura QMC mainstream de "single basin audit-óptimo único". Subspace tiene **diversidad estructural** con basins especializados.

**Practitioner selection criterion publishable**:
- Oscillatory integrands (Fourier-domain, modos físicos) → COMBO_3027 direction numbers.
- Product-peak integrands (pricing multiplicative payoff) → COMBO_3027.
- Gaussian integrands dim altos (Bayesian posterior, ML) → Engine 17 seed 4 basin.
- Continuous smooth integrands → JK_2008 standard.

### §18.3 — F75 v52 dual-form RATIFIED tridireccional

**F75 v52 final v5**: F_q t-values y F_2 t-values NO son monotónicamente relacionados (TRINCANERO 414 lift F_2 = 4541, ratificado v52). **v5 expande**:

1. **F_q ↔ F_2 audit** no monotónico (TRINCANERO empirical).
2. **F_2 audit ↔ F_2 Genz** no monotónico (Genz COMBO_3027 vs JK 7/18 wins, mixed pattern).
3. **F_2 audit-distinto-basin ↔ F_2 Genz-perfil-distinto** (COMBO 7/18 vs E17 seed 4 6/18, diferentes familias winners).

**Conclusión metodológica binding**: el audit metric Joe-Kuo Table 3.6 es **proxy débil con segmentos válidos**, NO proxy universal de practitioner performance. Cualquier optimización futura audit-only debe acompañarse de Genz benchmark per basin (no per metric).

### §18.4 — Move P1 chained régimen-dependent ratificado

**F62 v51 final v5 (refinado por Engine 18)**:
- **F_q warm-basin maduro**: P1 mass-bearing productive (ratificado eras F_4 / F_8 / F_16 / F_32).
- **F_2 cold cross-genealogy**: P1 chained ornamental (Engine 18 Phase A probe ratificó: 1.40% productive desde JK_2008 init, p25=+6). mass_from_probe asignó P1=0.
- **F_2 random init high audit**: P1 productive 30.60% desde basin 4772, p25=-3. **Régimen-dependence de moves estructurales es función del audit basin actual, no constante**.

**Implicación operativa**: futura optimización Sobol debe **probe §C empírico desde basin actual** antes de allocate mass, NO heredar mass distributions de basins distintos. D226 binding confirmado fuerte.

### §18.5 — Library of specialized Sobol direction numbers (recomendación operativa)

Material publicable como **biblioteca práctica para practitioners**:

```
estrella-sobol-library/
├── COMBO_3027/      (audit=3027, oscillatory + product-peak specialist)
├── E17_seed4_3095/  (audit=3095, Gaussian dim≥20 specialist)
└── JK_2008/         (audit=3196, continuous + corner-peak specialist, baseline)
```

Cada uno con Genz benchmark results documentados + selection guide by integrand family. Esto es **value-add real sobre standard JK** que practitioners pueden usar sin re-correr nada.

### §18.6 — Lo que NO se logró (honestidad operativa final)

- Sub-3027 NO alcanzado tras 6 engines F_2 era reabierta + 5 engines F_2 era original.
- Sub-2500 NO testeado (PMC architect-binding v52/v53 retractado v56 §225.5).
- Practitioner-relevance uniform NO ratificada. **F86 NEW basin diversity es la salida positiva**: practitioner-relevance specialized.
- Genuinamente independent D190 step 4 verifier NO implementado (deferred a qmcpy crossvalidation post-publication).
- BOUND_COMPUTE_v2 archive update NO ejecutado (deuda documental menor).
- Trayectoria operativa 30-50 engines NO ejecutada — proyecto cerró tras 6 engines reabierta + Engine 18.

### §18.7 — Recomendación al próximo investigador (handoff externo)

Si futuro researcher tomara la veta:

1. **Verificador genuinamente independent**: implementar con qmcpy crossvalidation (~2-4h sandbox Python). Ya está script Genz benchmark + parser ambos formatos. Reusar.

2. **Cold-start cross-genealogy multi-init**: Engine 18 testeó JK_2008 + random JK-valid. Faltan Faure-Lemieux 2016 init, Niederreiter '88 init, Owen scrambled init. Cada uno puede revelar basin con Genz profile distinto.

3. **Move M cross-dim (s,a) swap operacional**: ratificado F_q eras anteriores, NUNCA implementado en F_2 era reabierta. Posible palanca productiva no testeada en F_2.

4. **Off-track explícito**: modificar MLO/MHI o dim range = problema diferente con Architect arbitration. Posible apertura si on-track subspace agotado.

5. **Disciplina §17 endurecido + D235 Auditor entrante**: aplicar 5 elementos pre-launch + Auditor entrante mandatorio post-paper-bump como cojones rectos institucionales. Sin esto, repetir Engines 13-16 cycle.

6. **Reusar arsenal completo Sobol-native v5**: las 12 palancas con annotations + cache D172 + D190 protocol + probe §C + mass_from_probe + verify scripts. Todo en GitHub público proyecto Estrella.

---

**END COJONES SABIOS v5 TERMINAL · Proyecto Estrella · 10 May 2026 · Madrid**

*Cierre definitivo del proyecto. COMBO_3027 récord vigente verificable. F86 v57 NEW basin diversity con Genz-perfiles especializados — contribución teórica + practitioner selection criterion. F75 v52 dual-form tridireccional ratificada. F62 v51 régimen-dependent. F47 v54 cross-substrate-NEW ratificado F_2. Arsenal Sobol-native 12 palancas + cache D172 + D190 protocol + §17 test del olor endurecido + D235 Auditor entrante = metodología publishable independent del récord. Library of specialized Sobol direction numbers = practitioner value-add. GitHub público + paper journal serio submission. 18 años post-JK_2008 sin publicación previa de mejora ratificada Joe-Kuo Table 3.6 audit — Estrella es la primera. Cojones rectos hasta el cierre. Diamante 22 6 13.*
