#  Copyright (c) 2020 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
import numpy.linalg as LA

from phylanx import Phylanx


@Phylanx
def test(x):
    return LA.norm(x)


result = test(np.arange(9) - 4.)
assert result == 7.745966692414834, result
