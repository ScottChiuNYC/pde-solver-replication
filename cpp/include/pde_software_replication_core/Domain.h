#pragma once

#include "detail/Checked.h"

namespace PDE {
namespace detail {
inline void validateEndpoints(double xL, double xR, double t0, double t1) {
    finite(xL, "domain xL");
    finite(xR, "domain xR");
    finite(t0, "domain t0");
    finite(t1, "domain t1");
    if (!(xR > xL)) throw std::runtime_error("domain xR must exceed xL");
    if (!(t1 > t0)) throw std::runtime_error("domain t1 must exceed t0");
}
inline double span(double left, double right, std::string const& field) {
    const double result = sub(right, left, field);
    if (!(result > 0.0)) throw std::runtime_error(field + ": nonpositive span");
    return result;
}
}

struct Domain1D {
    double xL, xR, t0, t1;
    Domain1D(double xLeft, double xRight, double time0, double time1)
        : xL(xLeft), xR(xRight), t0(time0), t1(time1) {
        detail::validateEndpoints(xL, xR, t0, t1);
        detail::span(xL, xR, "domain spatial span");
        detail::span(t0, t1, "domain time span");
    }
};
} // namespace PDE
