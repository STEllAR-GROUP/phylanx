#  Copyright (c) 2021 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #1281: Phylanx Minimum Viable Product

from phylanx import Phylanx
import numpy as np


@Phylanx
def foo():
    li = [1, 2, 3, 4, 5]
    return np.array(li)


result = foo()
assert (result == np.array([1, 2, 3, 4, 5])).all(), result
