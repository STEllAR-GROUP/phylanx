#  Copyright (c) 2020 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# 1306: Error Running Random Forrest

from phylanx import Phylanx
import numpy as np


@Phylanx
def foo():
    arr = np.array([0, 1, 2, 3, 4])
    inds = np.array([2, 1, 3, 4])
    return arr[inds][0:2]


result = foo()
assert (result == np.array([2, 1])).all(), result
