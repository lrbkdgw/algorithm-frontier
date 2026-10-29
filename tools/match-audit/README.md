# matchaudit — auditing a claimed maximum-matching complexity

`matchaudit` is a harness for one narrow question:

> Someone says their program solves maximum matching in a claimed complexity
> class. Can that claim be **rejected** by evidence, and is their output even
> **correct**?

It cannot prove a bound. It can do two things that are checkable by a third
party, and it does them with an independent verifier rather than by trusting
the program under audit:

1. **Certify every returned matching.** Each output is checked for feasibility
   and then for maximality by an independent blossom search using Berge's
   augmenting-path criterion. A failure also yields an explicit augmenting
   path: a counterexample that needs no timing argument.
2. **Try to refute the growth claim.** The program is run on instance families
   built to stress each term of the claimed bound, the log-log growth is
   estimated, and it is compared with candidate complexity classes. A class is
   rejected when the measured growth is significantly steeper than the class
   allows, or when the whole shape fits worse than the noise floor.

Everything is recorded: protocol, families, sizes, repeat count, baseline,
noise floor, fitted parameters and thresholds. A `PASS` means "not refuted on
the tested families and sizes"; a `FAIL` is a counterexample.

## Build

```sh
cd tools/match-audit
make            # builds ./bin/matchaudit and the three example programs
make demo       # self-test, then three end-to-end audits
```

Requires a C++17 compiler and a Linux-like target. The harness uses `fork`,
`wait4`, `setrlimit` and `mkdtemp`; it has no third-party dependencies.

## The claim contract

Before measuring anything, fix what the program promises. A claim that is not
written down exactly cannot be tested, and most "we implemented the fast
algorithm" claims differ from the paper in one of these rows:

| Item | What to pin down |
|---|---|
| Bound and quantifier | exact function of the instance parameters; worst case, high probability or amortized; all sizes or "sufficiently large" |
| Model | word RAM or bit model; how long identifiers and comparisons are charged |
| Parameters | input edge records or distinct nonloop edges; vertex count of the original graph; is the optimum used as a parameter |
| Output contract | cardinality only, or an explicit matching with original edge identifiers |
| Exception handling | which branch runs below the asymptotic threshold, and whether that branch dominates the running time |
| Randomness | seed source, restart policy, failure probability |
| Space | peak records, and whether the reported bound includes the verifier or the certificate |

Write the answers into a copy of [AUDIT.template.md](AUDIT.template.md) before
running the tool. The report references that table.

## I/O protocol for the program under audit

The program is started once per instance and reads a file on standard input.

Input, first line, then one line per input edge record:

```
n m
u v
u v
```

Vertices are integers in `[0, n)`; loops and parallel records may appear and
both are part of the input. With `--protocol ids` each edge line carries a
third field, a 64-bit identifier chosen by the harness (`--id-kind sparse64`),
and the program must echo identifiers instead of endpoints.

Output: a cardinality, then that many matching edges, one per line.

```
k
u v        (or: identifier, under --protocol ids)
```

`--solver` is a shell command. `{input}` and `{output}` are replaced by the
harness paths; when `{output}` appears the program is expected to write its
answer to that file. Otherwise the answer is read from standard output and
standard input carries the instance.

Anything else — a solver that needs a custom loader, weights, or a different
format — is handled by a five-line adapter program that converts the protocol.

## Instance families

Random graphs are useless for this purpose: they hide every structure a fast
algorithm is supposed to exploit. Each family below stresses one term.

