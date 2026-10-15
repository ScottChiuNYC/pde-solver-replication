#include <pde_software_replication_core/PDE.h>
#include <cmath>

double heatProbe() {
    const double pi = std::acos(-1.0);
    PDE::Domain1D domain(0.0, 1.0, 0.0, 0.1);
    PDE::Grid1D grid(domain, 101, 1000);
    PDE::PDE1D pde;
    pde.initial = [pi](double, double x) { return std::sin(pi * x); };
    pde.boundaryL = pde.boundaryR = [](double, double) { return 0.0; };
    const auto result = PDE::makeSolver1D(grid, pde).solve();
    double error = 0.0;
    for (std::size_t i = 0; i < grid.nx(); ++i)
        error = std::max(error, std::abs(result[i] - std::exp(-pi * pi * domain.t1) * std::sin(pi * grid.x(i))));
    return error;
}
