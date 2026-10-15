#pragma once

#include "Types.h"
#include "Domain.h"
#include "Grid.h"
#include "Array.h"
#include "Equation.h"
#include "Tridiagonal.h"
#include "Scheme/CrankNicholson1D.h"

namespace PDE {
inline Scheme::CrankNicholson1D makeSolver1D(Grid1D const& grid, PDE1D const& pde) {
    return Scheme::CrankNicholson1D(grid, pde);
}
} // namespace PDE