| Family | Stresses |
|---|---|
| `empty` | the linear scan and any adjacency-matrix setup; a matrix implementation fails here first |
| `path`, `cycle` | long search trees, deep alternating paths; vertex labels are relabelled so a greedy start cannot hit the optimum by luck |
| `complete`, `complete-bipartite` | dense input records, quadratic memory risk |
| `random-bipartite:P` | dense but bipartite: the classical baseline for the square-root factor |
| `triangle-union`, `c5-union`, `petersen-union` | disjoint odd cycles: blossom contraction load with matching number near a third or a half of the vertices |
| `grid` | sparse geometric graph with long distances |
| `star-clique` | adversarial for cover-based kernels |
| `random-gnp:P` | generic density sweep |
| `random-regular:D`, `expander3` | sparse near-expander, three random permutations |
| `parallel-heavy:K` | many duplicate records: normalization, and any step that is charged per record rather than per distinct edge |

`--sizes` is a starting ladder. Unless `--no-calibrate` is given, the harness
rescales each ladder so that its smallest point is measurably above process
startup; a ladder whose points are all inside the startup spread proves
nothing and the tool says so instead of guessing.

## Measurement

* One discarded warm-up run per point, then `--repeats` measured runs
  (default 5); the best wall time is kept. The best run is a consistent
  estimator and is insensitive to upward interference.
* A startup baseline is measured by running the same command on a
  one-vertex, zero-edge instance, and is subtracted. It is reported with its
  own spread, which sets the resolution limit. Points below it are marked
  `[noise]` and excluded from the fit; points whose repeats disagree by more
  than a quarter are marked `[noisy]` and kept, with the spread reported.
* Peak resident memory comes from the child's `rusage`, so a program that
  allocates a quadratic matrix is visible even when it is fast on small
  inputs.
* Optional `--ops-prefix STR`: the program may print `STR<integer>`, for
  example an internal comparison counter, on standard error. Operation counts
  are the only way to test the *claimed* cost model rather than wall clock,
  and they are trusted only as far as the program is trusted. Wall clock is
  always reported alongside.
* Per-run wall and CPU limits are enforced; a timeout is reported as a
  timeout, never as a slow point.

## Statistical method

Two independent tests, because either one alone is easy to fool.

**Whole-shape fit.** Six candidate models, each with one free multiplicative
constant, are fitted by least squares in log space. Two of them also have a
free shape parameter, searched on a grid and refined:

| Model | Meaning |
|---|---|
| claim | the claimed class with the repository's exponent shape |
| near-linear polylog | a strong near-linear baseline |
| square-root | the classical matching baseline |
| power law | a free exponent, which is the direct "what does it actually grow like" answer |
| cubic | naive blossom |
| vertex times edge | the classic quadratic-work blossom |

**Slope test.** The fitted free exponent is reported with its standard error
and the span of the ladder. The claimed class has exponent one in the driving
parameter, up to a factor that grows slower than any power. A size-independent
constant cannot turn a slope of 1.5 into a slope of 1.0, so if the fitted
exponent minus two standard errors exceeds 1.15 on a span of at least four,
the family is refuted even when the whole-shape residual is small. This case
really occurs: on a four-fold ladder, a quadratic program can be fitted by the
claimed shape to within two percent.

The noise floor is estimated from the observed repeat spread; the residual
threshold is the larger of eight percent and three times that spread. Both
numbers appear in the report.

## Verdicts and exit codes

| Verdict | Meaning |
|---|---|
| `CORRECTNESS FAILURE` | a returned matching was rejected, or the output could not be parsed. A definite counterexample; the complexity claim is moot |
| `REFUTED on this family` | the measured growth is significantly steeper than the claimed class allows |
| `consistent` | no contradiction on this family. Not a proof, and not a separation: a fast program with a large constant and a slow program can both land here |
| `consistent (not the best-fitting model)` | no contradiction, but another class fits better; read the reported exponents |
| `INCONCLUSIVE` | fewer than three usable points, or the ladder span is too short |

Exit status is `1` on a correctness failure or a refutation, `0` otherwise, and
`2` on a usage error. Use it in scripts only together with the report: exit
`0` does not mean the claim was proven.

## Cheaper checks first

`--crosscheck K` runs `K` random small instances and verifies only. It takes
seconds and settles the question that actually matters first: is the program a
correct maximum-matching solver at all? Two of the three example programs ship
with a defect that this mode catches immediately; see below.

