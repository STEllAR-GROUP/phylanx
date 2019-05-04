#  Copyright (c) 2018 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from phylanx import Phylanx


@Phylanx
def test_vstack_default_d():
    x = np.vstack((0.0, 1.0, 2.0, 3.0, 4.0))
    return x


result = test_vstack_default_d()
assert(result.dtype.name == 'float64')
assert((result == np.array([[0.0], [1.0], [2.0], [3.0], [4.0]])).all())


@Phylanx
def test_vstack_default_i():
    x = np.vstack((0, 1, 2, 3, 4))
    return x


result = test_vstack_default_i()
assert(result.dtype.name == 'int64')
assert((result == np.array([[0], [1], [2], [3], [4]])).all())


@Phylanx
def test_vstack_d():
    x = np.vstack((0, 1, 2, 3, 4), dtype=float)
    return x


result = test_vstack_d()
assert(result.dtype.name == 'float64')
assert((result == np.array([[0.0], [1.0], [2.0], [3.0], [4.0]])).all())


@Phylanx
def test_vstack_i():
    x = np.vstack((0, 1, 2, 3, 4), dtype=int)
    return x


result = test_vstack_i()
assert(result.dtype.name == 'int64')
assert((result == np.array([[0], [1], [2], [3], [4]])).all())
