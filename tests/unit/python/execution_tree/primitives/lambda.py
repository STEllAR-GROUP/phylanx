# Copyright (c) 2018 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx
from phylanx.ast import *

import numpy as np


@Phylanx
def f(a, b):
    return map(lambda x, y: x * y, a, b)


assert (f(np.array([1, 2]), np.array([3, 4])) == np.array([3, 8])).all()


@Phylanx
def f(a):
    return map(lambda x: x * 2, a)


assert f(np.array([1])) == np.array([2])
