#include <pde_software_replication_core/Tridiagonal.h>
#include <gtest/gtest.h>
#include <cstring>
#include <initializer_list>
#include <limits>

namespace {
using namespace PDE;
Array1D array(std::initializer_list<double> values, double scale = 1.0) {
    Array1D a(values.size());
    std::size_t i = 0;
    for (double v : values) a[i++] = scale * v;
    return a;
}
bool sameBits(Array1D const& a, Array1D const& b) {
    return a.size() == b.size() &&
        (a.size() == 0 || std::memcmp(a.data(), b.data(), a.size() * sizeof(double)) == 0);
}
void failsUnchanged(Array1D const& l, Array1D const& d, Array1D const& u, Array1D& r) {
    const auto lc = l, dc = d, uc = u, rc = r;
    EXPECT_THROW(TridiagonalSolver::solve(l, d, u, r), std::runtime_error);
    EXPECT_TRUE(sameBits(l, lc));
    EXPECT_TRUE(sameBits(d, dc));
    EXPECT_TRUE(sameBits(u, uc));
    EXPECT_TRUE(sameBits(r, rc));
}

TEST(Thomas, ScalarAndThreeRowsAtAllUniformScales) {
    for (int exponent : {0, -500, 500}) {
        SCOPED_TRACE(exponent);
        const double scale = std::ldexp(1.0, exponent);
        const Array1D empty;
        const auto scalar = array({2}, scale);
        auto scalarRhs = array({6}, scale);
        const auto savedScalar = scalar;
        TridiagonalSolver::solve(empty, scalar, empty, scalarRhs);
        EXPECT_NEAR(scalarRhs[0], 3.0, 1e-12);
        EXPECT_TRUE(sameBits(scalar, savedScalar));
        const auto off = array({1, 1}, scale), diagonal = array({4, 4, 4}, scale);
        const auto offCopy = off, diagCopy = diagonal;
        auto rhs = array({6, 12, 14}, scale);
        TridiagonalSolver::solve(off, diagonal, off, rhs); // coefficient aliasing is legal
        for (std::size_t i = 0; i < 3; ++i) EXPECT_NEAR(rhs[i], double(i + 1), 1e-12);
        EXPECT_TRUE(sameBits(off, offCopy));
        EXPECT_TRUE(sameBits(diagonal, diagCopy));
    }
}

TEST(Thomas, RejectsEveryShapeAndRhsAliasTransactionally) {
    auto l = array({1, 1}), d = array({4, 4, 4}), u = l, rhs = array({6, 12, 14});
    Array1D empty;
    failsUnchanged(empty, empty, empty, empty);
    auto wrong = array({1});
    failsUnchanged(wrong, d, u, rhs);
    failsUnchanged(l, d, wrong, rhs);
    failsUnchanged(l, wrong, u, rhs);
    failsUnchanged(l, d, u, wrong);
    failsUnchanged(l, d, u, l);
    failsUnchanged(l, d, u, d);
    failsUnchanged(l, d, u, u);
}

TEST(Thomas, NonfiniteInputsInEveryRoleRemainBitwiseUnchanged) {
    for (int role = 0; role < 4; ++role) {
        for (double bad : {std::numeric_limits<double>::quiet_NaN(),
                           std::numeric_limits<double>::infinity(),
                           -std::numeric_limits<double>::infinity()}) {
            SCOPED_TRACE(role);
            auto l = array({1, 1}), d = array({4, 4, 4}), u = l, rhs = array({6, 12, 14});
            Array1D* inputs[] = {&l, &d, &u, &rhs};
            (*inputs[role])[inputs[role]->size() - 1] = bad;
            failsUnchanged(l, d, u, rhs);
        }
    }
}

TEST(Thomas, RejectsZeroRowsAndZeroPivotsWithoutPivoting) {
    auto rhs = array({1, 2});
    failsUnchanged(array({0}), array({1, 0}), array({0}), rhs); // zero row
    failsUnchanged(array({1}), array({0, 1}), array({1}), rhs); // nonsingular; pivot required
    failsUnchanged(array({1}), array({1, 1}), array({1}), rhs); // reduced pivot zero
}

TEST(Thomas, OriginalRowRelativeThresholdAndEqualityAtAllScales) {
    const double tau = 64.0 * std::numeric_limits<double>::epsilon();
    for (int exponent : {0, -500, 500}) {
        const double scale = std::ldexp(1.0, exponent);
        for (int deltaExponent : {-48, -46, -44}) {
            SCOPED_TRACE(exponent);
            SCOPED_TRACE(deltaExponent);
            const double delta = std::ldexp(1.0, deltaExponent);
            const auto off = array({1}, scale), d = array({1, 1 + delta}, scale);
            auto rhs = array({2, 2 + delta}, scale);
            if (deltaExponent == -44) {
                TridiagonalSolver::solve(off, d, off, rhs);
                EXPECT_NEAR(rhs[0], 1.0, 1e-12);
                EXPECT_NEAR(rhs[1], 1.0, 1e-12);
            } else failsUnchanged(off, d, off, rhs);
        }
        const auto l = array({1}, scale), d = array({1, 1}, scale), u = array({1 - tau}, scale);
        auto rhs = array({2 - tau, 2}, scale);
        failsUnchanged(l, d, u, rhs);
    }
}

TEST(Thomas, EveryGeneratedArithmeticFailureIsTransactional) {
    const double max = std::numeric_limits<double>::max();
    const double tiny = std::numeric_limits<double>::denorm_min();
    Array1D empty;
    auto rhs = array({max});
    failsUnchanged(empty, array({tiny}), empty, rhs); // final division overflow
    rhs = array({1, 1});
    failsUnchanged(array({max}), array({tiny, max}), array({0}), rhs); // multiplier overflow
    failsUnchanged(array({max}), array({1, max}), array({2}), rhs); // pivot product overflow before cancellation
    rhs = array({max, 0});
    failsUnchanged(array({2}), array({1, 1}), array({0}), rhs); // reduced RHS product
    rhs = array({-max, max});
    failsUnchanged(array({0}), array({1, 1}), array({2}), rhs); // substitution product
    rhs = array({max, -max});
    failsUnchanged(array({1}), array({1, 1}), array({0}), rhs); // reduced RHS subtraction
    failsUnchanged(array({0}), array({1, 1}), array({1}), rhs); // substitution subtraction
    rhs = array({1, 1});
    failsUnchanged(array({-max}), array({1, max}), array({1}), rhs); // pivot subtraction
}
} // namespace
