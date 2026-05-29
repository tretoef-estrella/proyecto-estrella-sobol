# Engine Lineage — Proyecto Estrella · Sobol Campaign

The search was carried out by a large family of single-purpose C++ engines (the campaign built dozens, each a full rewrite rather than a patch). The engines carry deliberately absurd names in Castilian Spanish — a working discipline of the project: *a ridiculous name is not allowed to survive a mediocre engine*. When an engine set a record, its name went into the record without apology.

This table lists the engines **documented with a verified record** in the project paper, by finite field, best to worst audit within each field. It is not the full count of every prototype ever compiled; it is the set of engines whose results are recorded and reproducible from the dumps and logs in this repository.

**Reminder on what the numbers mean.** Only the **GF(2)** result is a practitioner-relevant improvement on the Joe-Kuo Table 3.6 metric. The **GF(q)** records (q = 4, 8, 16, 32, 64) are real records *inside their own fields* but do not transfer to GF(2) — lifted back, they are worse than the baseline (see `MAIN_DISCOVERIES.md` Discovery 3 and the figure in the README). They are preserved here as a self-contained exploration of higher-cardinality digital-net theory, clearly labelled as non-practitioner-relevant.

---

## GF(2) — the practitioner-relevant field (Joe-Kuo metric)

| Engine | Audit | Date | vs JK_2008 (3196) | Notes |
|---|---:|---|---:|---|
| TOGORDOBIENGORDO (lineage) | **3027** | 19–20 Apr 2026 | **−5.3 %** | **Project record. Practitioner-relevant.** Dump: `TOGORDO_COMBO_v1_RECORD.txt` |
| TOGORDOELDESHEREDADO (Engine 17, seed 4) | 3095 | May 2026 | −3.16 % | Alternative basin, Gaussian dim≥20 specialist. Dump: `togordo17_seed3_dump.txt` |

The 3027 lineage closed at audit 3030 (TOGORDOBIENGORDO) and was refined to 3027 post-FORENSIC. JK_2008 baseline = 3196.

---

## GF(4) — closed exploration (not JK-comparable)

| Engine | Audit | Date | Verification |
|---|---:|---|---|
| TOGORDOUNREOMUYDIARREO | 1695 | 25 Apr 2026 | D190 triple ✓ |
| TOGORDOCONPIOLETYVERMUT | 1704 | 24 Apr 2026 | D190 triple ✓ |
| TOGORDOESPORMISHUEVOS1666 | 1711 | 24 Apr 2026 | D190 triple ✓ (387 min) |
| TOGORDO_TORREO_TABULAR | 1743 | 23 Apr 2026 | ✓ |
| TOGORDO_MISIL | 1773 | 23 Apr 2026 | ✓ |
| TOGORDO_ANTIBUNKER_v2 | 1813 | 23 Apr 2026 | ✓ |
| TOGORDO_CASCADA | 1867 | 23 Apr 2026 | ✓ |
| DOSTURRONESDOSCOJONES | 1910 | 23 Apr 2026 | ✓ |
| DOVELA_BOMBA_v3 | 1914 | 23 Apr 2026 | ✓ |
| DOVELA_BOMBA_v2 | 1925 | 23 Apr 2026 | ✓ |
| DOVELA_BOMBA_v1 | 1932 | 23 Apr 2026 | ✓ |
| PORMISCOJONES33V5 | 1950 | 22 Apr 2026 | ✓ |
| TOGORDOELHUEVEROV4 | 1960 | 22 Apr 2026 | ✓ |
| TOGORDO_GF4_v3 | 1969 | 22 Apr 2026 | ✓ |
| DEFROST_PROBE_4_6 | 1978 | 22 Apr 2026 | ✓ |
| TOGORDO_GF4_v2_CLEAN | 2108 | 22 Apr 2026 | ✓ |
| TOGORDO_GF4_v1 | 2817 | 21 Apr 2026 | ✓ (413 s) |

---

## GF(8) — closed exploration (not JK-comparable)

| Engine | Audit | Date | Verification |
|---|---:|---|---|
| SANGORDORTOGORDOLETALPMC | 996 | 30 Apr 2026 | D190 quadruple ✓ (18.3 h) |
| TOGORDOPIMPAMPUMJUBILATORE | 1012 | 28–29 Apr 2026 | D190 quadruple ✓ (16.7 h) |
| TOGORDOELTRAGASUELOSVAAEXPLOTAR | 1033 | 27 Apr 2026 | D190 quadruple ✓ (14.6 h) |
| TOGORDOLITTLEBOYESMASQUEFATMANYMILVECESBATMAN | 1053 | 27 Apr 2026 | D190 quadruple ✓ (20.1 h) |
| TOGORDOESTAMASGORDOYSUMOTORMASDESTRUCTOR | 1066 | 27 Apr 2026 | D190 quadruple ✓ (17.7 h) |
| TOGORDOESMASFATMANQUEBATMAN | 1136 | 26 Apr 2026 | D190 quadruple ✓ (14.9 h) |

---

## GF(16) — closed exploration (not JK-comparable)

| Engine | Audit | Date | Verification |
|---|---:|---|---|
| TOGORDOESELMASGORDOYADEMASELREYMASGORDO | 672 | 2 May 2026 | D190 quadruple ✓ (24.0 h) |
| TOGORDOESELMASGORDOPERONOELREYDELOSGORDOS | 700 | 1 May 2026 | D190 quadruple ✓ (24.0 h) |

---

## GF(32) — closed exploration (not JK-comparable)

| Engine | Audit | Date | Verification |
|---|---:|---|---|
| TOGORDOELGRANGRASIENTOESELREYQUEGANALENTO | 547 | 4 May 2026 | D190 quadruple ✓ 4/4 (44.7 h) |

---

## GF(64) — closed exploration (not JK-comparable, not D190-ratified)

| Engine | Audit | Date | Verification |
|---|---:|---|---|
| TOGORDOTRINCANEROELDELOSCOJONESPUROSPERFUMADOSPLUS | 414 | May 2026 | **NOT D190 ratified** — terminated voluntarily after the Genz benchmark showed the GF(64) record lifts to a GF(2) value of 4541 (worse than baseline). This is the engine that proved the field detour was the wrong mountain. |

---

## A note on the names

The lineage is deliberate. *TOGORDO* ("too fat") is the family root; the rest of each name is an escalating joke in the campaign's own voice. This is documented as a methodological choice, not an accident: the names are a discipline against shipping mediocre work under a name that would embarrass it. The convention is the author's signature on the work, and it stays.

---

*Proyecto Estrella · Sobol Campaign · Rafael Amichis Luengo, Madrid · May 2026.*
*GF(2) record 3027 is the practitioner-relevant result. All GF(q) records are real within their fields and documented as a closed, non-transferable exploration.*
