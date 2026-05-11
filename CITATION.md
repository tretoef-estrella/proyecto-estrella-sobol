# Citation

If you use this work, please cite it. Three formats are provided below: BibTeX (for LaTeX papers), APA, and IEEE.

A machine-readable `CITATION.cff` is also included in the repository — GitHub will automatically use it to render the **"Cite this repository"** button on the right-hand sidebar.

---

## BibTeX

```bibtex
@misc{amichis2026estrella,
  author       = {Amichis Luengo, Rafael},
  title        = {Proyecto Estrella: Empirical characterization of basin
                  diversity in the F\_2 Joe-Kuo Table 3.6 audit subspace,
                  with functional-family-specialised Genz performance profiles},
  year         = {2026},
  month        = may,
  howpublished = {GitHub repository},
  url          = {https://github.com/REPLACE_WITH_YOUR_USERNAME/proyecto-estrella-sobol},
  note         = {Records: COMBO\_3027 (audit 3027, -5.3\% vs JK\_2008);
                  E17\_seed4\_3095 (audit 3095, Gaussian dim$\geq$20 specialist).
                  MIT licence. Byte-exact reproducible.}
}
```

---

## APA (7th edition)

> Amichis Luengo, R. (2026). *Proyecto Estrella: Empirical characterization of basin diversity in the F₂ Joe-Kuo Table 3.6 audit subspace, with functional-family-specialised Genz performance profiles* [Computer software]. GitHub. https://github.com/REPLACE_WITH_YOUR_USERNAME/proyecto-estrella-sobol

---

## IEEE

> R. Amichis Luengo, "Proyecto Estrella: Empirical characterization of basin diversity in the F₂ Joe-Kuo Table 3.6 audit subspace, with functional-family-specialised Genz performance profiles," GitHub repository, May 2026. [Online]. Available: https://github.com/REPLACE_WITH_YOUR_USERNAME/proyecto-estrella-sobol

---

## What to cite, depending on what you use

- **If you use the `COMBO_3027` direction numbers** in your own integration code, cite as above with a note clarifying which dataset: *"using `COMBO_3027` direction numbers from Amichis Luengo (2026)"*.
- **If you use the `E17_seed4_3095` alternative basin**, cite similarly: *"using `E17_seed4_3095` direction numbers from Amichis Luengo (2026), the Gaussian-dim≥20-specialised basin"*.
- **If you use the methodology** (cache D172, D190 quadruple verify, probe §C, the hardened pre-launch checklist): cite this work and refer to `METHODOLOGY.md` in the repository.
- **If you use the Genz benchmark script** to compare your own direction numbers against Joe-Kuo (2008), cite this work plus the original Genz reference:

    Genz, A. (1984). *Testing multidimensional integration routines*. In B. Ford, J. C. Rault, & F. Thomasset (Eds.), *Tools, Methods, and Languages for Scientific and Engineering Computation* (pp. 81–94). North-Holland.

---

## Required upstream citations

The Joe-Kuo (2008) construction this work improves upon should be cited together with this work:

```bibtex
@article{joe2008constructing,
  author  = {Joe, Stephen and Kuo, Frances Y.},
  title   = {Constructing {S}obol sequences with better two-dimensional projections},
  journal = {SIAM Journal on Scientific Computing},
  volume  = {30},
  number  = {5},
  pages   = {2635--2654},
  year    = {2008},
  doi     = {10.1137/070709359}
}
```

The original Sobol' sequence:

```bibtex
@article{sobol1967distribution,
  author  = {Sobol', I. M.},
  title   = {On the distribution of points in a cube and the approximate
             evaluation of integrals},
  journal = {USSR Computational Mathematics and Mathematical Physics},
  volume  = {7},
  number  = {4},
  pages   = {86--112},
  year    = {1967},
  doi     = {10.1016/0041-5553(67)90144-9}
}
```

---

## Pre-submission note

Before publishing this repository, please replace `REPLACE_WITH_YOUR_USERNAME` in the BibTeX, APA, IEEE, and URL fields above with your actual GitHub username. The same replacement is needed in `CITATION.cff` (4 instances) and in `README.md` (1 instance in the BibTeX example).

---

*Madrid, May 2026.*
