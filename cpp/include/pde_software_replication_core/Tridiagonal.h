#pragma once

#include "Array.h"
#include "detail/Checked.h"

namespace PDE {
class TridiagonalSolver {
public:
    static void solve(Array1D const& lower, Array1D const& diagonal,
                      Array1D const& upper, Array1D& rhs) {
        if (&rhs == &lower || &rhs == &diagonal || &rhs == &upper)
            throw std::runtime_error("tridiagonal RHS aliases a coefficient array");
        const std::size_t n = diagonal.size();
        if (n == 0 || rhs.size() != n || lower.size() != n - 1 || upper.size() != n - 1)
            throw std::runtime_error("tridiagonal shape: expected n,n,n-1,n-1 with n>=1");
        for (std::size_t j = 0; j < n; ++j) {
            const std::string row = "tridiagonal input row " + std::to_string(j);
            detail::finite(diagonal[j], row + " diagonal");
            detail::finite(rhs[j], row + " RHS");
            if (j > 0) detail::finite(lower[j - 1], row + " lower");
            if (j + 1 < n) detail::finite(upper[j], row + " upper");
        }

        // Never eliminate in caller storage: even late failures preserve all
        // inputs. Original coefficients also supply each pivot's row scale.
        Array1D p(n), y(n), x(n);
        p[0] = diagonal[0];
        checkPivot(p[0], rowScale(lower, diagonal, upper, 0), 0);
        y[0] = rhs[0];
        for (std::size_t j = 1; j < n; ++j) {
            const std::string row = "tridiagonal elimination row " + std::to_string(j);
            const double w = detail::div(lower[j - 1], p[j - 1], row + " multiplier");
            const double wp = detail::mul(w, upper[j - 1], row + " pivot product");
            p[j] = detail::sub(diagonal[j], wp, row + " pivot");
            checkPivot(p[j], rowScale(lower, diagonal, upper, j), j);
            const double wy = detail::mul(w, y[j - 1], row + " RHS product");
            y[j] = detail::sub(rhs[j], wy, row + " reduced RHS");
        }
        x[n - 1] = detail::div(y[n - 1], p[n - 1],
                              "tridiagonal substitution row " + std::to_string(n - 1));
        for (std::size_t j = n - 1; j > 0; --j) {
            const std::string row = "tridiagonal substitution row " + std::to_string(j - 1);
            const double product = detail::mul(upper[j - 1], x[j], row + " product");
            const double numerator = detail::sub(y[j - 1], product, row + " numerator");
            x[j - 1] = detail::div(numerator, p[j - 1], row + " division");
        }
        rhs.swap(x);
    }
private:
    static double rowScale(Array1D const& lower, Array1D const& diagonal,
                           Array1D const& upper, std::size_t j) {
        double scale = std::abs(diagonal[j]);
        if (j > 0) scale = std::max(scale, std::abs(lower[j - 1]));
        if (j + 1 < diagonal.size()) scale = std::max(scale, std::abs(upper[j]));
        return scale;
    }
    static void checkPivot(double pivot, double scale, std::size_t j) {
        const std::string context = "tridiagonal pivot index " + std::to_string(j);
        detail::finite(pivot, context);
        if (scale == 0.0 || pivot == 0.0)
            throw std::runtime_error(context + ": zero row or pivot");
        const double magnitude = std::abs(pivot);
        const double tau = 64.0 * std::numeric_limits<double>::epsilon();
        if (magnitude < scale && !(detail::div(magnitude, scale, context) > tau))
            throw std::runtime_error(context + ": unusable relative pivot");
    }
};
} // namespace PDE
