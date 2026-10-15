#include "pde_software_replication_core/hello.h"
#include <pde_software_replication_core/PDE.h>
#include <cmath>
#include <iostream>

double heatProbe();

int main() {
    const auto message = hello();
    std::cout << message << std::endl;
    PDE::Grid1D grid(PDE::Domain1D(0, 1, 0, 0.1), 5, 2);
    PDE::PDE1D pde;
    pde.initial = pde.boundaryL = pde.boundaryR = [](double, double) { return 1.0; };
    const auto result = PDE::makeSolver1D(grid, pde).solve();
    if (result.size() != grid.nx()) return 2;
    for (double value : result)
        if (!std::isfinite(value) || std::abs(value - 1.0) > 1e-12) return 2;
    const double error = heatProbe();
    std::cout << "Two-translation-unit installed solver heat E_inf=" << error << std::endl;
    return message == "Hello, World!" && std::isfinite(error) && error <= 4e-5 ? 0 : 1;
}
