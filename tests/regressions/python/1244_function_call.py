#  Copyright (c) 2020 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #1244: Function call weirdness

from phylanx import Phylanx
import numpy as np


@Phylanx
def fun(a):
    return a


@Phylanx
def foo(c):
    return fun(c[0])


result = foo(np.array([0, 1]))
assert result == 0, result
