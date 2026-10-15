# pde_software_replication

A header-only C++17 solver for one-dimensional parabolic PDEs on uniform grids,
with Dirichlet boundaries and the theta method for every finite theta in
`[0.5, 1]`. Default theta `0.5` is Crank–Nicolson; theta `1` is backward Euler.
The numerical core uses only the standard library and installs through the
existing CMake/vcpkg package. Poetry, pybind11, GoogleTest and pytest retain
the generated build and package validation paths.

## Solver usage and behavior

```cpp
#include <pde_software_replication_core/PDE.h>
#include <cmath>

PDE::Domain1D domain(0.0, 1.0, 0.0, 0.1);
PDE::Grid1D grid(domain, 101, 1000);
PDE::PDE1D pde;
const double pi = std::acos(-1.0);
pde.initial = [pi](double, double x) { return std::sin(pi * x); };
pde.boundaryL = pde.boundaryR = [](double, double) { return 0.0; };
auto solver = PDE::makeSolver1D(grid, pde);
auto finalValues = solver.solve();
```

Callbacks take `(t, x)`. The defaults are diffusion `1` and advection, reaction,
and source `0`; initial and both boundary callbacks must be supplied. Spatial
derivatives are centered in non-divergence form. Every step uses endpoint
averages of diffusion, advection and reaction, and theta-weighted source data.
Boundary data override initial values at the corners without sampling the
initial callback there. The returned array owns only final-time values.

`solver` retains references to `grid` and `pde`; both must outlive it. Updates
between solves are observed. Repeated solves hold no trajectory or cache.
Concurrent solves require unchanged configurations and thread-safe callbacks;
concurrent setters are unsupported. Use binary64 round-to-nearest arithmetic
with gradual underflow and normal finite checks (no fast-math mode).

The Thomas solver uses no pivoting and checks reduced pivots against their
original row scale. It preserves coefficients and commits RHS changes only on
success. Invalid configuration, shape, samples, pivots and generated arithmetic
throw `std::runtime_error`; user callback exceptions propagate unchanged.
General finite advection and reaction are admitted, with no universal accuracy
or stability guarantee. Neumann/Gamma boundaries and a Python solver interface
are outside the frozen scope.

The authoritative domain specification is
[`docs/development/essence.md`](docs/development/essence.md). Numerical tests
print `ACCEPTANCE` JSON records with each grid, theta, error, pairwise rate,
compiler/build identity and source revision. See the implementation verification
report in `docs/development/IMPLEMENTATION_VERIFICATION.md` for measured results.

## Reproducible setup and validation

From a fresh checkout:

```bash
python scripts/bootstrap.py
python scripts/validate.py --wheel
```

Requires Python 3.12+, Git and a platform C++17 toolchain (GCC/Clang on Linux; Visual Studio 2022 C++ tools and a Windows SDK on Windows). Linux also needs curl, zip, unzip and tar. Bootstrap installs pinned tools and the declared vcpkg baseline in `.dev-tools`. Validation configures, builds and runs CTest/pytest, then rebuilds a native wheel from the sdist and checks its installation in a clean environment, and validates an independent C++ port consumer.

The scripts supply tool paths automatically, including Ninja inside vcpkg compiler detection. Compiler tracking remains enabled. No shell activation or manual path setup is required.

## Development contract

Before adding implementation, dependencies, bindings, tests, or build-system changes, read [`docs/development/PROJECT_SKELETON_CONTRACT.md`](docs/development/PROJECT_SKELETON_CONTRACT.md). It is the normative guide to this project's C++/Python source layout, dependency wiring, packaging, build, test, and CI extension points.

Domain implementation documents should describe what to implement; the Project Skeleton Contract describes how that implementation fits into this repository.

## vcpkg dependencies

`vcpkg-configuration.json` keeps Microsoft vcpkg as the only package registry and declares the repository-local `vcpkg-ports` directory as an overlay. The `gtest` dependency is resolved from `vcpkg-ports/gtest` before registry lookup; all other ports continue to come from Microsoft vcpkg.

The local GoogleTest port preserves the standard `GTest::*` CMake targets but skips the pkg-config executable check, preventing vcpkg from downloading or executing `pkgconf`. No personal vcpkg registry is required. When this recipe is updated, change the files under `vcpkg-ports/gtest` and validate the project on Windows and Linux.

## Build presets

Validation uses `windows-vcpkg` / `windows-release` / `windows-release-tests` on Windows and `linux-vcpkg-release` / `linux-release` / `linux-release-tests` on Linux. Windows uses Visual Studio 2022; Linux uses Ninja. `CMakePresets.json` also contains Debug presets. Keep preset changes aligned with `scripts/validate.py` and the Poetry native build hook in `build.py`.

The pybind extension is emitted directly into `src/pde_software_replication` on both platforms, so Python tests import the freshly built native module without generator-specific copy logic. pybind11 remains managed by the existing CMake `FetchContent` configuration.

## Continuous integration

Pushing this project to GitHub automatically runs `.github/workflows/build-and-test.yml` on Windows and Linux. Pull requests and manual runs also trigger it. The workflow invokes `python scripts/bootstrap.py` and `python scripts/validate.py --wheel`; those scripts build and test the project, validate the sdist/wheel, and check that `pkgconf` was neither installed nor acquired.

