#  Copyright (c) 2020 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #1258: np.random.randn does not work

from phylanx import Phylanx
import numpy as np


@Phylanx
def generate():
    return np.random.randn(100)


result = generate()
assert np.shape(result) == (100,), result
