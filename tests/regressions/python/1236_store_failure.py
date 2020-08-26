#  Copyright (c) 2020 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #1236: Can't store to an array element

from phylanx import Phylanx
import numpy as np


@Phylanx
def foo():
    z = np.zeros((3, 4))
    z[1][0] = 1

    return [z[1][0], z[1, 0]]


result = foo()
assert result[0] == 1, result[0]
assert result[1] == 1, result[1]
