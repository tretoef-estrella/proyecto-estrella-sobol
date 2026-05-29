# GUIDE_FOR_EVERYONE

**A plain-language tour of Proyecto Estrella.** No mathematical prerequisites. If you have already read `MAIN_DISCOVERIES.md` and want detail, skip this file. If you arrived from a link and are wondering "what is this and why should I care", start here.

---

## What kind of thing is this?

This is a piece of mathematical and software work that produces a better version of a tool used by many software libraries you have probably already used indirectly: the **Sobol sequence**.

A Sobol sequence is a clever way to generate points inside a many-dimensional cube so that the points cover the cube **evenly** — much more evenly than purely random points. They were invented by I. M. Sobol' in 1967. Today they are used to:

- Price financial options (Monte Carlo simulation under Black-Scholes and many extensions).
- Calibrate machine-learning models (hyperparameter search).
- Render realistic 3D images (path tracing in computer graphics).
- Solve high-dimensional integrals in physics, chemistry, and engineering.
- Run experiments in Bayesian inference, robotics planning, and reinforcement learning.

If you use `scipy.stats.qmc.Sobol` in Python, or BoTorch in PyTorch, or QuantLib in C++, or any of dozens of other libraries, you are using a Sobol sequence. The quality of those sequences depends on a set of numbers called **direction numbers**. Better direction numbers → more even coverage → smaller errors in your simulation.

The de facto worldwide standard for direction numbers since 2008 has been the construction by **Stephen Joe and Frances Y. Kuo**, two researchers at the University of New South Wales. Their direction numbers are what `scipy` ships with.

---

## Who made this, and why it is unusual

This was done by one person — Rafael Amichis Luengo, a psychologist from Madrid with no formal training in mathematics — on a single MacBook Air, running the chip at a quarter of one core, over a few weeks, working with AI systems as accelerators. That is worth stating plainly because the result is the kind of thing normally produced by a university research group: the first verified, reproducible improvement on the Joe-Kuo audit criterion in eighteen years. The engines have deliberately ridiculous Spanish names (you will meet `TOGORDOELGRANGRASIENTOESELREYQUEGANALENTO` shortly) because the project's rule was that a silly name cannot be allowed to survive a mediocre engine — the name forces the work to be good.

---

## What does this project do?

Two things — but to understand the second, you need the story of a wrong turn.

### Thing one — a better set of direction numbers (for some uses)

Joe and Kuo did not just publish their direction numbers; they also published the criterion by which they judged them. That criterion is a number called the **audit value** (Joe-Kuo Table 3.6). For their direction numbers, the audit value is 3196. Lower is better.

For eighteen years, nobody published a set of direction numbers with a lower audit value — verified, reproducible, on exactly the same criterion as the original paper.

**This project did.** The set called `COMBO_3027` (file `TOGORDO_COMBO_v1_RECORD.txt`) has an audit value of 3027 — about 5.3 % lower. You can verify this yourself in under a second on any laptop with the Python script in this repository.

### The wrong mountain

Here is where the project becomes interesting, and it is worth telling honestly because it is the part most people would have quietly deleted.

Once the binary field (the one Joe-Kuo work in) seemed squeezed dry near 3027, there was an obvious idea: *use a bigger number system*. Mathematicians have whole families of "finite fields" — GF(2), GF(4), GF(8), GF(16), GF(32), GF(64) — and a bigger one has more room to manoeuvre. So the project climbed. A new engine, a new arithmetic, learned from scratch for each field. And the audit value collapsed, field after field:

> GF(4): **1711**. GF(8): **996**. GF(16): **672**. GF(32): **547**. GF(64): **414**.

That last number is **eighty-seven percent below** the eighteen-year-old baseline. Weeks of effort. Engines that ran for two full days each. It looked like a complete demolition of the state of the art. It felt like a celebration was in order.

Then the honest question: *does this actually make anyone's simulation faster?*

To answer it, the GF(64) champion had to be translated back into the binary form that real software (scipy, QuantLib) actually uses. And when it was translated back, the beautiful `414` turned into a binary score of **4541 — forty-two percent *worse* than plain Joe-Kuo**. On a standard battery of test integrals it lost almost everything.

