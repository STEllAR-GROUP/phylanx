#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from phylanx import Phylanx


@Phylanx
def test1():
    x = np.zeros((4, 3))
    return x[0, :]


assert (np.array([0, 0, 0]) == test1()).all


@Phylanx
def test2():
    x = np.zeros((3, 4))
    return x[0, :]


assert (np.array([0, 0, 0, 0]) == test2()).all
