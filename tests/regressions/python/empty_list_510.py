#  Copyright (c) 2018 Weile Wei
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx.ast import *
import numpy as np


@Phylanx()
def test_empty_array_1(a, b):
    c = np.array([])
    return c


a = np.array([1, 2, 3])
b = np.array([4, 5])
c = test_empty_array_1(a, b)
assert(c.size == 0)
