#include <pde_software_replication_core/PDE.h>
#include <gtest/gtest.h>
#include <array>
#include <cmath>
#include <limits>
#include <set>
#include <string>
#include <utility>

namespace {
using namespace PDE;
using Field = Func::F1D PDE1D::*;
const std::array<Field, 7> fields{&PDE1D::diffusion, &PDE1D::advection,
    &PDE1D::reaction, &PDE1D::source, &PDE1D::initial, &PDE1D::boundaryL, &PDE1D::boundaryR};
const std::array<char const*, 7> names{"diffusion", "advection", "reaction", "source",
                                      "initial", "boundaryL", "boundaryR"};
struct UserError { std::size_t role; std::string payload; };
PDE1D validPde() {
    PDE1D p;
    p.diffusion = [](double, double) { return 1.0; };
    p.advection = p.reaction = p.source = [](double, double) { return 0.0; };
    p.initial = p.boundaryL = p.boundaryR = [](double, double) { return 1.0; };
    return p;
}

TEST(Scheme, DefaultsReferencesCopiesAndIndependentResults) {
    Grid1D grid(Domain1D(0, 1, 0, 0.1), 5, 2);
    PDE1D pde;
    EXPECT_TRUE(pde.diffusion && pde.advection && pde.reaction && pde.source);
    EXPECT_FALSE(pde.initial || pde.boundaryL || pde.boundaryR);
    EXPECT_EQ(pde.diffusion(0, 0), 1.0);
    EXPECT_EQ(pde.advection(0, 0), 0.0);
    EXPECT_EQ(pde.reaction(0, 0), 0.0);
    EXPECT_EQ(pde.source(0, 0), 0.0);
    pde.initial = pde.boundaryL = pde.boundaryR = [](double, double) { return 1.0; };
    auto solver = makeSolver1D(grid, pde);
    EXPECT_EQ(solver.theta(), 0.5);
    auto first = solver.solve();
    auto repeated = solver.solve();
    EXPECT_TRUE(std::equal(first.begin(), first.end(), repeated.begin()));
    for (double v : first) EXPECT_NEAR(v, 1.0, 1e-12);
    pde.initial = pde.boundaryL = pde.boundaryR = [](double, double) { return 2.0; };
    auto second = solver.solve();
    for (double v : first) EXPECT_NEAR(v, 1.0, 1e-12);
    for (double v : second) EXPECT_NEAR(v, 2.0, 1e-12);
    solver.setTheta(0.75);
    auto copy = solver;
    auto moved = std::move(copy);
    EXPECT_EQ(moved.theta(), 0.75);
    // Existing schemes see grid assignment, rather than retaining a grid copy.
    grid = Grid1D(Domain1D(-1, 2, 0, 0.2), 7, 3);
    EXPECT_EQ(moved.solve().size(), 7u);
    auto fresh = moved.solve();
    for (double v : fresh) EXPECT_NEAR(v, 2.0, 1e-12);
    second.fill(100.0);
    for (double v : moved.solve()) EXPECT_NEAR(v, 2.0, 1e-12);
}

TEST(Scheme, ThetaValidationPreservesPreviousValue) {
    Grid1D grid(Domain1D(0, 1, 0, 1), 3, 1);
    PDE1D pde;
    auto solver = makeSolver1D(grid, pde); // no PDE validation at construction
    for (double good : {0.5, 0.75, 1.0, std::nextafter(0.5, 1.0)}) {
        solver.setTheta(good);
        EXPECT_EQ(solver.theta(), good);
    }
    const double before = solver.theta();
    for (double bad : {0.49, 1.01, std::numeric_limits<double>::quiet_NaN(),
                       std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()}) {
        EXPECT_THROW(solver.setTheta(bad), std::runtime_error);
        EXPECT_EQ(solver.theta(), before);
    }
}

TEST(Scheme, RejectsMissingCallbacksBeforeInvokingAny) {
    Grid1D grid(Domain1D(0, 1, 0, 1), 5, 2);
    PDE1D defaults;
    int called = 0;
    defaults.diffusion = [&called](double, double) { ++called; return 1.0; };
    EXPECT_THROW(makeSolver1D(grid, defaults).solve(), std::runtime_error);
    EXPECT_EQ(called, 0);
    for (std::size_t missing = 0; missing < fields.size(); ++missing) {
        SCOPED_TRACE(names[missing]);
        auto pde = validPde();
        for (auto field : fields) pde.*field = [&called](double, double) { ++called; return 1.0; };
        pde.*fields[missing] = {};
        auto solver = makeSolver1D(grid, pde);
        EXPECT_THROW(solver.solve(), std::runtime_error);
        EXPECT_EQ(called, 0);
    }
}

TEST(Scheme, RejectsAllUnsupportedBoundaryTypesOnBothSidesBeforeCallbacks) {
    Grid1D grid(Domain1D(0, 1, 0, 1), 3, 1);
    int called = 0;
    for (std::size_t side = 0; side < 2; ++side) {
        for (auto type : {BoundaryType::Neumann, BoundaryType::Gamma, static_cast<BoundaryType>(123)}) {
            auto pde = validPde();
            for (auto field : fields) pde.*field = [&called](double, double) { ++called; return 1.0; };
            pde.boundaryConfig.types[side] = type;
            auto solver = makeSolver1D(grid, pde);
            EXPECT_THROW(solver.solve(), std::runtime_error);
            EXPECT_EQ(called, 0);
        }
    }
}

TEST(Scheme, RejectsNonfiniteValuesAtEveryRequiredSampleInEveryRole) {
    Grid1D grid(Domain1D(0, 1, 0, 0.1), 5, 2);
    for (std::size_t role = 0; role < fields.size(); ++role) {
        for (std::size_t k = 0; k <= grid.nt(); ++k) {
            if (role == 4 && k != 0) continue;
            for (std::size_t i = 0; i < grid.nx(); ++i) {
                if ((role <= 4 && (i == 0 || i + 1 == grid.nx())) ||
                    (role == 5 && i != 0) || (role == 6 && i + 1 != grid.nx())) continue;
                for (double bad : {std::numeric_limits<double>::quiet_NaN(),
                        std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()}) {
                    SCOPED_TRACE(std::string(names[role]) + " k=" + std::to_string(k) + " i=" + std::to_string(i));
                    auto pde = validPde();
                    const auto original = pde.*fields[role];
                    pde.*fields[role] = [=](double t, double x) {
                        return t == grid.t(k) && x == grid.x(i) ? bad : original(t, x);
                    };
                    try {
                        makeSolver1D(grid, pde).solve();
                        FAIL() << "required invalid sample was not rejected";
                    } catch (std::runtime_error const& e) {
                        EXPECT_NE(std::string(e.what()).find(names[role]), std::string::npos);
                    }
                }
            }
        }
    }
    for (double bad : {0.0, -1.0}) {
        auto pde = validPde();
        pde.diffusion = [=](double t, double x) { return t == grid.t(1) && x == grid.x(2) ? bad : 1.0; };
        EXPECT_THROW(makeSolver1D(grid, pde).solve(), std::runtime_error);
    }
}

TEST(Scheme, PreservesEveryUserExceptionTypeAndPayload) {
    Grid1D grid(Domain1D(0, 1, 0, 0.1), 5, 2);
    for (std::size_t role = 0; role < fields.size(); ++role) {
        auto pde = validPde();
        pde.*fields[role] = [role](double, double) -> double { throw UserError{role, "unchanged payload"}; };
        try {
            makeSolver1D(grid, pde).solve();
            FAIL() << "callback should throw";
        } catch (UserError const& e) {
            EXPECT_EQ(e.role, role);
            EXPECT_EQ(e.payload, "unchanged payload");
        } catch (...) { FAIL() << "callback exception was wrapped"; }
    }
}

TEST(Scheme, SamplesExactlyThePermittedSetIncludingBackwardEulerSourceRule) {
    Grid1D grid(Domain1D(-1, 1, 0.2, 0.4), 5, 3);
    using Node = std::pair<std::size_t, std::size_t>;
    for (double theta : {0.5, 0.75, 1.0}) {
        std::array<std::set<Node>, 7> seen;
        auto pde = validPde();
        for (std::size_t role = 0; role < fields.size(); ++role) {
            pde.*fields[role] = [&, role](double t, double x) {
                std::size_t k = 0, i = 0;
                while (k <= grid.nt() && grid.t(k) != t) ++k;
                while (i < grid.nx() && grid.x(i) != x) ++i;
                if (k > grid.nt() || i >= grid.nx()) throw UserError{role, "off-grid sample"};
                const bool interior = i > 0 && i + 1 < grid.nx();
                const bool allowed = role < 3 ? interior : role == 3 ? interior && (theta < 1 || k > 0)
                    : role == 4 ? interior && k == 0 : role == 5 ? i == 0 : i + 1 == grid.nx();
                if (!allowed) throw UserError{role, "unused sample"};
                seen[role].emplace(k, i);
                return role == 0 || role >= 4 ? 1.0 : 0.0;
            };
        }
        auto solver = makeSolver1D(grid, pde);
        solver.setTheta(theta);
        EXPECT_NO_THROW(solver.solve());
        for (std::size_t role = 0; role < fields.size(); ++role) {
            std::set<Node> expected;
            for (std::size_t k = 0; k <= grid.nt(); ++k)
                for (std::size_t i = 0; i < grid.nx(); ++i) {
                    const bool interior = i > 0 && i + 1 < grid.nx();
                    if ((role < 3 && interior) ||
                        (role == 3 && interior && (theta < 1 || k > 0)) ||
                        (role == 4 && interior && k == 0) ||
                        (role == 5 && i == 0) || (role == 6 && i + 1 == grid.nx())) expected.emplace(k, i);
                }
            EXPECT_EQ(seen[role], expected) << names[role];
        }
    }
}

TEST(Scheme, OneInteriorRowWeightedCoefficientsAndCornerPrecedence) {
    Grid1D grid(Domain1D(0, 2, 0, 1), 3, 1);
    auto pde = validPde();
    pde.diffusion = [](double t, double) { return 1 + t; };
    pde.advection = [](double t, double) { return 2 * t; };
    pde.reaction = [](double t, double) { return -2 * t; };
    pde.source = [](double t, double) { return 2 + 4 * t; };
    pde.initial = [](double, double x) {
        if (x == 0 || x == 2) throw UserError{4, "initial endpoint"};
        return 2.0;
    };
    pde.boundaryL = [](double t, double) { return 1 + 2 * t; };
    pde.boundaryR = [](double t, double) { return 5 - t; };
    for (double theta : {0.5, 0.75, 1.0}) {
        auto solver = makeSolver1D(grid, pde);
        solver.setTheta(theta);
        const auto result = solver.solve();
        EXPECT_EQ(result[0], 3.0);
        EXPECT_EQ(result[2], 4.0);
        EXPECT_NEAR(result[1], (7 + 12 * theta) / (1 + 4 * theta), 1e-12);
    }
}

TEST(Scheme, HeatModeAmplificationAtLargeTimeStep) {
    const double pi = std::acos(-1.0);
    Grid1D grid(Domain1D(0, 1, 0, 1), 21, 1);
    auto pde = validPde();
    pde.initial = [pi](double, double x) { return std::sin(19 * pi * x); };
    pde.boundaryL = pde.boundaryR = [](double, double) { return 0.0; };
    const double lambda = -4 * std::pow(std::sin(19 * pi / 40), 2) / (grid.dx() * grid.dx());
    for (double theta : {0.5, 0.75, 1.0}) {
        auto solver = makeSolver1D(grid, pde);
        solver.setTheta(theta);
        const double g = (1 + (1 - theta) * lambda) / (1 - theta * lambda);
        EXPECT_LE(std::abs(g), 1.0);
        const auto result = solver.solve();
        EXPECT_EQ(result[0], 0.0);
        EXPECT_EQ(result[20], 0.0);
        for (std::size_t i = 0; i < grid.nx(); ++i)
            EXPECT_NEAR(result[i], g * std::sin(19 * pi * grid.x(i)), 2e-11);
    }
}

TEST(Scheme, GeneratedOverflowAndZeroImplicitPivotFailWithoutChangingConfigurations) {
    Grid1D grid(Domain1D(0, 1, 0, 1), 3, 1);
    auto pde = validPde();
    pde.diffusion = [](double, double) { return std::numeric_limits<double>::max(); };
    auto solver = makeSolver1D(grid, pde);
    EXPECT_THROW(solver.solve(), std::runtime_error);
    EXPECT_EQ(solver.theta(), 0.5);
    EXPECT_EQ(pde.diffusion(0, 0), std::numeric_limits<double>::max());
    grid = Grid1D(Domain1D(0, 2, 0, 1), 3, 1);
    pde = validPde();
    pde.reaction = [](double, double) { return 4.0; };
    pde.boundaryL = pde.boundaryR = [](double, double) { return 0.0; };
    EXPECT_THROW(solver.solve(), std::runtime_error);
    EXPECT_EQ(pde.initial(0, 1), 1.0);
    // Failure leaves no persistent factorization or partial state.
    pde = validPde();
    for (double value : solver.solve()) EXPECT_NEAR(value, 1.0, 1e-12);
}

TEST(Scheme, FiniteLargeEndpointMeansAvoidSpuriousOverflowAndSmallMeanUnderflow) {
    // Huge coefficient averages are finite, and the tiny time step keeps the
    // generated rows finite. Ordinary p+q would overflow before division.
    Grid1D grid(Domain1D(0, 2, 0, 1e-308), 3, 1);
    auto pde = validPde();
    const double half = std::numeric_limits<double>::max() / 2;
    pde.diffusion = [half](double t, double) { return t == 0 ? half * 1.25 : half * 1.5; };
    pde.advection = [half](double, double) { return -half * 1.25; };
    pde.reaction = [half](double, double) { return -half * 1.25; };
    EXPECT_NO_THROW(makeSolver1D(grid, pde).solve());
    // This checks the small-value branch: halving both operands would lose
    // denorm_min, while (p+q)/2 retains it.
    EXPECT_EQ(PDE::detail::mean(std::numeric_limits<double>::denorm_min(),
                               std::numeric_limits<double>::denorm_min(), "test mean"),
              std::numeric_limits<double>::denorm_min());
}
} // namespace
