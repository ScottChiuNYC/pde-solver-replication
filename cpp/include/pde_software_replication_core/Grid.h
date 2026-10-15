#pragma once

#include "Domain.h"
#include <cstddef>
#include <vector>

namespace PDE {
class Grid1D {
public:
    Grid1D(Domain1D const& domain, std::size_t nx, std::size_t nt)
        : m_domain(domain), m_nx(nx), m_nt(nt), m_dx(0.0), m_dt(0.0) {
        if (nx < 3 || nx > std::vector<double>().max_size())
            throw std::runtime_error("grid nx: invalid spatial cardinality");
        if (nt < 1) throw std::runtime_error("grid nt: invalid time cardinality");
        detail::validateEndpoints(m_domain.xL, m_domain.xR, m_domain.t0, m_domain.t1);
        const double width = detail::span(m_domain.xL, m_domain.xR, "grid spatial span");
        const double duration = detail::span(m_domain.t0, m_domain.t1, "grid time span");
        const double dx = detail::div(width, double(nx - 1), "grid dx");
        const double dt = detail::div(duration, double(nt), "grid dt");
        if (!(dx > 0.0)) throw std::runtime_error("grid dx: nonpositive spacing");
        if (!(dt > 0.0)) throw std::runtime_error("grid dt: nonpositive spacing");
        m_dx = dx;
        m_dt = dt;
    }
    std::size_t nx() const { return m_nx; }
    std::size_t nt() const { return m_nt; }
    double dx() const { return m_dx; }
    double dt() const { return m_dt; }
    double x(std::size_t i) const {
        if (i >= m_nx) throw std::runtime_error("grid x index " + std::to_string(i));
        if (i == 0) return m_domain.xL;
        if (i == m_nx - 1) return m_domain.xR;
        const std::string context = "grid x index " + std::to_string(i);
        return detail::add(m_domain.xL, detail::mul(double(i), m_dx, context), context);
    }
    double t(std::size_t k) const {
        if (k > m_nt) throw std::runtime_error("grid t index " + std::to_string(k));
        if (k == 0) return m_domain.t0;
        if (k == m_nt) return m_domain.t1;
        const std::string context = "grid t index " + std::to_string(k);
        return detail::add(m_domain.t0, detail::mul(double(k), m_dt, context), context);
    }
    Domain1D const& domain() const { return m_domain; }
private:
    Domain1D m_domain;
    std::size_t m_nx, m_nt;
    double m_dx, m_dt;
};
} // namespace PDE
