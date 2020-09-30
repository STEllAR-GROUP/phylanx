#  Copyright (c) 2020 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #1266: np.sum does not know the axis argument

from phylanx import Phylanx
import numpy as np


@Phylanx
def test_sum(d):
    return np.sum(d, axis=0, dtype='int')


result = test_sum(np.array([1, 2, 3, 4, 5]))
assert result == 15, result
