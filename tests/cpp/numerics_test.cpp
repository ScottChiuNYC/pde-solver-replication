#include <pde_software_replication_core/PDE.h>
#include <gtest/gtest.h>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {
using namespace PDE;
const double pi = std::acos(-1.0);
struct Errors { double inf, l2; };

Errors measure(char const* fixture, Grid1D const& grid, PDE1D const& pde,
               double theta, Func::F1D const& exact) {
    auto solver = makeSolver1D(grid, pde);
    solver.setTheta(theta);
    const auto result = solver.solve();
    double largest = 0.0, sum = 0.0;
    for (std::size_t i = 0; i < grid.nx(); ++i) {
        const double error = result[i] - exact(grid.domain().t1, grid.x(i));
        largest = std::max(largest, std::abs(error));
        sum += error * error;
    }
    Errors errors{largest, std::sqrt(grid.dx() * sum)};
    EXPECT_TRUE(std::isfinite(errors.inf));
    EXPECT_TRUE(std::isfinite(errors.l2));
    const auto& d = grid.domain();
    std::cout << std::setprecision(17) << "ACCEPTANCE {\"fixture\":\"" << fixture
        << "\",\"xL\":" << d.xL << ",\"xR\":" << d.xR
        << ",\"t0\":" << d.t0 << ",\"t1\":" << d.t1
        << ",\"nx\":" << grid.nx() << ",\"nt\":" << grid.nt()
        << ",\"dx\":" << grid.dx() << ",\"dt\":" << grid.dt()
        << ",\"theta\":" << theta << ",\"E_inf\":" << errors.inf
        << ",\"E_2\":" << errors.l2 << ",\"build\":\"" << PDE_BUILD_ID
        << "\",\"source_revision\":\"" << PDE_SOURCE_REVISION << "\"}\n";
    return errors;
}

void checkRates(char const* fixture, std::vector<Errors> const& errors,
                double lower, double upper, double maxInf, double maxL2) {
    ASSERT_EQ(errors.size(), 3u);
    for (std::size_t j = 1; j < errors.size(); ++j) {
        ASSERT_GT(errors[j - 1].inf, 0.0);
        ASSERT_GT(errors[j].inf, 0.0);
        ASSERT_GT(errors[j - 1].l2, 0.0);
        ASSERT_GT(errors[j].l2, 0.0);
        const double pInf = std::log2(errors[j - 1].inf / errors[j].inf);
        const double pL2 = std::log2(errors[j - 1].l2 / errors[j].l2);
        std::cout << std::setprecision(17) << "ACCEPTANCE {\"fixture\":\"" << fixture
            << "\",\"pair\":" << j << ",\"p_inf\":" << pInf << ",\"p_2\":" << pL2
            << ",\"build\":\"" << PDE_BUILD_ID << "\",\"source_revision\":\""
            << PDE_SOURCE_REVISION << "\"}\n";
        EXPECT_TRUE(std::isfinite(pInf));
        EXPECT_TRUE(std::isfinite(pL2));
        EXPECT_GE(pInf, lower);
        EXPECT_LE(pInf, upper);
        EXPECT_GE(pL2, lower);
        EXPECT_LE(pL2, upper);
    }
    EXPECT_LE(errors.back().inf, maxInf);
    EXPECT_LE(errors.back().l2, maxL2);
}

PDE1D variableCoefficients() {
    PDE1D pde;
    pde.diffusion = [](double t, double x) { return 1 + 0.2 * t + 0.1 * x; };
    pde.advection = [](double t, double x) { return 0.3 + 0.1 * t - 0.2 * x; };
    pde.reaction = [](double t, double x) { return -0.4 + 0.2 * t + 0.1 * x; };
    return pde;
}

