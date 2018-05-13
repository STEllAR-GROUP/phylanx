#  Copyright (c) 2018 Parsa Amini
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from phylanx.ast import *


@Phylanx()
def f(a):
    return shape(a)


a = np.array([[1], [2]])
assert(f(a) == [2, 1])
