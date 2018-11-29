#  Copyright (c) 2018 Weile Wei
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx
import numpy as np


@Phylanx
def phlyanx_array_shape(a):
    return np.shape(a)


assert ([3, 1] == phlyanx_array_shape(np.array([[1], [1], [0]])))
