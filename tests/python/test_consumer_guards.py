"""The installed-consumer checks must reject nonfinite solver output.

Compile the real two consumer translation units against an API-level test
double. This exercises their executable exit status, independently of the
production solver and without changing its implementation.
"""
import os
from pathlib import Path
import subprocess

import pytest


@pytest.fixture(scope="module")
def consumer_guard_executable(tmp_path_factory):
    project = Path(__file__).resolve().parents[2]
    work = tmp_path_factory.mktemp("consumer-guards")
    include = work / "include" / "pde_software_replication_core"
    include.mkdir(parents=True)
    (include / "PDE.h").write_text(r'''
#pragma once
#include <pde_software_replication_core/Array.h>
#include <pde_software_replication_core/Equation.h>
#include <pde_software_replication_core/Grid.h>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <string>
namespace PDE {
class ConsumerTestDouble {
    Grid1D const& grid;
public:
    explicit ConsumerTestDouble(Grid1D const& g) : grid(g) {}
    Array1D solve() const {
        Array1D result(grid.nx(), 1.0);
        if (grid.nx() == 101) {
            const double pi = std::acos(-1.0);
            for (std::size_t i = 0; i < grid.nx(); ++i)
                result[i] = std::exp(-pi*pi*grid.domain().t1)*std::sin(pi*grid.x(i));
            result[0] = result[grid.nx()-1] = 0.0;
        }
        const char* badGrid = std::getenv("CONSUMER_TEST_BAD_GRID");
        if (badGrid && grid.nx() == std::stoul(badGrid)) {
            const std::string kind = std::getenv("CONSUMER_TEST_BAD_VALUE");
            if (kind == "empty") return Array1D();
            result[1] = kind == "nan" ? std::numeric_limits<double>::quiet_NaN()
                : kind == "inf" ? std::numeric_limits<double>::infinity()
                : -std::numeric_limits<double>::infinity();
        }
        return result;
    }
};
inline ConsumerTestDouble makeSolver1D(Grid1D const& grid, PDE1D const&) {
    return ConsumerTestDouble(grid);
}
}
''', encoding="utf-8")
    sources = [project / "tests/vcpkg-consumer/main.cpp",
               project / "tests/vcpkg-consumer/solver_probe.cpp",
               project / "cpp/src/hello.cpp"]
    source_list = "\n".join(f'"{p.as_posix()}"' for p in sources)
    (work / "CMakeLists.txt").write_text(f'''
cmake_minimum_required(VERSION 3.25)
project(consumer_guards LANGUAGES CXX)
add_executable(consumer_guards {source_list})
target_compile_features(consumer_guards PRIVATE cxx_std_17)
target_include_directories(consumer_guards PRIVATE
    "{(work / 'include').as_posix()}" "{(project / 'cpp/include').as_posix()}")
set_target_properties(consumer_guards PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "{(work / 'bin').as_posix()}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "{(work / 'bin').as_posix()}")
''', encoding="utf-8")
    subprocess.run(["cmake", "-S", str(work), "-B", str(work / "build")], check=True)
    subprocess.run(["cmake", "--build", str(work / "build"), "--config", "Release"], check=True)
    return work / "bin" / ("consumer_guards.exe" if os.name == "nt" else "consumer_guards")


def test_consumer_accepts_finite_reference_data(consumer_guard_executable):
    env = os.environ.copy()
    env.pop("CONSUMER_TEST_BAD_GRID", None)
    result = subprocess.run([str(consumer_guard_executable)], env=env, check=False)
    assert result.returncode == 0


@pytest.mark.parametrize("grid_size", [5, 101])
@pytest.mark.parametrize("bad_value", ["nan", "inf", "-inf", "empty"])
def test_consumer_rejects_nonfinite_data(consumer_guard_executable, grid_size, bad_value):
    env = dict(os.environ, CONSUMER_TEST_BAD_GRID=str(grid_size),
               CONSUMER_TEST_BAD_VALUE=bad_value)
    result = subprocess.run([str(consumer_guard_executable)], env=env, check=False)
    assert result.returncode != 0
