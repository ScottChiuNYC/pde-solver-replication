#pragma once

#include "../Grid.h"
#include "../Equation.h"
#include "../Tridiagonal.h"

namespace PDE { namespace Scheme {
class CrankNicholson1D {
public:
    CrankNicholson1D(Grid1D const& grid, PDE1D const& pde) : m_grid(grid), m_pde(pde) {}
    CrankNicholson1D(CrankNicholson1D const&) = default;
    CrankNicholson1D(CrankNicholson1D&&) = default;
    CrankNicholson1D& operator=(CrankNicholson1D const&) = delete;
    CrankNicholson1D& operator=(CrankNicholson1D&&) = delete;
    void setTheta(double theta) {
        if (!std::isfinite(theta) || theta < 0.5 || theta > 1.0)
            throw std::runtime_error("scheme theta must be finite and in [0.5,1]");
        m_theta = theta;
    }
    double theta() const { return m_theta; }
    Array1D solve() const {
        // Configuration errors are rejected before solve scratch allocation or
        // callback invocation. User exceptions are deliberately never caught.
        validate(m_pde);
        const std::size_t nx = m_grid.nx(), m = nx - 2;
        const double dt = m_grid.dt(), dx = m_grid.dx();
        const double dtOverDx = detail::div(dt, dx, "PDE mesh dt/dx");
        const double r = detail::div(dtOverDx, dx, "PDE mesh dt/dx/dx");
        const double s = detail::div(dtOverDx, 2.0, "PDE mesh dt/dx/2");
        const double oldWeight = detail::sub(1.0, m_theta, "PDE theta weight");
        Array1D old(nx), next(nx), lower(m - 1), diagonal(m), upper(m - 1), rhs(m);
        const auto& domain = m_grid.domain();
        old[0] = sample(m_pde.boundaryL, "boundaryL", domain.t0, domain.xL, 0, 0);
        old[nx - 1] = sample(m_pde.boundaryR, "boundaryR", domain.t0, domain.xR, 0, nx - 1);
        for (std::size_t i = 1; i + 1 < nx; ++i)
            old[i] = sample(m_pde.initial, "initial", domain.t0, m_grid.x(i), 0, i);

        for (std::size_t n = 0; n < m_grid.nt(); ++n) {
            const double tOld = m_grid.t(n), tNew = m_grid.t(n + 1);
            next[0] = sample(m_pde.boundaryL, "boundaryL", tNew, domain.xL, n + 1, 0);
            next[nx - 1] = sample(m_pde.boundaryR, "boundaryR", tNew, domain.xR, n + 1, nx - 1);
            for (std::size_t j = 0; j < m; ++j) {
                const std::size_t i = j + 1;
                const double x = m_grid.x(i);
                const std::string row = "PDE step " + std::to_string(n) + " row " + std::to_string(i);
                const double aOld = sample(m_pde.diffusion, "diffusion", tOld, x, n, i, true);
                const double aNew = sample(m_pde.diffusion, "diffusion", tNew, x, n + 1, i, true);
                const double bOld = sample(m_pde.advection, "advection", tOld, x, n, i);
                const double bNew = sample(m_pde.advection, "advection", tNew, x, n + 1, i);
                const double cOld = sample(m_pde.reaction, "reaction", tOld, x, n, i);
                const double cNew = sample(m_pde.reaction, "reaction", tNew, x, n + 1, i);
                const double a = detail::mean(aOld, aNew, row + " diffusion mean");
                const double b = detail::mean(bOld, bNew, row + " advection mean");
                const double c = detail::mean(cOld, cNew, row + " reaction mean");
                const double ar = detail::mul(a, r, row + " ar");
                const double bs = detail::mul(b, s, row + " bs");
                const double cdt = detail::mul(c, dt, row + " cdt");
                const double qL = detail::sub(ar, bs, row + " qL");
                const double minusTwoAr = detail::mul(-2.0, ar, row + " -2ar");
                const double qD = detail::add(minusTwoAr, cdt, row + " qD");
                const double qR = detail::add(ar, bs, row + " qR");
                const double aL = detail::mul(-m_theta, qL, row + " implicit left");
                const double thetaQD = detail::mul(m_theta, qD, row + " theta qD");
                diagonal[j] = detail::sub(1.0, thetaQD, row + " implicit diagonal");
                const double aR = detail::mul(-m_theta, qR, row + " implicit right");

                const double leftWeight = detail::mul(oldWeight, qL, row + " explicit left weight");
                const double left = detail::mul(leftWeight, old[i - 1], row + " explicit left product");
                const double middleWeight = detail::add(1.0,
                    detail::mul(oldWeight, qD, row + " explicit diagonal weight"), row);
                const double middle = detail::mul(middleWeight, old[i], row + " explicit diagonal product");
                const double rightWeight = detail::mul(oldWeight, qR, row + " explicit right weight");
                const double right = detail::mul(rightWeight, old[i + 1], row + " explicit right product");
                double sourceOld = 0.0;
                if (m_theta < 1.0) {
                    const double value = sample(m_pde.source, "source", tOld, x, n, i);
                    sourceOld = detail::mul(oldWeight, value, row + " old source product");
                }
                const double newValue = sample(m_pde.source, "source", tNew, x, n + 1, i);
                const double sourceNew = detail::mul(m_theta, newValue, row + " new source product");
                const double sourceSum = detail::add(sourceOld, sourceNew, row + " source sum");
                const double source = detail::mul(dt, sourceSum, row + " source dt product");
                double value = detail::add(left, middle, row + " RHS left+diagonal");
                value = detail::add(value, right, row + " RHS +right");
                value = detail::add(value, source, row + " RHS +source");
                if (j == 0) {
                    const double correction = detail::mul(aL, next[0], row + " left boundary product");
                    value = detail::sub(value, correction, row + " left boundary correction");
                } else lower[j - 1] = aL;
                if (j + 1 == m) {
                    const double correction = detail::mul(aR, next[nx - 1], row + " right boundary product");
                    value = detail::sub(value, correction, row + " right boundary correction");
                } else upper[j] = aR;
                rhs[j] = value;
            }
            TridiagonalSolver::solve(lower, diagonal, upper, rhs);
            for (std::size_t j = 0; j < m; ++j) next[j + 1] = rhs[j];
            old.swap(next);
        }
        return old;
    }
private:
    static void validate(PDE1D const& pde) {
        for (std::size_t side = 0; side < 2; ++side)
            if (pde.boundaryConfig.types[side] != BoundaryType::Dirichlet)
                throw std::runtime_error("PDE unsupported boundary type on side " + std::to_string(side));
        if (!pde.diffusion) throw std::runtime_error("PDE missing diffusion callback");
        if (!pde.advection) throw std::runtime_error("PDE missing advection callback");
        if (!pde.reaction) throw std::runtime_error("PDE missing reaction callback");
        if (!pde.source) throw std::runtime_error("PDE missing source callback");
        if (!pde.initial) throw std::runtime_error("PDE missing initial callback");
        if (!pde.boundaryL) throw std::runtime_error("PDE missing boundaryL callback");
        if (!pde.boundaryR) throw std::runtime_error("PDE missing boundaryR callback");
    }
    static double sample(Func::F1D const& callback, char const* role,
                         double t, double x, std::size_t k, std::size_t i,
                         bool positive = false) {
        const double value = callback(t, x);
        if (!std::isfinite(value) || (positive && !(value > 0.0)))
            throw std::runtime_error(std::string("PDE callback ") + role +
                " at time-index " + std::to_string(k) + " spatial-index " + std::to_string(i) +
                " (t=" + std::to_string(t) + ", x=" + std::to_string(x) +
                "): expected finite" + (positive ? " positive value" : " value"));
        return value;
    }
    Grid1D const& m_grid;
    PDE1D const& m_pde;
    double m_theta = 0.5;
};
}} // namespace PDE::Scheme
