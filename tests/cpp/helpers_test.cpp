#include <pde_software_replication_core/PDE.h>
#include <gtest/gtest.h>
#include <limits>
#include <type_traits>
#include <utility>

namespace {
using namespace PDE;
const double inf = std::numeric_limits<double>::infinity();
const double nan = std::numeric_limits<double>::quiet_NaN();
const double maxDouble = std::numeric_limits<double>::max();
const double tiny = std::numeric_limits<double>::denorm_min();

static_assert(std::is_copy_constructible<Scheme::CrankNicholson1D>::value);
static_assert(std::is_move_constructible<Scheme::CrankNicholson1D>::value);
static_assert(!std::is_copy_assignable<Scheme::CrankNicholson1D>::value);
static_assert(!std::is_move_assignable<Scheme::CrankNicholson1D>::value);
static_assert(std::numeric_limits<double>::is_iec559);
static_assert(std::numeric_limits<double>::digits == 53);

TEST(Array, OwnershipAccessAndEmptyStorage) {
    Array1D empty;
    Array1D zero(0, 7.0);
    EXPECT_EQ(empty.size(), 0u);
    EXPECT_EQ(empty.data(), nullptr);
    EXPECT_EQ(zero.data(), nullptr);
    EXPECT_EQ(empty.begin(), empty.end());
    EXPECT_THROW(empty[0], std::runtime_error);
    Array1D one(1);
    EXPECT_EQ(one[0], 0.0);
    Array1D a(3, 2.0);
    Array1D const& ca = a;
    EXPECT_EQ(ca.end() - ca.begin(), 3);
    EXPECT_EQ(ca.data(), &ca[0]);
    a[1] = 4.0;
    a.at(2) = 5.0;
    EXPECT_EQ(ca[1], 4.0);
    EXPECT_EQ(ca.at(2), 5.0);
    EXPECT_EQ(a.data() + 2, &a[2]);
    for (auto i = 0u; i < a.size(); ++i) EXPECT_EQ(*(a.begin() + i), ca[i]);
    auto before = a;
    EXPECT_THROW(a[3] = 0.0, std::runtime_error);
    EXPECT_THROW(a.at(3) = 0.0, std::runtime_error);
    EXPECT_THROW(ca[3], std::runtime_error);
    EXPECT_THROW(ca.at(3), std::runtime_error);
    EXPECT_TRUE(std::equal(a.begin(), a.end(), before.begin()));
    Array1D copy(a);
    copy[0] = 9.0;
    EXPECT_EQ(a[0], 2.0);
    Array1D assigned;
    assigned = a;
    assigned[0] = 8.0;
    EXPECT_EQ(a[0], 2.0);
    a = a;
    EXPECT_EQ(a[1], 4.0);
    auto pointer = a.data();
    a.fill(6.0);
    EXPECT_EQ(a.data(), pointer);
    for (double v : ca) EXPECT_EQ(v, 6.0);
    Array1D moved(std::move(a));
    EXPECT_EQ(a.size(), 0u);
    EXPECT_EQ(a.data(), nullptr);
    EXPECT_EQ(moved.data(), pointer);
    a = std::move(moved);
    EXPECT_EQ(moved.size(), 0u);
    EXPECT_EQ(moved.data(), nullptr);
    a = std::move(a);
    EXPECT_EQ(a.data(), pointer);
    double& element = a[0];
    a.swap(copy);
    EXPECT_EQ(copy.data(), pointer);
    element = 11.0;
    EXPECT_EQ(copy[0], 11.0);
    zero.fill(nan);
    one.fill(inf);
    EXPECT_EQ(one[0], inf); // General container permits nonfinite values.
    EXPECT_EQ(static_cast<Array1D const&>(moved).data(), nullptr);
}

TEST(Grid, OwnsDomainAndUsesExactEndpoints) {
    Grid1D grid = [] {
        Domain1D domain(-0.3, 1.1, 0.2, 0.9);
        Grid1D original(domain, 8, 9);
        Grid1D copy(original);
        domain.xR = 100.0;
        return copy;
    }();
    EXPECT_EQ(grid.nx(), 8u);
    EXPECT_EQ(grid.nt(), 9u);
    EXPECT_EQ(grid.dx(), (1.1 + 0.3) / 7.0);
    EXPECT_EQ(grid.dt(), (0.9 - 0.2) / 9.0);
    EXPECT_EQ(grid.x(0), -0.3);
    EXPECT_EQ(grid.x(7), 1.1);
    EXPECT_EQ(grid.t(0), 0.2);
    EXPECT_EQ(grid.t(9), 0.9);
    // Match the contract's separately rounded binary64 operations. A fused
    // reference expression can differ by one ULP on an FMA-enabled target.
    volatile double xOffset = 3.0 * grid.dx();
    volatile double tOffset = 4.0 * grid.dt();
    const double expectedX = -0.3 + xOffset;
    const double expectedT = 0.2 + tOffset;
    EXPECT_EQ(grid.x(3), expectedX);
    EXPECT_EQ(grid.t(4), expectedT);
    EXPECT_EQ(grid.domain().xR, 1.1);
    EXPECT_THROW(grid.x(8), std::runtime_error);
    EXPECT_THROW(grid.t(10), std::runtime_error);
    EXPECT_THROW(grid.x(std::numeric_limits<std::size_t>::max()), std::runtime_error);
}

TEST(Grid, RejectsInvalidCardinalitiesAndSpacing) {
    Domain1D d(0, 1, 0, 1);
    for (std::size_t nx : {0u, 1u, 2u}) EXPECT_THROW(Grid1D(d, nx, 1), std::runtime_error);
    EXPECT_THROW(Grid1D(d, 3, 0), std::runtime_error);
    EXPECT_THROW(Grid1D(d, std::numeric_limits<std::size_t>::max(), 1), std::runtime_error);
    EXPECT_THROW(Grid1D(Domain1D(0, tiny, 0, 1), 3, 1), std::runtime_error);
    EXPECT_THROW(Grid1D(Domain1D(0, 1, 0, tiny), 3, 2), std::runtime_error);
    d.xR = d.xL;
    EXPECT_THROW(Grid1D(d, 3, 1), std::runtime_error);
    d = Domain1D(0, 1, 0, 1);
    d.t0 = nan;
    EXPECT_THROW(Grid1D(d, 3, 1), std::runtime_error);
    d.t0 = -maxDouble;
    d.t1 = maxDouble;
    EXPECT_THROW(Grid1D(d, 3, 1), std::runtime_error);
}

TEST(Domain, RejectsInvalidEndpointsAndSpans) {
    EXPECT_THROW(Domain1D(1, 0, 0, 1), std::runtime_error);
    EXPECT_THROW(Domain1D(0, 0, 0, 1), std::runtime_error);
    EXPECT_THROW(Domain1D(0, 1, 1, 0), std::runtime_error);
    EXPECT_THROW(Domain1D(0, 1, 1, 1), std::runtime_error);
    for (double bad : {nan, inf, -inf}) {
        EXPECT_THROW(Domain1D(bad, 1, 0, 1), std::runtime_error);
        EXPECT_THROW(Domain1D(0, bad, 0, 1), std::runtime_error);
        EXPECT_THROW(Domain1D(0, 1, bad, 1), std::runtime_error);
        EXPECT_THROW(Domain1D(0, 1, 0, bad), std::runtime_error);
    }
    EXPECT_THROW(Domain1D(-maxDouble, maxDouble, 0, 1), std::runtime_error);
    EXPECT_THROW(Domain1D(0, 1, -maxDouble, maxDouble), std::runtime_error);
}
} // namespace
