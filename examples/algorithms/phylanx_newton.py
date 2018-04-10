#  Copyright (c) 2018 Adrain Serio
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx
from phylanx.ast import *

print("hello world!")

print(phylanx.major_version())

@Phylanx("PhySL")
def print_func(x):
    cout(x)

@Phylanx("PhySL")
def func (a):
    a = 4.0-(a*a)
    return a

@Phylanx("PhySL")
def d_func (a):
    a = 2.0 * a
    return a

@Phylanx("PhySL")
def newton(xn, max_iter, cutoff):
    # Newton's Method
    # 
    #   xn: Initial guess
    #   max_iter: Max number of iterations
    #   cutoff: Value of error for convergence

    error = 1.0
    iter = 0.0
    fx = 0.0
    dfx = 0.0
    x_guess = 0.0
    xx = xn
    while error > cutoff and iter < max_iter:
#    while error > cutoff:
        cout("xx - before", xx)
        iter += 1.0
        fx = func(xx)
        dfx = d_func(xx)
        x_guess = xx + (fx/dfx)
        error = xx - x_guess
        xx = x_guess
        cout("xx - after", xx)
    return xx

strg = "Decorator!"
print_func(strg)
print(newton(3.0, 10.0, 0.000001))