TEST(Numerics, SpatialConvergenceAllTerms) {
    auto pde = variableCoefficients();
    Func::F1D exact = [](double t, double x) { return std::exp(t) * (1 + x + std::sin(pi * x)); };
    pde.initial = pde.boundaryL = pde.boundaryR = exact;
    pde.source = [](double t, double x) {
        const double a = 1 + 0.2 * t + 0.1 * x;
        const double b = 0.3 + 0.1 * t - 0.2 * x;
        const double c = -0.4 + 0.2 * t + 0.1 * x;
        const double p = 1 + x + std::sin(pi * x);
        return std::exp(t) * ((1 - c) * p + a * pi * pi * std::sin(pi * x) - b * (1 + pi * std::cos(pi * x)));
    };
    std::vector<Errors> errors;
    for (std::size_t nx : {21u, 41u, 81u}) {
        Grid1D grid(Domain1D(0, 1, 0, 0.1), nx, 4096);
        errors.push_back(measure("space_all_terms", grid, pde, 0.5, exact));
    }
    checkRates("space_all_terms", errors, 1.85, 2.15, 2e-4, 2e-4);
}

TEST(Numerics, CrankNicolsonTemporalConvergenceVariableCoefficientsAndBoundaries) {
    auto pde = variableCoefficients();
    Func::F1D exact = [](double t, double x) { return std::exp(t) * (1 + x + x * x); };
    pde.initial = pde.boundaryL = pde.boundaryR = exact;
    pde.source = [](double t, double x) {
        const double a = 1 + 0.2 * t + 0.1 * x;
        const double b = 0.3 + 0.1 * t - 0.2 * x;
        const double c = -0.4 + 0.2 * t + 0.1 * x;
        return std::exp(t) * ((1 - c) * (1 + x + x * x) - 2 * a - b * (1 + 2 * x));
    };
    std::vector<Errors> errors;
    for (std::size_t nt : {20u, 40u, 80u}) {
        Grid1D grid(Domain1D(0, 1, 0, 0.2), 41, nt);
        errors.push_back(measure("time_CN", grid, pde, 0.5, exact));
    }
    checkRates("time_CN", errors, 1.8, 2.2, 2e-6, 2e-6);
}

TEST(Numerics, NondefaultThetaFirstOrderTemporalConvergence) {
    PDE1D pde;
    pde.diffusion = [](double, double) { return 1.0; };
    pde.advection = pde.reaction = [](double, double) { return 0.0; };
    pde.source = [](double t, double x) { return std::exp(t) * (x * (1 - x) + 2); };
    Func::F1D exact = [](double t, double x) { return std::exp(t) * x * (1 - x); };
    pde.initial = pde.boundaryL = pde.boundaryR = exact;
    for (double theta : {0.75, 1.0}) {
        const char* fixture = theta == 0.75 ? "time_theta_075" : "time_theta_1";
        std::vector<Errors> errors;
        for (std::size_t nt : {10u, 20u, 40u}) {
            Grid1D grid(Domain1D(0, 1, 0, 0.2), 41, nt);
            errors.push_back(measure(fixture, grid, pde, theta, exact));
        }
        checkRates(fixture, errors, 0.85, 1.15, 1e-4, 7e-5);
    }
}

TEST(Numerics, FixedHeatBenchmark) {
    PDE1D pde;
    pde.diffusion = [](double, double) { return 1.0; };
    pde.advection = pde.reaction = pde.source = [](double, double) { return 0.0; };
    pde.initial = [](double, double x) { return std::sin(pi * x); };
    pde.boundaryL = pde.boundaryR = [](double, double) { return 0.0; };
    Func::F1D exact = [](double t, double x) { return std::exp(-pi * pi * t) * std::sin(pi * x); };
    Grid1D grid(Domain1D(0, 1, 0, 0.1), 101, 1000);
    const auto errors = measure("heat_fixed", grid, pde, 0.5, exact);
    EXPECT_LE(errors.inf, 4e-5);
    EXPECT_LE(errors.l2, 3e-5);
}
} // namespace