The records were real *inside their own number systems* and worthless outside them — because no real software lives in GF(64), and the bridge back to the binary world destroys exactly the structure that had been so carefully optimised. The descent to 414 was a beautifully engineered walk down a path that no working software travels, and none is planning to.

That is not a failure to hide. It is the **central scientific finding**: driving the audit number down in one number system tells you nothing reliable about quality in the one that matters. So the project labelled the field records honestly as a closed side-exploration, turned around, walked back to the binary field, and went after the only criterion that governs real-world error — carrying every lesson the detour had paid for. That return is where `COMBO_3027` comes from.

### Thing two — the surprising part, back in the binary world

Even back home, there was a catch, and it is itself a useful discovery:

A lower audit value does **not** automatically mean smaller errors in your real simulations.

We checked by running the new direction numbers and the original Joe-Kuo ones through a standard battery of 18 test integrations (the **Genz suite**: six families of test functions, each in three dimensions). On these 18 tests, the new set wins 7 and loses 11. Not a uniform improvement — **better on some kinds of problem and worse on others**.

And we found a *second* set (file `togordo17_seed3_dump.txt`, audit value 3095) with a *different* pattern of wins: it wins on Gaussian-type integrands in high dimensions, where the first set loses.

That is the genuinely interesting finding: **the space of "good" Sobol direction numbers is not organised around a single best point**. There are several distinct sets, all better than Joe-Kuo on the original criterion, each good at a different kind of problem. A practitioner should choose by the kind of integrand they have, not by the audit number alone. This refutes a quiet assumption in the literature that audit-metric optimisation is a single-target problem.

---

## What should I do with this?

**If you write Monte Carlo simulations**: the three direction-number files here (`TOGORDO_COMBO_v1_RECORD.txt`, `togordo17_seed3_dump.txt`, `new-joe-kuo-6.21201`) are plain text and easy to load. Pick by integrand type:

- Oscillatory or product-peak integrands, dimension ≥ 20 → `COMBO_3027`.
- Gaussian-type integrands, dimension ≥ 20 → `E17_seed4_3095`.
- Smooth integrands or low dimension → Joe-Kuo (2008) standard (the scipy default).

A worked example loading any of the three sets into `scipy.stats.qmc.Sobol` is below.

**If you are a researcher in QMC, computational finance, or computer graphics**: please cite this work if you use it, replicate it if you doubt it, and extend it if you can. The full paper (`PAPER_TERMINAL.md`), the methods primer (`METHODOLOGY.md`), the operational arsenal (`COJONES_SABIOS_TERMINAL.md`), and the contributions in compressed form (`MAIN_DISCOVERIES.md`) are all here. The C++ engines, the Python verifier, the Genz benchmark, the full campaign logs, and every dump are included so that any disagreement can be settled against the files.

**If you maintain a software library**: the records here are MIT-licensed. Practitioner guidance is in `MAIN_DISCOVERIES.md`.

**If you are simply curious**: read on. The rest explains how this works without mathematical notation.

---

## How did the project produce these results?

The recipe was:

1. **A cache trick.** A naive evaluation of the audit value gets quadratically slower as dimensions grow. The project built an incremental cache that updates the audit value with a bounded amount of work when a single direction-number slot changes, plus a rollback structure so that failed trial moves cost almost nothing. Measured speedup: 65× on a single laptop core. This is what makes everything else affordable on one M2.

2. **Search engines.** Six independent C++ programmes, each a different search strategy, each combining an *exhaustive greedy* phase (try many polynomials against many dimensions, keep the best start), a *simulated annealing* phase (five parallel "temperature" chains exploring at once, occasionally swapping states), and a *polish* phase (systematically squeeze out every last local improvement).

3. **A productivity test before commitment.** Before letting any move type consume the annealing time budget, sample 1000 random attempts of it from the current state and measure how often it actually helps. If it rarely helps from *here*, it gets zero time — even if it was useful in a previous project. This is the antidote to trusting old lore: the same move was useful in one situation and useless in another, and only the live data could tell which.

4. **Quadruple verification of every record.** Four independent checks must pass before a number is called a record: rebuild from scratch and compare; save to a text file, read it back, rebuild, compare; check every structural invariant; run a separate Python implementation. (That fourth check is honest about its limits — it shares the underlying formula with the main code, so it catches coding mistakes but not formula mistakes. A fully independent check is the recommended next step.)

