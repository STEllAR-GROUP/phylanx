#  Copyright (c) 2020 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# 1312: Iteration on arrays doesn't work

from phylanx import Phylanx
import numpy as np


# silence flake
def append(lst, x):
    return lst


def constant(val, sh, dtype):
    return val


@Phylanx
def foo():
    sh = [2, 2]
    z = constant(0, sh, dtype="float")
    result = []
    for a in z:
        result = append(result, a)
    return result


result = foo()
assert (result == np.array([[0, 0], [0, 0]])).all(), result