```sh
./bin/matchaudit --solver ./bin/hopcroft_karp --crosscheck 24
```

## Worked examples

Auditing a correct but classical solver: it should be refuted.

```sh
./bin/matchaudit --solver ./bin/blossom_reference \
  --families empty,path,triangle-union,random-regular:3 \
  --sizes 500,1000,2000 --json report.json --md report.md
```

Auditing a program that is fast but only solves half the problem model: its
greedy fallback outside the bipartite case is caught, with the augmenting
path.

```sh
./bin/matchaudit --solver ./bin/hopcroft_karp --crosscheck 40
./bin/matchaudit --solver ./bin/hopcroft_karp --families grid,random-bipartite:4
```

Auditing a control program whose true cost is exactly linear: a harness that
rejects everything is worthless, so this one must not be refuted.

```sh
./bin/matchaudit --solver ./bin/trivial_scan --families empty \
  --sizes 1000000,2000000,4000000 --max-n 16000000
```

The `--self-test` mode checks the harness itself: every graph on up to nine
vertices against exhaustive dynamic programming, the augmenting-path
extraction, and the recovery of a known exponent from synthetic data.

## Limits

1. A correct implementation of an asymptotically better algorithm can be
   slower than a simple one at every size that fits in memory. A flat ladder
   never refutes a claim; only a steeper-than-allowed slope does.
2. The claimed exponent shape grows slower than any power, so over any finite
   ladder the claim and a power law are hard to separate. Report the fitted
   exponent and the span; do not turn a flat ladder into a proof.
3. Passing the audit establishes neither a worst-case bound, nor a constant
   independent of the input, nor determinism. Those need the proof and the
   source.
4. Verification is decided within a work budget; larger instances contribute
   feasibility evidence only and are marked as such. The summary reports how
   many instances were actually certified.
5. The audited program's own interpreter, runtime, startup and I/O are charged
   to it. A shell wrapper, a garbage collector or a slow loader can dominate
   every point.
6. This is a benchmark protocol, not a proof assistant. It is the same
   distinction the repository draws between executable finite checks and
   asymptotic proofs.

## Relation to this repository's matching claim

The claimed bound on the [general matching card](../../../problems/general-matching/)
is `O(n + m)` plus a factor of the form `exp(O(log(m_s)^(4/5) * loglog(m_s)^(1/5)))`
on the distinct-edges count `m_s`. Two consequences shape any audit of it:

* The first term is linear, so the `empty` and `parallel-heavy` families are
  not optional: an implementation that builds an `n`-by-`n` structure is
  refuted before any blossom runs.
* The second term is subpolynomial, so its constant is invisible at every size
  that fits in memory. No measurement can confirm that theorem. What
  measurement can do is reject a program that grows like the classical
  square-root factor while claiming the subpolynomial one, and certify its
  answers.

The repository's own executable solver
([`reference/matching.py`](../../../problems/general-matching/reference/README.md))
is certified at a polynomial bound and explicitly does **not** implement the
fast hierarchy, so auditing it should refute the fast claim and confirm
correctness. That is the expected, honest outcome.

## Files

| Path | Role |
|---|---|
| `matchaudit.cpp` | the harness: generators, independent verifier, runner, fits, reports |
| `AUDIT.template.md` | claim contract and result table to fill in before and after a run |
| `examples/blossom_reference.cpp` | correct classical solver: expected to be refuted |
| `examples/hopcroft_karp.cpp` | bipartite solver with a greedy out-of-scope fallback: expected to fail correctness |
| `examples/trivial_scan.cpp` | control program with linear cost: expected not to be refuted |

The example solvers are written for this directory and share algorithmic
ancestry with the harness's verifier, but not code. For a real audit the
independence that matters is between the *verifier* and the program under
audit, so verify with a program you did not write, or use a second harness.
