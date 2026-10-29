# Complexity audit — record sheet

Copy this file next to the program under audit and fill it in. Anything left
blank is an untested claim; the audit cannot be more precise than this sheet.

## 1. The claim, before any measurement

| Field | Value |
|---|---|
| Program and version / commit | |
| Exact claimed running time | |
| Driven by which parameters | vertex count, input edge records, distinct nonloop edges, matching number, something else |
| Worst case, high probability, amortized, or expected | |
| All inputs, or inputs above an internal threshold | |
| Threshold value, and which branch runs below it | |
| Machine model and what is charged | word operations, bit operations, comparisons of long identifiers, I/O |
| Output contract | cardinality, explicit matching, original edge identifiers, certificate |
| Space bound | |
| Randomness | seed source, restart policy, failure probability |
| Deterministic on repeated runs | |

## 2. Static questions (no runs needed)

- [ ] Is there a mapping from each theorem or lemma to the code that realizes it?
- [ ] Does any code path fall back to a different algorithm when the fast
      branch rejects the input, exceeds a depth, or hits a guard?
- [ ] Are the data structures the worst-case versions the proof assumes, or
      amortized ones?
- [ ] Is the input read more than once?
- [ ] Is any per-call work linear in the whole instance, inside a loop that
      the proof counts separately?
- [ ] Are the sizes the proof charges for actually built, or is a large ground
      set handled implicitly while an explicit one is allocated anyway?
- [ ] Does the program use randomness, hash-ordered iteration, or parallelism?

## 3. Correctness evidence

Command:

```sh
bin/matchaudit --solver "<command>" --crosscheck 40
```

| Item | Result |
|---|---|
| Instances checked | |
| Rejected outputs | |
| Longest augmenting path reported | |
| Families covered by the sweep | |

A single rejected output ends the audit: the claim is about solving the
problem, so an incorrect answer makes the running time irrelevant.

## 4. Growth evidence

Command:

```sh
bin/matchaudit --solver "<command>" --families <list> --sizes <ladder> --json report.json --md report.md
```

| Family | usable points | fitted exponent | standard error | span | claim residual | verdict |
|---|---|---|---|---|---|---|
| | | | | | | |

## 5. What this audit does not establish

- It does not prove the claimed bound, at any size.
- It does not separate the claimed subpolynomial factor from a power law over
  a finite ladder.
- It does not check the proof, the constants, or determinism.
- A `consistent` verdict on a flat ladder is compatible with a much slower
  program whose constant is large.

## 6. Conclusion

| Question | Answer |
|---|---|
| Is the program a correct maximum-matching solver on the tested inputs? | |
| Is the claimed class refuted anywhere? | |
| Which family produced the sharpest evidence? | |
| What would change the answer (larger sizes, different family, source access)? | |
