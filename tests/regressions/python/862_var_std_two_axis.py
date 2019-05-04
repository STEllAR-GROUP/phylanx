#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from phylanx import Phylanx


@Phylanx
def variance(x, axis=None, keepdims=False):
    return np.var(x, axis, keepdims)


y = np.array([[[1., 2, 3]], [[-4, 5, 6]]])

assert np.allclose(variance(y, [1, 2]), np.array([0.66666667, 20.22222222]))
assert np.allclose(variance(y, [0, 1]), np.array([6.25, 2.25, 2.25]))
assert np.allclose(variance(y, [0, 2]), np.array([10.47222222]))
