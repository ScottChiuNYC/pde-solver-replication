#pragma once

#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>

namespace PDE { namespace detail {

inline double finite(double value, std::string const& context) {
    if (!std::isfinite(value))
        throw std::runtime_error(context + ": nonfinite arithmetic");
    return value;
}

// Volatile binary64 stores make each prescribed operation observable before
// its finite check. Inlining cannot fuse a multiply with a subsequent sum or
// retain a wider intermediate. Consumers must use normal IEEE arithmetic.
inline double add(double a, double b, std::string const& context) {
    volatile double result = a + b;
    return finite(result, context);
}
inline double sub(double a, double b, std::string const& context) {
    volatile double result = a - b;
    return finite(result, context);
}
inline double mul(double a, double b, std::string const& context) {
    volatile double result = a * b;
    return finite(result, context);
}
inline double div(double a, double b, std::string const& context) {
    volatile double result = a / b;
    return finite(result, context);
}
inline double mean(double a, double b, std::string const& context) {
    const double halfMax = std::numeric_limits<double>::max() / 2.0;
    if (std::abs(a) <= halfMax && std::abs(b) <= halfMax)
        return div(add(a, b, context), 2.0, context);
    const double left = div(a, 2.0, context);
    const double right = div(b, 2.0, context);
    return add(left, right, context);
}

}} // namespace PDE::detail
