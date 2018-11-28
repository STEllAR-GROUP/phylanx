#  Copyright (c) 2017-2018 Hartmut Kaiser
#  Copyright (c) 2018 Steven R. Brandt
#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from phylanx import Phylanx, parallel, PhylanxSession

PhylanxSession(1)


def fib0(n):
    if n < 2:
        return n
    return fib0(n - 1) + fib0(n - 2)


@Phylanx
def fib_(n, a, b):
    if n < 2:
        return n
    elif n < 12:
        a[n] = fib_(n - 1, a, b)
        b[n] = fib_(n - 2, a, b)
        return a[n] + b[n]
    else:
        with parallel:
            a[n] = fib_(n - 1, a, b)
            b[n] = fib_(n - 2, a, b)
        return a[n] + b[n]


def fib(n):
    return fib_(n, np.zeros(n + 1), np.zeros(n + 1))


n = 15
assert fib(n) == fib0(n)
