# PDE AI Implementation Plan

## Overview

Header-only C++17 library for solving 1D parabolic PDEs using Crank-Nicholson finite difference method. 

Scope: 1D only, No 2D code. 

## PDE Form

$$
u_t = a(t, x)u_{xx} + b(t, x)u_x + c(t, x)u + f(t, x), 
$$
where $a$ is diffusion, $b$ is advection, $c$ is reation, $f$ is source. 

## File Structure

* include/pde: Header-only interface library
    * PDE.h: Main include (imports all components)
    * Types.h: BoundaryType enum, BoundaryConfig1D, Func::F1D
    * Domain.h: Domain1D: spatial [xL, xR] x temporal [t0, t1]
    * Grid.h: Grid1D: uniform discretization (nx, nt, dx, dt)
    * Array.h: Array1D; std::vector wrapper
    * Equation.h: PDE1D: coefficients + ICs + BCs
    * Tridiagonal.h: TridiagonalSolver: Thomas algorithm
    * Schemes/
        * CrankNicholson1D.h: Theta-scheme solver (theta=0.5)

## Core Types (Types.h)

```cpp
enum class BoundaryType { Dirichlet, Neumann, Gamma };

struct BoundaryConfig1D {
    std::array<BoundaryType, 2> types;  // [Left=0, Right=1]
    static constexpr std::size_t Left = 0, Right = 1;
};

namespace Func { using F1D = std::function<double(double, double)>; } // f(t, x)
```

## Domain & Grid (Domain.h, Grid.h)

```cpp
struct Domain1D{
    double xL, xR, t0, t1;
    // validation: xR>xL, t1>t0
};

class Grid1D{
    Domain1D m_domain;
    std::size_t m_nx, m_nt;
    double m_dx = lengthX/(nx-1), m_dt = lengthT/nt;
    // Methods: nx(), nt(), dx(), dt(), x(i), t(k), domain()
};
```

## PDE Definition (Equation.h)

```cpp
struct PDE1D {
    Func::F1D diffusion, advection, source, reaction; // Coefficients
    Func::F1D initial; // u(t0, x)
    Func::F1D boundaryL, boundaryR; // BC values
    BoundaryConfig1D boundaryConfig; // BC types
    // Defaults: heat equation (diffusion=1, others=0), Dirichlet BCs
};
```

## Array Container (Array.h)

```cpp
class Array1D {
    std::vector<double> m_data;
    std::size_t m_size;
    // Element access: operator[], at(), size(), fill(), swap()
    // Iterators: begin(), end()
    // Raw access: data()
};
```

## Tridiagonal Solver (Tridiagonal.h)

Thomas algorithm for $Ax = b$ where $A$ is tridiagonal:
    * Input: sub-diagonal `a`, diagonal `b`, super-diagonal `c`, RHS `d`
    * Output: solution in-place in `d`
    * Algorithm: forward sweep (eliminate sub-diag) -> back substitution

## Crank-Nicholson Solver (Schemes/CrankNicholson1D.h)

### Discretization
 
Theta-scheme with theta=0.5: 

$$
\frac{U^{n+1} - U^{n}}{dt} = \theta{\mathcal L}[U^{n+1}] + (1-\theta) {\mathcal L}[U^{n}]
$$

### Spatial Operator L

$$
L[U^{n}] = a U_{xx} + b U_x + c U + f\\
U_{xx} = \frac{U^{i-1} - 2U^{i} + U^{i+1}}{(dx)^2}\\
U_{x} = \frac{U^{i+1} - U^{i-1}}{2dx}
$$

### Matrix Assembly

For interior points $i=1, \ldots, nx-2$, build tridiagonal system:

* Coefficients: average at times $t_n$ and $t_{n+1}$
* Let $r = dt/(dx)^2, s = dt/(2dx)$
* Implicit side ($\theta=0.5$): 
    * alpha_impl = -theta (ar - bs) [U_{i-1} coeff] 
    * beta_impl = 1 + theta (2ar - cdt) [U_{i-1} coeff] [U_{i} coeff] 
    * gamma_impl = -theta (ar + bs) [U_{i+1} coeff] 
* Explicit side ($1-\theta=0.5$): similar with $(1-\theta)$ prefix
* RHS = alpha_expl * U^{n[i-1]} + beta_expl * U^{n[i]} + gamma_expl * U^{n[i-1]} + dt*sources

### Boundary Conditions

#### Stage 1: Dirichlet Only

* Throw `std::runtime_error` for Neumann/Gamma
* Modify RHS for boundary contributions before solving
* Set `u[0] = boundaryL, u[nx-1] = boundaryR` after solving

#### Future (Neumann/Gamma If Implemented): 

* Neumann: $u_x = g$ -> `u[0] = u[1] - dx*g`
* Gamma: $u_{xx} = g$ -> `u[0] = 2u[1] - u[2] - dx^2*g`

### Algorithm

```cpp
Array1D solve() {
    // 1. Initialize u with IC
    // 2. For each timestep k=0..nt-1:
    //    a. Build tridiagonal system for interior points
    //    b. Call applyBoundaryConditions():
    //       - Check BC types (must be Dirichlet)
    //       - Modify RHS for boundary terms
    //       - Solve tridiagonal system
    //       - Set boundary values
    // 3. Return final u
}
```

### Public Interface

```cpp
class CrankNicholson1D {
    CrankNicholson1D(Grid1D const&, PDE1D const&);
    void setTheta(double);  // Default theta = 0.5
    Array1D solve() const;
};
```

## Main Include (PDE.h)

Imports all headers and provides factory: 

```cpp
namespace PDE {
    inline Scheme::CrankNicholson1D makeSolver1D(Grid1D const& grid, PDE1D const& pde);
}
```

## Example Usage Pattern

```cpp
#include "pde/PDE.h"

// Define domain and grid
PDE::Domain1D domain(xL, xR, t0, t1);
PDE::Grid1D grid(domain, nx, nt);

// Define PDE
PDE::PDE1D pde;
pde.diffusion = [](double t, double x) { return 1.0; };
pde.initial = [](double t, double x) { return initial_condition(x); };
pde.boundaryL = [](double t, double x) { return bc_left(t); };
pde.boundaryR = [](double t, double x) { return bc_right(t); };
pde.boundaryConfig.types[PDE::BoundaryConfig1D::Left] = PDE::BoundaryType::Dirichlet;

// Solve
auto solver = PDE::makeSolver1D(grid, pde);
PDE::Array1D solution = solver.solve();
```

## Key Design Decisions

* C++17: std::function, structured bindings, if constexpr ready
* no templates
* Row-major only: Array1D uses std::vector directorly
* Functional API: Lambdas for coefficient/BC specification
* no exception specs: Let std::runtime_error propagate
* Const-correctness: solve() is const, references to config objects
* Static factor: makeSolver1D for clean instantiation

## Validation Approach

* Example: 1D heat equation with manufactured solution
* Compare computed vs analytical at final time
* Report max error and L2 error

## Dependencies

* Standard library only: `<vector>`, `<array>`, `<functional>`, `<stdexcept>`, `<cmath>`, `<algorithm>`
* No external libraries

## Critical Implementation Notes

* Grid indexing: nx points include boundaries; solve for nx-2 interior points
* Time-averaging: Coefficients averaged between t_n and t_{n+1} for accuracy
* Boundary RHS modification: Much happen before solving, not after
* Thomas algorithm stability: Assumes diagonal dominance (always true for Crank-Nicholson with theta=0.5)
* Include order: Types.h first, then Domain/Grid/Array/Equation, then Tridiagonal/Schemes