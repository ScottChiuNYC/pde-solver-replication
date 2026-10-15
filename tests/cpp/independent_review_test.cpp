#include <pde_software_replication_core/PDE.h>
#include <gtest/gtest.h>
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>

namespace {
using namespace PDE;

TEST(IndependentNumerics, FullDenseRationalReferenceWithAsymmetricRowsAndBoundaries) {
    // Regenerate these constants with tests/python/generate_dense_reference.py.
    // The oracle uses exact rational arithmetic and a full 6x6 matrix with
    // boundary identity rows, solved by dense Gaussian elimination with pivoting.
    // It shares neither Thomas elimination nor boundary RHS corrections.
    const std::array<double, 4> theta{0.5, 0.625, 0.75, 1.0};
    const double expected[4][6] = {
        {3.64, 2.8455828417129885, 2.0313272147291435, 1.3550301696038165, 0.97624302443823352, 1.112},
        {3.64, 2.8374495095777044, 2.0621030507088696, 1.4346041966999064, 1.045119515732073, 1.112},
        {3.64, 2.8246710591948472, 2.0852913487146312, 1.5148362257270682, 1.1191718667515109, 1.112},
        {3.64, 2.7875408885093345, 2.125270431622674, 1.6608161934735957, 1.28319217082757, 1.112}
    };
    Grid1D grid(Domain1D(-1.0, 2.0, 0.2, 0.8), 6, 3);
    PDE1D pde;
    pde.diffusion = [](double t, double x) { return 1 + t*t + x*x/10; };
    pde.advection = [](double t, double x) { return 0.1 + t*x/2; };
    pde.reaction = [](double t, double x) { return 0.2 - t/4 + x/20; };
    pde.source = [](double t, double x) { return 2*t*t - x + t*x*x/7; };
    pde.initial = [](double, double x) { return 1 + x + x*x*x/8; };
    pde.boundaryL = [](double t, double) { return 3 + t*t; };
    pde.boundaryR = [](double t, double) { return -1 + 2*t + t*t*t; };
    for (std::size_t j = 0; j < theta.size(); ++j) {
        SCOPED_TRACE(theta[j]);
        auto solver = makeSolver1D(grid, pde);
        solver.setTheta(theta[j]);
        const auto result = solver.solve();
        ASSERT_EQ(result.size(), 6u);
        for (std::size_t i = 0; i < result.size(); ++i)
            EXPECT_NEAR(result[i], expected[j][i], 3e-14) << "node " << i;
        EXPECT_EQ(result[0], pde.boundaryL(grid.domain().t1, -1));
        EXPECT_EQ(result[5], pde.boundaryR(grid.domain().t1, 2));
    }
}

TEST(IndependentNumerics, MultipleHeatModesOverMultipleSteps) {
    const double pi = std::acos(-1.0);
    Grid1D grid(Domain1D(0, 1, 0, 0.3), 21, 7);
    const std::array<int, 3> modes{1, 3, 8};
    const std::array<double, 3> amplitudes{1, 0.3, -0.2};
    PDE1D pde;
    pde.diffusion = [](double, double) { return 1.0; };
    pde.advection = pde.reaction = pde.source = [](double, double) { return 0.0; };
    pde.initial = [&](double, double x) {
        double value = 0;
        for (std::size_t j = 0; j < modes.size(); ++j)
            value += amplitudes[j] * std::sin(modes[j]*pi*x);
        return value;
    };
    pde.boundaryL = pde.boundaryR = [](double, double) { return 0.0; };
    for (double theta : {0.5, std::nextafter(0.5, 1.0), 0.625, 0.75, 1.0}) {
        SCOPED_TRACE(theta);
        auto solver = makeSolver1D(grid, pde);
        solver.setTheta(theta);
        const auto result = solver.solve();
        for (std::size_t i = 1; i + 1 < grid.nx(); ++i) {
            double exactDiscrete = 0;
            for (std::size_t j = 0; j < modes.size(); ++j) {
                const double eigenvalue = -4*std::pow(std::sin(modes[j]*pi/40), 2)/(grid.dx()*grid.dx());
                const double g = (1 + (1-theta)*grid.dt()*eigenvalue)/(1-theta*grid.dt()*eigenvalue);
                exactDiscrete += amplitudes[j]*std::pow(g, double(grid.nt()))*std::sin(modes[j]*pi*grid.x(i));
            }
            EXPECT_NEAR(result[i], exactDiscrete, 3e-14);
        }
        EXPECT_EQ(result[0], 0.0);
        EXPECT_EQ(result[grid.nx()-1], 0.0);
    }
}

TEST(IndependentNumerics, AdmittedReactionMayGiveANegativeImplicitDiagonal) {
    // A = -3, B = 5: non-dominance/positivity is not a rejection policy.
    Grid1D grid(Domain1D(0, 2, 0, 1), 3, 1);
    PDE1D pde;
    pde.diffusion = [](double, double) { return 1.0; };
    pde.advection = pde.source = [](double, double) { return 0.0; };
    pde.reaction = [](double, double) { return 10.0; };
    pde.initial = [](double, double) { return 1.0; };
    pde.boundaryL = pde.boundaryR = [](double, double) { return 0.0; };
    const auto result = makeSolver1D(grid, pde).solve();
    EXPECT_NEAR(result[1], -5.0/3.0, 1e-15);
}

TEST(IndependentNumerics, ExtremeCoefficientMeansHaveObservableCorrectValues) {
    const double large = std::ldexp(1.0, 1023);
    const double smallTime = std::ldexp(1.0, -1022);
    Grid1D grid(Domain1D(0, 2, 0, smallTime), 3, 1);
    PDE1D pde;
    pde.diffusion = [large](double, double) { return large; };
    pde.advection = pde.reaction = [large](double t, double) { return t == 0 ? large : -large; };
    pde.source = [](double, double) { return 0.0; };
    pde.initial = [](double, double) { return 1.0; };
    pde.boundaryL = pde.boundaryR = [](double, double) { return 0.0; };
    EXPECT_EQ(makeSolver1D(grid, pde).solve()[1], -1.0/3.0); // ar=2, A=3, B=-1

    // Halving subnormal endpoints before adding would lose diffusion entirely.
    grid = Grid1D(Domain1D(0, 2, 0, large), 3, 1);
    pde.diffusion = [](double, double) { return std::numeric_limits<double>::denorm_min(); };
    pde.advection = pde.reaction = [](double, double) { return 0.0; };
    pde.initial = [](double, double) { return std::ldexp(1.0, 52); };
    EXPECT_EQ(makeSolver1D(grid, pde).solve()[1], std::ldexp(1.0, 52)-4);
}

TEST(IndependentThomas, AsymmetricSignedRowsAndUniformScaling) {
    for (int exponent : {-500, 0, 500}) {
        const double scale = std::ldexp(1.0, exponent);
        Array1D lower(3), diagonal(4), upper(3), rhs(4);
        const double l[]{-0.3, 2, -1}, d[]{3, -4, 5, 2}, u[]{1, -2, 0.75};
        const double r[]{1, 6.7, 0.75, 5.5}, expected[]{1, -2, 0.5, 3};
        for (std::size_t j = 0; j < 4; ++j) {
            diagonal[j] = scale*d[j]; rhs[j] = scale*r[j];
            if (j < 3) { lower[j] = scale*l[j]; upper[j] = scale*u[j]; }
        }
        const auto savedLower = lower, savedDiagonal = diagonal, savedUpper = upper;
        TridiagonalSolver::solve(lower, diagonal, upper, rhs);
        for (std::size_t j = 0; j < 4; ++j) EXPECT_NEAR(rhs[j], expected[j], 2e-15);
        EXPECT_TRUE(std::equal(lower.begin(), lower.end(), savedLower.begin()));
        EXPECT_TRUE(std::equal(diagonal.begin(), diagonal.end(), savedDiagonal.begin()));
        EXPECT_TRUE(std::equal(upper.begin(), upper.end(), savedUpper.begin()));
    }
}

TEST(IndependentThomas, RelativeThresholdAlsoAppliesToFirstPivot) {
    Array1D lower(1, 0), diagonal(2, 1), upper(1, 1), rhs(2, 1);
    diagonal[0] = rhs[0] = std::ldexp(1.0, -60);
    const auto saved = rhs;
    try {
        TridiagonalSolver::solve(lower, diagonal, upper, rhs);
        FAIL() << "first pivot must be rejected";
    } catch (std::runtime_error const& e) {
        EXPECT_NE(std::string(e.what()).find("pivot index 0"), std::string::npos);
    }
    EXPECT_TRUE(std::equal(rhs.begin(), rhs.end(), saved.begin()));
}

TEST(IndependentNumerics, LateCallbackFailurePropagatesAndAllowsFreshSolve) {
    Grid1D grid(Domain1D(0, 1, 0, 0.1), 7, 3);
    PDE1D pde;
    pde.initial = pde.boundaryL = pde.boundaryR = [](double, double) { return 1.0; };
    bool fail = true;
    struct LateFailure { int payload; };
    pde.source = [&](double t, double x) {
        if (fail && t == grid.t(grid.nt()) && x == grid.x(grid.nx()-2))
            throw LateFailure{712};
        return 0.0;
    };
    const auto solver = makeSolver1D(grid, pde);
    try {
        solver.solve();
        FAIL() << "last row at last time must propagate the callback exception";
    } catch (LateFailure const& e) { EXPECT_EQ(e.payload, 712); }
    fail = false;
    const auto result = solver.solve();
    for (double value : result) EXPECT_NEAR(value, 1.0, 2e-15);
}
} // namespace
