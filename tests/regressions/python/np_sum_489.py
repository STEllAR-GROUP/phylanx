# Copyright (c) 2018 Weile Wei
# Copyright (c) 2018 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Fixing #489: @Phylanx np.sum needs be written in transformation.py

from phylanx.ast import *
import numpy as np


@Phylanx
def testing_sum(a):
    return np.sum(a)


assert (6 == testing_sum(np.array([1, 2, 3])))
assert (0.0 == testing_sum(np.array([])))
