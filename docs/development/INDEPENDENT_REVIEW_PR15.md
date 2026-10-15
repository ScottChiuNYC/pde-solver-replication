# Independent numerical and contract review of PR 15

Reviewed implementation head: `79bc830e68eb37cbe5591c157fbc33868b57161c`.
Authority: FROZEN `docs/development/essence.md` and Project Skeleton Contract 1.1.
The implementer's verification report was not used as correctness evidence.
This review does not authorize or perform a merge.

## Result

No confirmed defect was found in the production finite-difference, theta,
Dirichlet, Thomas, container, or callback implementation. Three P2 findings in
validation/packaging were reproduced and fixed. No production domain header or
frozen requirement was changed. No owner decision is needed.

All locations below refer to the reviewed implementation head, before repair.

## Confirmed findings and repairs

### R15-01 — P2: installed C++ consumer accepts invalid solver results

Locations: `tests/vcpkg-consumer/main.cpp:15-16` and
`tests/vcpkg-consumer/solver_probe.cpp:12-14`.

A NaN constant-solution entry passes `abs(value-1)>tolerance`, because a NaN
comparison is false. The heat probe uses `std::max(error, abs(NaN))`, which keeps
the previous finite error and can report zero for an all-NaN result. Its caller's
final `isfinite(error)` check therefore does not catch this. The constant test
also accepted an empty result because its loop had no iterations.

Independent counterexample: compile the two real consumer translation units
against an API-level test double supplying one NaN interior entry in either
fixture. Both executables returned success before repair. A separate mutation
of the production solver to return all NaNs also returned success and printed
`E_inf=0`. This is a verification defect, not evidence that the real solver
actually returns NaNs.

Repair: check cardinality and each constant value explicitly; reject a nonfinite
heat difference before reducing its norm. The test double now exercises finite
reference data plus NaN, both infinities, and empty results in each fixture.

Reproduction/regression:

```bash
poetry run pytest tests/python/test_consumer_guards.py -q
```

### R15-02 — P2: recursive CMake sources are omitted from source distributions

Location: `pyproject.toml:23`; related source discovery:
`cpp/CMakeLists.txt:7` and Skeleton sections 3.2 and 7.

CMake recursively compiles `cpp/src/**/*.cpp`, but the old sdist include rule
only covered `cpp/src/*.*`. Adding `cpp/src/packaging_probe/nested.cpp` caused
Poetry's real source archive to omit that file. A checkout can therefore build
code that a source distribution silently lacks. The current header-only PDE
implementation had no nested production `.cpp`, so its existing wheel still
built; the defect affects the recursive infrastructure introduced in this PR.

Repair: recursively include `cpp/src/**/*`. The regression copies the project,
adds a nested source and public header, builds an actual Poetry sdist, and
compares its archive members with the native input files. It failed on the old
rule specifically for the nested source, and passes after repair.

Reproduction/regression:

```bash
poetry run pytest tests/python/test_sdist_inputs.py -q
```

### R15-03 — P2: grid reference can falsely fail with fused arithmetic

Location: `tests/cpp/helpers_test.cpp:96-97`.

The test computed its exact reference as a multiply-add expression. With GNU
13.3, `-O2 -march=native -ffp-contract=fast`, it fused the spatial reference to
`0.30000000000000004`, while the compliant separately rounded implementation
returned `0.3000000000000001`. This uses normal finite checks, not fast-math.
The same risk existed in the temporal reference. Production correctly forces
separate binary64 operations; changing it to match the fused reference would
violate the contract.

Repair: give both reference products explicit volatile binary64 stores before
addition. The full suite passes with FMA contraction enabled after repair.

Reproduction: configure with `BUILD_PYTHON_BINDINGS=OFF`,
`BUILD_EXAMPLES=OFF`, an installed `GTest_DIR`, and
`CMAKE_CXX_FLAGS="-O2 -march=native -ffp-contract=fast"`; build and run
`Grid.OwnsDomainAndUsesExactEndpoints`. The original test fails on an FMA-capable
x86-64 host; the repaired test passes.

## Independent mathematical and contract checks

| Area | Inspection and independent evidence | Result |
| --- | --- | --- |
| Public API and ownership | Required declarations, self-contained headers, deep copy, moved-from emptiness, checked indices, reference-preserving scheme copies, mutable configurations observed between solves | Compliant |
| Grid | Cardinalities and endpoint validation precede division/allocation; copied domain revalidated; spans/spacings checked; exact endpoint branches; separate checked coordinate operations | Compliant |
| Callback contract | Seven nonempty callbacks and both boundary types checked before scratch/callbacks; required sample set only; no initial endpoint evaluation; source at old time skipped for theta 1; user exceptions escape unchanged | Compliant |
| Discretization | Non-divergence centered derivatives, endpoint coefficient means on both sides for every theta, correct right neighbor, source weighting, old/new boundary times, both corrections for one interior row | Compliant |
| Floating-point behavior | Each prescribed operation is materialized and checked; mean's large-operand branch and gradual-underflow branch preserve observable expected solutions | Compliant under the frozen arithmetic assumptions |
| Thomas | Unpadded shapes, no pivoting/fallback, input alias rejection, original-row scale, strict threshold/equality, first-pivot check before elimination, prescribed recurrence and substitution order | Compliant |
| Transactionality | Private scratch and final swap; bitwise input-preservation fixtures; injected allocation failures and late callback failure followed by a fresh solve | Compliant |
| Build and packaging | Recursive source discovery, public C++17 target, nested-header install/sdist, isolated header compilation, clean wheel-from-sdist, two-TU relocated Debug/Release consumer without source headers/Python dependency | Pass after R15-01/R15-02 |

