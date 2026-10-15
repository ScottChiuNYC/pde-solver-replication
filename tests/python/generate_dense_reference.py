"""Regenerate the independent C++ fixture using exact rational arithmetic.

Run directly with Python; no numerical libraries or production solver imports.
The full matrix includes boundary equations and uses dense pivoted elimination.
"""
from fractions import Fraction as F


def dense_solve(matrix, rhs):
    n = len(rhs)
    for k in range(n):
        pivot = max(range(k, n), key=lambda j: abs(matrix[j][k]))
        matrix[k], matrix[pivot] = matrix[pivot], matrix[k]
        rhs[k], rhs[pivot] = rhs[pivot], rhs[k]
        for i in range(k + 1, n):
            weight = matrix[i][k] / matrix[k][k]
            for j in range(k, n):
                matrix[i][j] -= weight * matrix[k][j]
            rhs[i] -= weight * rhs[k]
    solution = [F(0)] * n
    for i in reversed(range(n)):
        solution[i] = (rhs[i] - sum(matrix[i][j] * solution[j]
                                   for j in range(i + 1, n))) / matrix[i][i]
    return solution


def reference(theta):
    nx, h, dt = 6, F(3, 5), F(1, 5)
    space = [-1 + i*h for i in range(nx)]
    times = [F(1, 5) + i*dt for i in range(4)]
    a = lambda t, x: 1 + t*t + x*x/10
    b = lambda t, x: F(1, 10) + t*x/2
    c = lambda t, x: F(1, 5) - t/4 + x/20
    f = lambda t, x: 2*t*t - x + t*x*x/7
    left = lambda t: 3 + t*t
    right = lambda t: -1 + 2*t + t*t*t
    values = [1 + x + x*x*x/8 for x in space]
    values[0], values[-1] = left(times[0]), right(times[0])
    for t0, t1 in zip(times, times[1:]):
        matrix = [[F(0) for _ in range(nx)] for _ in range(nx)]
        rhs = [F(0)] * nx
        matrix[0][0] = matrix[-1][-1] = F(1)
        rhs[0], rhs[-1] = left(t1), right(t1)
        for i in range(1, nx-1):
            x = space[i]
            aa = (a(t0, x) + a(t1, x))/2
            bb = (b(t0, x) + b(t1, x))/2
            cc = (c(t0, x) + c(t1, x))/2
            operator = [aa/(h*h)-bb/(2*h), -2*aa/(h*h)+cc, aa/(h*h)+bb/(2*h)]
            rhs[i] = values[i] + dt*((1-theta)*sum(
                weight*values[j] for weight, j in zip(operator, [i-1, i, i+1]))
                + (1-theta)*f(t0, x) + theta*f(t1, x))
            for weight, j in zip(operator, [i-1, i, i+1]):
                matrix[i][j] = (1 if j == i else 0) - theta*dt*weight
        values = dense_solve(matrix, rhs)
    return values


if __name__ == "__main__":
    for theta in (F(1, 2), F(5, 8), F(3, 4), F(1)):
        print(f"theta={theta}: " + ", ".join(format(float(v), ".17g") for v in reference(theta)))
