#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# 925: Cannot pass a None dtype

import numpy as np
from phylanx import Phylanx


@Phylanx
def eye_eager(n, dtype, name):
    return np.eye(n, dtype=dtype)


def eye(n, dtype=None, name=None):
    return eye_eager.lazy(n, dtype, name)


result = eye(3).eval()
expected = np.array([[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]])
assert result.dtype.name == 'float64'
assert (result == expected).all()