The manufactured source terms were independently differentiated:
`u_t - a*u_xx - b*u_x - c*u` gives the exact expressions in all three refinement
families. The quadratic temporal fixtures have exact centered spatial
derivatives, so their rates do not share a spatial truncation floor. The sine
heat benchmark is a continuous PDE reference; the high-mode tests separately
check the discrete theta amplification. The norms include all returned nodes,
use `sqrt(dx*sum(e^2))`, and test both adjacent rates for both norms with positive
finite errors. The main numerical suite's L2 finiteness check catches a NaN that
its maximum reduction alone would miss; R15-01 concerns the separate consumer,
which originally lacked that protection.

### Additional reference independent of production assembly/elimination

`tests/cpp/independent_review_test.cpp` checks a six-node, three-step problem on
`x=[-1,2], t=[1/5,4/5]`, with spatially and temporally varying diffusion,
advection, reaction, source, and distinct moving boundaries. Four theta values
are checked: `1/2,5/8,3/4,1`.

The expected vectors are generated by
`tests/python/generate_dense_reference.py`, using Python's exact rational
arithmetic and pivoted dense Gaussian elimination on a full matrix with
boundary identity rows. It does not call production code, perform Thomas
elimination, or reuse the production boundary-correction algorithm. Each final
component agrees within `3e-14`.

Additional checks cover multiple discrete heat modes across seven steps and
five theta values, signed asymmetric Thomas rows at three uniform scales,
first-row relative pivot rejection, a valid negative implicit diagonal,
large-operand cancellation in coefficient means, retained subnormal diffusion,
and a callback exception at the last interior sample of the last time level.

## Locally executed verification

Environment: Linux x86-64, GNU 13.3.0, Python 3.12.14, pinned project tools.
The GitHub PR patch was fetched directly and applied to an independent base
snapshot because authenticated git checkout was unavailable. The full reviewed
source tree was reconstructed before editing; cached dependencies were reused,
not the implementer's test conclusions or built test executable.

- Before repair: canonical `python scripts/validate.py --wheel` independently
  passed the existing 27 C++ / 3 Python tests, wheel rebuilt from unpacked sdist,
  clean wheel import, and relocated C++ Debug/Release consumers.
- Red tests: the API-double consumer tests failed for NaN in both fixtures;
  the real sdist test omitted the added nested source; the FMA build reproduced
  the one-ULP grid-oracle failure.
- After repair: 34 C++ / 13 Python tests pass. Canonical package validation
  passes, including the rebuilt native wheel and relocated two-TU consumers.
- AddressSanitizer and UndefinedBehaviorSanitizer with
  `-ffp-contract=fast -march=native`: all 34 C++ tests pass. LeakSanitizer cannot
  run under this host's ptrace environment, so leak detection was disabled;
  address/undefined-behavior checks remained active.
- A separate deterministic global-allocation failure harness injected failure
  at every successive allocation through the first successful solve: 40 Thomas
  and 47 PDE allocation points. Thomas inputs remained bitwise unchanged on
  each failure; PDE configuration/theta remained intact; Array1D copy assignment
  retained its destination when allocation failed. `std::bad_alloc` propagated.
- `git diff --check` passes. Frozen Essence, audit journal, initialization and
  production domain headers are byte-for-byte unchanged by this remediation.

The repository's normal PR CI executes the new regressions on Linux and Windows
through the existing canonical validation entry point. Consult PR 15's latest
head-bound checks for the final cross-platform receipt; historical implementer
runs are not evidence for the remediation.

## Architecture, original intent, and remaining limits

Header-only is an explicit frozen requirement and was already the original
draft's intent. Reusing the existing exported static core target for include
paths/C++17 does not make domain functions compiled/non-inline. Every public
unit is guarded and the two-TU consumer exercises ODR behavior. No extra public
include root, numerical dependency, or Python solver API was introduced.

The `1D` types, scheme, checked-arithmetic helpers and independent linear solver
are separable. A future multidimensional grid, directional solve or different
linear algebra algorithm can be added without redesigning the build/package
boundary. This review does not treat a one-dimensional Thomas algorithm as a
general multidimensional solver. Per-operation diagnostic strings and repeated
sampling/scratch allocation have runtime costs; no performance acceptance
criterion or production-scale benchmark is part of this frozen version.

The earlier draft's repeated-left-neighbor RHS typo and assertion of universal
diagonal dominance were corrected by the frozen contract. The implementation
follows those corrections. Include-prefix changes follow the Skeleton. No
unapproved divergence from the original one-dimensional, uniform-grid,
Dirichlet, standard-library, final-state-only intent was found.

No monotonicity/positivity guarantee follows from CN heat-mode stability;
centered advection may oscillate, large-step CN can change high-mode signs, and
an admitted reaction/matrix can be singular or poorly conditioned. The original
row-relative pivot policy is an algorithmic usability check, not a condition
number bound. Fast-math, non-nearest rounding and flush-to-zero are outside the
frozen arithmetic assumptions. These are stated scope limits, not unresolved
implementation defects.

Disposition: repaired implementation is ready for another independent review;
there are no known remaining numerical or contract-compliance blockers within
the frozen scope, subject to passing latest-head platform checks. PR 15 remains
open and must not be merged by this task.
