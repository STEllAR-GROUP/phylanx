# Copyright (c) 2018 Steven R. Brandt
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Fixing #369: Exception getting swallowed

import numpy as np
from phylanx import Phylanx


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
        '<unknown>(14, 8): __add:: the dimensions of the ' + \
        'operands do not match: HPX(bad_parameter)'
    assert (str(e) == expected)
    exception_thrown = True

assert (exception_thrown)
