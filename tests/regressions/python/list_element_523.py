#  Copyright (c) 2018 Weile Wei
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx.ast import *
import numpy as np


@Phylanx
def foo(x, y):
    local_x = x
    local_x[0] += y
    return local_x


a = [1, 1]
b = 1
c = foo(a, b)
assert (c[0] == 2)


@Phylanx
def foo(x, y):
    local_x = x
    local_x[0] += y
    return local_x


a = [np.array([[2], [2]]), np.array([[4], [4]])]
b = np.array([[1], [1]])
c = foo(a, b)
v_0 = c[0] == np.array([[3], [3]])
assert(True == v_0.any())
