# NEW_DISCOVERIES

**Living journal for findings made after the formal closure of Proyecto Estrella (10 May 2026).**

This file exists so that any subsequent work — by the project's original author or by anyone who picks up the veta — can be recorded next to the closed record, without polluting the canonical citation (`MAIN_DISCOVERIES.md`, `PAPER_TERMINAL.md`) or the historical record.

---

## How to use this file

When you find something that:
- improves on `COMBO_3027` (audit < 3027 with D190 verification);
- improves on `E17_seed4_3095` on Genz integration error in Gaussian dim ≥ 20;
- reveals a new basin with a previously undocumented Genz specialisation;
- refutes a finding from `MAIN_DISCOVERIES.md` empirically;
- proposes a methodological refinement that survives Auditor disenta;
- replicates or extends any project result independently;

**add a new dated entry below the most recent one**. Use the template at the end of this file.

Each entry should be:

- **Reproducible against files in the repository** (numbers from files only).
- **Verifiable in minutes** by a third party with a laptop.
- **Honest about scope** — what it shows and what it does not show.
- **Either delivered or labelled "open"**. No "I will publish the data next month" entries. Either the file is here or the entry is marked as work-in-progress.

---

## Empty marker

*No new discoveries have been recorded yet. The project closed on 10 May 2026 with the contributions documented in `MAIN_DISCOVERIES.md`.*

---

## Template for a new entry

Copy and adapt:

```markdown
## YYYY-MM-DD — Short descriptive title

**Author**: <name and contact>
**Severity / kind**: <improvement | refutation | replication | methodology | open question>

**Statement.** <one paragraph stating the finding in precise terms>

**Evidence.**
- File: `<path/to/added/file>`
- Verification command: `python3 <script> <args>`
- Expected output: `<...>`

**Caveats.** <what this does not show; known limitations>

**Status.** <delivered | replicated by N parties | under review | open>
```

---

*This file is meant to grow over time. If a finding here matures into a substantive contribution worth standalone citation, it can be promoted into a future paper or into a revision of `MAIN_DISCOVERIES.md`; in that case, leave a back-pointer here noting the promotion.*
