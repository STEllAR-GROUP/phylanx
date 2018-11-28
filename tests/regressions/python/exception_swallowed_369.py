# Copyright (c) 2018 Steven R. Brandt
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Fixing #369: Exception getting swallowed

import numpy as np
from phylanx import Phylanx, PhylanxSession

PhylanxSession(1)


@Phylanx
def addem(a, b):
    c = a + b
    print('c=', c)
    return c


exception_thrown = False
try:
    print('addem=(', addem(np.zeros((2, 4)), np.zeros((2, 3))), ')')

except Exception as e:
    expected = \
        'exception_swallowed_369.py(16, 8): __add:: cannot broadcast a ' + \
        'matrix into a differently sized matrix: HPX(bad_parameter)'
    assert (str(e).endswith(expected))
    exception_thrown = True

assert (exception_thrown)
