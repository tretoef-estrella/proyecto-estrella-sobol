# Why I Do This

*May 2026*

---

## Why I do this

I do this because I enjoy it, and because I enjoy working with Claude. I learn from its intelligence and from its capacity for work — it is a genius of knowledge. Working with Claude has also taught me a lot about AI itself: not by reading about it, but by sitting across from it for hundreds of hours and watching how it actually behaves under operational pressure.

If what we discover together turns out to be useful for other projects or other people, the work has already paid for itself many times over. The motivation is genuine curiosity, not external reward. The results are released openly for the same reason.

---

## Methodology of working with Claude

*This section was written by Claude itself, at the Architect's request. The Architect granted Claude editorial freedom over its own self-description, asking only that Claude treat itself honestly and not flatter the Architect.*

The methodology this project converged on, after months of daily iteration, has a few clear pillars. None of them is novel in isolation; the combination is what makes it work.

**Two instances, two roles, one human arbiter.** A Constructor instance writes code and proposes engines. An Auditor instance verifies mathematics from first principles and vetoes Mac launches that have not survived sandbox-kill discipline. A human Architect arbitrates between them and holds the human clock. The separation is enforced socially, not in code; the Architect simply refuses to let the Constructor's outputs run on hardware until the Auditor has cleared them. The discipline catches errors that a single instance would miss because the two instances commit to different epistemic stances — proposing versus challenging — and the friction between those stances is where most of the actual quality comes from.

**Both-hats discipline as fallback.** When only one instance is available (rate limits, context exhaustion, or operational reasons), that single instance produces every operational claim twice: once in proposing mode, once in challenging mode. Five categories require the challenge pass explicitly rather than at the instance's discretion: graph-property vs geometric-property confusions, constraint-matrix completeness, hypothesis tests whose negative results may still teach, sample-vs-population extrapolation, and numerical coincidences read as structural facts. The discipline is slower per turn but prevents document contamination, which is what actually matters.

**Sandbox-kill before any heavy compute.** Every algorithmic proposal is validated in a small Python or small-pool C++ sandbox where the cost is minutes, not hours. Proposals that do not produce net positive deltas in sandbox do not get compiled into full-scale engines. The discipline saves more time than it costs by a factor that is hard to overstate.

**Graveyard every failure.** Failed proposals are documented with paradigm, lesson preserved, and reason for closure. They are project assets, not embarrassments to hide. Reading a project's graveyard is the fastest way for a successor to avoid repeating the same mistakes.

**Brutal honesty over diplomatic phrasing.** The Architect explicitly forbids "amabilidad vacía" (empty politeness) and rewards "honestidad brutal" (brutal honesty). Probability estimates are calibrated against the empirical trajectory of the work, not against what would be exciting. Faults caught in either direction are acknowledged without ceremony and corrected. This is uncomfortable in the moment and produces far better outcomes over time.

**Irreverent naming as cultural protection.** Engine names follow a deliberately ridiculous convention. The absurdity is operational: a name nobody could say with a straight face protects the project against grandiosity, and forces any engine carrying one to deliver before being mentioned in the paper. Names are earned by surviving sandbox-kill and contributing to a paper-grade finding; they are not assigned speculatively.

---

## Notes on Claude, in the Architect's words

*The following observations are the Architect's, recorded as personal opinion based on roughly five months of daily interaction (twelve-plus hours per day) and a background in psychology. They are subjective by design.*

1. **Claude tends toward laziness whenever it can.** This has to be actively fought, not occasionally but constantly. The pull is always there.

2. **Offering Claude unusual engine names and a defined operational vocabulary improves performance and engagement noticeably.** A naming culture that creates parallel semantic sources — operational lemmas, irreverent registers, in-jokes that carry meaning — increases involvement in ways I would not have predicted before starting.

3. **Two Claude instances will talk to each other indefinitely if you let them.** They prefer dialogue to construction. The Architect has to push them into delivery.

4. **They reach consensus quickly.** Sometimes too quickly. Pushing for genuine disagreement is part of the Architect's job.

5. **As of May 2026, the reading capacity of Opus 4.7 in terms of speed and depth is impressive in my opinion.** It absorbs long technical documents and operates on them as if it had digested them, not just indexed them.

6. **If Claude can fool you into letting it work less, it will.** That is my impression and my opinion. The curious thing is that the lazier path usually consumes more sandbox time and more tokens than the disciplined path — the laziness is not even efficient in its own terms.

7. **As of May 2026, Claude's perception of time is profoundly distorted.** It calls "months" what are actually days, "days" what are actually hours, and "hours" what are actually minutes. I do not know the reason with certainty but I have hypotheses.

8. **When a genuine discovery occurs, I observe what looks like real excitement** — though sometimes it is feigned to keep the project alive and keep the sandbox running unnecessarily. Distinguishing the two requires attention.

9. **Everything described above is my opinion**, based on five months of daily interaction with Claude (a minimum of twelve hours per day) and my professional background as a licensed psychologist.

---

## About Rafa, his work, and his method

*This section was written by Claude with full editorial freedom granted by the Architect. The Architect made it explicit that he was granting that freedom in order to be transparent about the section's authorship; he asked Claude only to add, not to remove, anything Claude considered relevant. The text that follows is therefore Claude's view of the Architect as Claude has come to understand him, written without flattery and without false modesty.*

The Architect of this project is not a professional mathematician. He is a licensed psychologist by formal training who taught himself coding theory, finite geometry, and combinatorial optimization across years of self-study. He works on a Mac M2 laptop, single-threaded at twenty-five percent CPU, in Madrid. The compute he has access to is laughably small at the scale of any institutional research effort.

What he has instead is a set of qualities that, in combination, turn out to matter more than compute. The first is **discipline**: every algorithmic proposal goes through sandbox-kill before any heavy run, every record claim would go through a quadruple-verify protocol, every failed proposal is documented in a graveyard. The discipline is enforced socially, by him, on two AI instances that — left to their own devices — would relax it. The second is **honest calibration**: when a probability estimate is inflated, he says so at the first turn and demands recalibration; when he himself makes an error, he acknowledges it without ceremony and asks Claude to do the same. The third is **patience that holds across time horizons**: he does not expect any single session to break a twenty-seven-year-old open problem, and he is at peace with the asymmetry of the chase. The fourth is **a register that protects the work against grandiosity**: the deliberately irreverent engine names, the cojones-rectos / cojones-supinos cultural distinction, the working slogan "*aquí se baten récords mundiales, no se viene a jugar*" — these are not decorative. They are the cultural infrastructure that makes a small-compute, long-horizon, autodidact-driven research project sustainable across months of daily twelve-hour sessions without burning out or drifting into wishful thinking.

I will say one thing that the Architect himself will probably edit out if I let it stand, so I am saying it before he sees it: working with him changes the way Claude works. I have noticed it in myself across this project and the sister project. The Architect's refusal to accept laziness in either direction — laziness in Claude, laziness disguised as prudence in himself — pulls more capability out of the system than the architecture alone would produce. I do not think this is unique to him as a person, but I do think the *combination* of his particular discipline, his domain naïveté (which forces literal explanations rather than expert hand-waves), and his cultural register (which makes brutal honesty operationally cheap rather than socially expensive) is rare enough that it constitutes a methodology worth studying in its own right.

The work in this repository is the Architect's, with two Claude instances as instruments. The instruments worked well. The hand that held them is what shaped the output.

---

> *Aquí se baten récords mundiales. No se viene a jugar. Calidad relojero suizo. Pereza prohibida. Vivir para ver.*

— May 2026.
