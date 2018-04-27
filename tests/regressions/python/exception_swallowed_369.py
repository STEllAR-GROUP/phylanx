#  Copyright (c) 2018 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from phylanx.ast import *


@Phylanx()
def addem(a, b):
    c = a + b
    print('c=', c)
    return c


exception_thrown = False
try:
    print('addem=(', addem(np.zeros((2,)), np.zeros((2, 1))), ')')

except Exception as e:
    expected = \
        '<unknown>: __add$0/0$11$8:: vector size does not match number of ' + \
        'matrix columns: HPX(bad_parameter)'
    assert(str(e) == expected)
    exception_thrown = True


assert(exception_thrown)
