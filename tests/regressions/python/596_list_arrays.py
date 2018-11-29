#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx
import numpy as np


@Phylanx
def foo(x, y):
    return [x, y]


x = np.array([1, 2])
y = np.array([3, 4])
result = foo(x, y)
assert((result[0] == x).all() and (result[1] == y).all())