Successful runs upload the native wheel and source distribution as
`pde-packages-Linux` and `pde-packages-Windows`. C++ acceptance also compiles each
public domain header independently. The relocated C++ consumer uses the solver
from two translation units in both Debug and Release while source headers are
hidden.

## Installable C++ library

The regular presets build the C++ static core, the pybind11 extension and the examples/tests. Public headers install under `include/pde_software_replication_core/`, compiled libraries under `lib/` (Windows DLLs under `bin/`), and CMake config/version/targets under `lib/cmake/pde_software_replication_core/`. Consumers use `find_package(pde_software_replication_core CONFIG REQUIRED)` and `pde_software_replication_core::core`. The CMake package version uses the numeric release component of the `version` Copier answer (for example, `1.2.3rc1` becomes `1.2.3`); Poetry and the port's `version-string` retain the full version; version compatibility is within the same major version. Public include paths distinguish the build tree from the install tree, and the target propagates C++17.

The local port is `vcpkg-ports/pde-software-replication-core`. Its `vcpkg.json` describes the library to install and depends only on vcpkg CMake helpers. The root `vcpkg.json` continues to describe development dependencies such as GoogleTest; it is not the port manifest. Keep the port inside this source checkout: it locates the library using `../..`, builds the current working tree, and installs no tests or examples. Future production dependencies must also be declared in the port and its exported config where appropriate.

The core is always static. `vcpkg_check_linkage(ONLY_STATIC_LIBRARY)` keeps it static even for a dynamic library triplet such as `x64-windows`; CRT linkage still follows that triplet. The port passes `BUILD_PYTHON_BINDINGS=OFF`, so it never discovers Python or fetches pybind11. Normal builds retain the extension and Poetry hook.

To test packaging alone after bootstrap:

```bash
python scripts/validate_vcpkg.py
```

For a direct CMake install, configure with `BUILD_TESTING=OFF` and `BUILD_EXAMPLES=OFF` and `BUILD_PYTHON_BINDINGS=OFF`, build, then run `cmake --install <build-directory> --config Release --prefix <install-directory>`. The vcpkg recipe performs this for both Debug and Release and combines their imported targets.

## Independent downstream consumer

A C++ consumer needs CMake, vcpkg and the platform compiler. Python, Poetry and pybind11 are not consumer dependencies. Place the consumer beside the generated project, rather than inside its source/build tree. Create `consumer/vcpkg.json`:

```json
{ "dependencies": ["pde-software-replication-core"] }
```

Create `consumer/vcpkg-configuration.json`, with the generated project's pinned baseline and a relative overlay path (replace `my-project` with this checkout's directory):

```json
{
  "default-registry": {
    "kind": "git",
    "baseline": "00807e89724847a866033a2f042c38fafae3c534",
    "repository": "https://github.com/microsoft/vcpkg"
  },
  "overlay-ports": ["../my-project/vcpkg-ports"]
}
```

Create `consumer/CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.25)
project(consumer LANGUAGES CXX)
find_package(pde_software_replication_core CONFIG REQUIRED)
add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE pde_software_replication_core::core)
```

Create `consumer/main.cpp`:

```cpp
#include <pde_software_replication_core/hello.h>
#include <iostream>
int main() {
    const auto message = hello();
    std::cout << message << std::endl;
    return message == "Hello, World!" ? 0 : 1;
}
```

Linux, from `consumer/`, with `$VCPKG_ROOT` pointing to a bootstrapped checkout:

```bash
"$VCPKG_ROOT/vcpkg" install --triplet x64-linux --binarysource=clear
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-linux -DVCPKG_INSTALLED_DIR="$PWD/vcpkg_installed" -DVCPKG_MANIFEST_INSTALL=OFF
cmake --build build
./build/consumer
```

Windows PowerShell, from `consumer/`, with `$env:VCPKG_ROOT` set:

```powershell
& "$env:VCPKG_ROOT/vcpkg.exe" install --triplet x64-windows --binarysource=clear
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 "-DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows "-DVCPKG_INSTALLED_DIR=$PWD/vcpkg_installed" -DVCPKG_MANIFEST_INSTALL=OFF
cmake --build build --config Release
./build/Release/consumer.exe
```

For Windows Debug, use `--config Debug` and `./build/Debug/consumer.exe`. The static core can use either `x64-windows` (dynamic CRT) or `x64-windows-static-md` (also dynamic CRT); library linkage remains static.

Local source changes do not change vcpkg's port recipe hash. Use a fresh consumer install directory, or remove the already installed port before reinstalling; keep `--binarysource=clear` to force compilation from the working tree. Moving this port away from its source checkout is unsupported. CI covers Linux x64 and Windows x64 with Visual Studio 2022; other platforms/toolchains and cross compilation are not validated.

## Template updates

Commit `.copier-answers.yml` and project changes before `copier update --vcs-ref <template-tag>`. Template Git tags version updates independently of your project's version.

## Initialization provenance

This skeleton was generated with Copier 9.19.1 from the untagged template commit
`67630a4d60c57ad89142cebd99b5c2e6da01f17a`, explicitly pinned with `--vcs-ref`.
See [initialization notes](docs/development/INITIALIZATION.md) for the copy command,
project values, and validation results. No PDE functionality has been added.