5. **Brutal honesty about negative results.** Midway through, a fresh pair of eyes — a separate AI instance given the explicit role of forensic auditor — caught a real bug: a move type that had been *named* in the code but secretly *behaved like a different move*. The result was retracted, the engine rebuilt with the move done properly, and the move then turned out to be unhelpful in that situation — also a useful finding. Inviting a fresh audit after every major change became a standing rule.

6. **A clean stop.** After six engines back in the binary field and a Genz benchmark with honestly mixed results, the project was declared finished. Any later findings go into `NEW_DISCOVERIES.md` in this same repository.

---

## Worked example: using the records in Python

The three direction-number sets here are plain text. Here is how to load any of them.

```python
import re

def parse_direction_numbers(path):
    """
    Parses any of the three direction-number files in this repository:
      - new-joe-kuo-6.21201          (Joe-Kuo 2008 baseline,  audit = 3196)
      - TOGORDO_COMBO_v1_RECORD.txt  (this project's record,  audit = 3027)
      - togordo17_seed3_dump.txt     (alternative basin,      audit = 3095)
    Returns a list of (d, s, a, m_init) per dimension, starting at dim 2.
    Tolerates both the labelled dump format ('d= 8: s=5 a=  7 m: 1 3 5 1 29')
    and the plain whitespace format ('8 5 7 1 3 5 1 29').
    """
    specs = []
    with open(path, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#') or line.lower().startswith('the '):
                continue
            # Labelled dump format
            mobj = re.search(
                r'd\s*=?\s*(\d+).*?s\s*=?\s*(\d+).*?a\s*=?\s*(\d+).*?m\s*[:=]?\s*([\d ]+)',
                line)
            if mobj:
                d, s, a = int(mobj.group(1)), int(mobj.group(2)), int(mobj.group(3))
                m = [int(x) for x in mobj.group(4).split()]
                if len(m) == s:
                    specs.append((d, s, a, m))
                continue
            # Plain whitespace format
            parts = line.replace(',', ' ').split()
            try:
                d, s, a = int(parts[0]), int(parts[1]), int(parts[2])
                m = [int(x) for x in parts[3:3 + s]]
                if len(m) == s:
                    specs.append((d, s, a, m))
            except (ValueError, IndexError):
                continue
    return specs

# Load the project record
specs = parse_direction_numbers('TOGORDO_COMBO_v1_RECORD.txt')
print(f"Loaded {len(specs)} dimensions, first row: {specs[0]}")

# To build a scipy Sobol generator you need the full direction matrices.
# verify_F2_independent.py in this repository contains a complete, heavily
# commented reference implementation of the Joe-Kuo recurrence that turns
# (s, a, m_init) into the direction matrices and then into points.
```

The dump column layout can vary slightly between files; the parser above tolerates both observed formats, but always sanity-check `len(specs) == 36` (dims 2..37) after loading, and re-run `verify_F2_independent.py` on the file to confirm the audit value before trusting it in production.

For a complete, self-contained example that builds and uses a Sobol generator from any of the three files, see `verify_F2_independent.py` — it parses the file, builds the direction matrices, generates points, and computes the audit value, and is heavily commented.

---

## Acknowledgements and people

The project was conducted by Rafael Amichis Luengo, a psychologist by training who learnt the relevant mathematics autodidactically over several months. The code was written in collaboration with Claude (Anthropic), in a working pattern with three distinct roles: Architect (Rafa, who arbitrated every decision and ran every job on his M2), Constructor (Claude, who wrote the C++ engines and Python scripts), and Auditor (a separate Claude instance, brought in fresh at key transitions to audit the work against the archives). The triple-role discipline and the practice of inviting fresh-eye audits after major changes are themselves documented as methodological contributions.

The work builds on Joe & Kuo (2008), Sobol' (1967), Niederreiter (1987–1988), Owen (1995), Faure & Lemieux (2016), and the standard Genz test suite.

---

## License and citation

Released under the MIT license. If you use it in a paper, please cite it; a machine-readable citation file (`CITATION.cff`) is included.

---

*Madrid, May 2026 — Rafael Amichis Luengo.*
