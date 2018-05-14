# Copyright (c) 2018 Parsa Amini
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Fixing #410: Different results from numpy when performing arithmetic
# operations between a vector and a matrix with only one column

import numpy as np
from phylanx.ast import Phylanx


@Phylanx()
def f(lhs, rhs):
    return lhs + rhs


def compare_add_with_numpy(l, r):
    lhs = np.array(l)
    rhs = np.array(r)

    expected = lhs + rhs
    actual = f(lhs, rhs)
    return np.array_equal(expected, actual)


# (1)+(1)
assert compare_add_with_numpy([1.], [2.])
# (1)+(2)
assert compare_add_with_numpy([1.], [2., 3.])
# (2)+(1)
assert compare_add_with_numpy([1., 2.], [3.])
# (2x1)+(1x2)
assert compare_add_with_numpy([[1.], [2.]], [[1., 2.]])
# (1x2)+(2x1)
assert compare_add_with_numpy([[1., 2.]], [[1.], [2.]])
# (2)+(2x1)
assert compare_add_with_numpy([1., 2.], [[1.], [2.]])
# (1x2)+(2)
assert compare_add_with_numpy([[1., 2.]], [1., 2.])
# (1x2)+(1x1)
assert compare_add_with_numpy([[1., 2.]], [[3.]])
# (1x1)+(1x2)
assert compare_add_with_numpy([[3.]], [[1., 2.]])
# (1x2)+(1)
assert compare_add_with_numpy([[1., 2.]], [3.])
# (1)+(1x2)
assert compare_add_with_numpy([3.], [[1., 2.]])
# (2)+(1x2)
assert compare_add_with_numpy([3., 3.], [[1., 2.]])
# (1x1)+(1x2)
assert compare_add_with_numpy([[3.]], [[1., 2.]])
# (2x1)+(1x1)
assert compare_add_with_numpy([[1.], [2.]], [[3.]])
# (1x1)+(2x1)
assert compare_add_with_numpy([[3.]], [[1.], [2.]])
