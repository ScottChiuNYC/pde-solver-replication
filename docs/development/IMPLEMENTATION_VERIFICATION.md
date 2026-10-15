# PDE implementation verification

This implementation follows the frozen `essence.md` without modifying it or its
audit history. The public numerical API is header-only C++17 under
`cpp/include/pde_software_replication_core/`; the installed umbrella is
`pde_software_replication_core/PDE.h`. The Python package keeps its existing
pybind11 `hello` interface, as the frozen domain scope excludes a Python solver API.

## Skeleton gate

Copier source: `https://github.com/ScottChiuNYC/copier-pypackage-cpp.git`.

| Item | Revision / evidence |
| --- | --- |
| Original recorded revision | `67630a4` = `67630a4d60c57ad89142cebd99b5c2e6da01f17a` |
| Existing latest tag at inspection | `v1.0.0`, still pointing to `67630a4d60c57ad89142cebd99b5c2e6da01f17a` |
| Explicit update target | `89ce8d23659ecc0a929284c535e6cd4c642e44d7` |
| New recorded revision | `v1.0.0-1-g89ce8d2` |
| Template validation | [Template Acceptance 38778240104](https://github.com/ScottChiuNYC/copier-pypackage-cpp/actions/runs/38778240104), Linux and Windows passed |
| Skeleton-only commit | `2bc6aae278c416f5b16c257f4845d3076d034754` |
| Skeleton-only CI | [Build and Test 38780244088](https://github.com/ScottChiuNYC/pde-solver-replication/actions/runs/38780244088), Linux and Windows passed |

Command: `python -m copier update --vcs-ref 89ce8d23659ecc0a929284c535e6cd4c642e44d7 --defaults`
with Copier 9.19.1. No update conflicts occurred. The update changed only the
Copier revision, recursive C++ source rule, and corresponding skeleton-contract
paragraph. The CMake rule is `GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS` over
`cpp/src/*.cpp`, including nested directories.

Before any domain implementation, `python scripts/bootstrap.py` and
`python scripts/validate.py --wheel` completed locally: 2 C++ tests, 3 Python
tests, sdist-to-wheel rebuilding, clean isolated wheel import, and relocated
Debug/Release vcpkg consumers with source headers hidden. Skeleton CI independently
passed the same validation on Ubuntu and Windows 2022.

The following original Git blob identities are preserved:

| File | Git blob |
| --- | --- |
| `docs/development/essence.md` | `4f8c036f9e916fcacce37ee56c72566af9f5b8ed` |
| `docs/development/ESSENCE_AUDIT.md` | `6c45e264c3d3812516ca4e4efcd8b796b4702f2c` |
| `docs/development/INITIALIZATION.md` | `490be5912ccf68b423323ce65412aa0fa0391bd3` |

## Architecture and contract gates

| Unit | Responsibility |
| --- | --- |
| `Types.h`, `Equation.h` | Callback types, recognized boundary enums, mutable PDE data and defaults |
| `Domain.h`, `Grid.h` | Checked finite domains, owned domain copy, uniform coordinates and exact endpoints |
| `Array.h` | Owning contiguous values, checked access, deep copies and empty moved-from state |
| `Tridiagonal.h` | Transactional no-pivot Thomas solve with original-row relative pivot checks |
| `Scheme/CrankNicholson1D.h` | Referenced configurations, theta validation, exact sampling set, centered non-divergence rows and Dirichlet corrections |
| `detail/Checked.h` | Separately rounded and checked binary64 arithmetic and overflow-safe endpoint means |
| `PDE.h` | Public umbrella and factory |

All numerical definitions are inline and depend only on the standard library.
Solve storage is O(nx), with exactly nt steps and no persistent trajectory or
factorization cache. Volatile binary64 stores preserve each specified arithmetic
operation and its finite check across inlining/contraction. No pivoting,
regularization, clipping, mesh/theta changes or method fallback is used.

The implementation gates were run in the required order:

1. Helpers: 4 GoogleTest groups; contract Section 6 compiled as C++17 through the
   public umbrella; every individual public header compiled in isolation.
2. Thomas: 6 groups covering scalar/multirow success, scales `2^-500`, `1`, `2^500`,
   all shape/alias/nonfinite roles, zero pivots/rows, exact relative thresholds,
   equality rejection, original coefficient preservation and generated overflow.
3. Stepping: 11 groups covering defaults/references/copies, independent returned
   arrays, setter validation, all missing and unsupported data, all required
   callback samples with NaN/+infinity/-infinity, positive diffusion, unchanged
   user exceptions, exact permitted sample sets including the skipped t0 source
   at theta 1, one-interior-row formulas, large-step heat modes, singular rows
   and large finite coefficient averages.
4. Numerical acceptance: 4 groups with the five exact frozen fixtures below.
   Combined with the two retained baseline groups, the suite has 27 C++ tests.

## Numerical evidence

Initial local acceptance: GNU 13.3.0, Release, C++17, Linux x64, binary64;
Python 3.12.14 for package validation. The exact grids, coefficients, initial
data, boundary data, source data and limits are those in frozen Section 7.1.

Errors use all returned nodes:
`E_inf = max(abs(error))`, `E_2 = sqrt(dx * sum(error^2))`.
Rates are `log2(E_coarse/E_fine)` and are checked in both norms at both adjacent
refinement pairs. Every numerical run prints complete `ACCEPTANCE` JSON records
containing grid/domain, theta, fixture, both errors, rates, compiler/build identity
and source revision. GitHub Actions records its actual checked-out `GITHUB_SHA`.

| Fixture | nx | nt | theta | E_inf | E_2 |
| --- | ---: | ---: | ---: | ---: | ---: |
| Space, all terms | 21 | 4096 | 0.5 | 1.3799853132e-3 | 9.7275826931e-4 |
| Space, all terms | 41 | 4096 | 0.5 | 3.4547449797e-4 | 2.4315913917e-4 |
| Space, all terms | 81 | 4096 | 0.5 | 8.6368836470e-5 | 6.0787891069e-5 |
| CN time, variable coefficients/boundaries | 41 | 20 | 0.5 | 1.0322351844e-6 | 7.5754378418e-7 |
| CN time, variable coefficients/boundaries | 41 | 40 | 0.5 | 2.5801381698e-7 | 1.8935410903e-7 |
| CN time, variable coefficients/boundaries | 41 | 80 | 0.5 | 6.4500643937e-8 | 4.7336538363e-8 |
| Theta time | 41 | 10 | 0.75 | 1.2667045989e-4 | 9.0001921959e-5 |
| Theta time | 41 | 20 | 0.75 | 6.3633555192e-5 | 4.5211726119e-5 |
| Theta time | 41 | 40 | 0.75 | 3.1895673495e-5 | 2.2661604346e-5 |
| Theta time | 41 | 10 | 1 | 2.4818346422e-4 | 1.7634989254e-4 |
| Theta time | 41 | 20 | 1 | 1.2596887967e-4 | 8.9503692596e-5 |
| Theta time | 41 | 40 | 1 | 6.3465710149e-5 | 4.5092504911e-5 |
| Fixed heat | 101 | 1000 | 0.5 | 3.0224654256e-5 | 2.1372057983e-5 |

| Refinement | First pair p_inf / p_2 | Second pair p_inf / p_2 | Required interval |
| --- | --- | --- | --- |
| Space, all terms | 1.998002 / 2.000181 | 1.999996 / 2.000045 | [1.85, 2.15] |
| CN time | 2.000251 / 2.000243 | 2.000063 / 2.000061 | [1.8, 2.2] |
| Theta 0.75 time | 0.993220 / 0.993259 | 0.996427 / 0.996447 | [0.85, 1.15] |
| Theta 1 time | 0.978340 / 0.978422 | 0.989018 / 0.989060 | [0.85, 1.15] |

All finest-grid bounds and pairwise rate bounds passed. Initial numerical output
was produced while assembling this change; final CI logs provide source-linked
acceptance records on both supported platforms.

## Final validation

Local `python scripts/validate.py --wheel` passed after implementation:

- 27 C++ tests and 3 retained Python tests passed.
- Every public domain header compiled independently as C++17.
- The platform-native wheel rebuilt from the unpacked sdist and imported in a
  clean virtual environment outside the source tree.
- The local vcpkg port installed the complete public header hierarchy and its
  existing static core/config exports without discovering Python or pybind11.
- Independent two-translation-unit solver consumers compiled, linked and ran
  from the relocated install in Release and Debug with source headers hidden.
  Their heat maximum error was approximately `3.02247e-5`.

Implementation CI evidence is appended after the Linux and Windows runs finish.
