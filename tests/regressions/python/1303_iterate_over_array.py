#  Copyright (c) 2021 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #1303: Can't iterate on a vector

from phylanx import Phylanx
import numpy as np


def append(lst, x):   # silence flake
    return lst


@Phylanx
def foo():
    vec = np.array([1, 2, 3, 4, 5])
    result = []
    for vi in vec:
        result = append(result, vi)
    return result


result = foo()
assert (result == np.array([1, 2, 3, 4, 5])).all(), result
