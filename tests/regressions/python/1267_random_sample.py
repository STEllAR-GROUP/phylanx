#  Copyright (c) 2020 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #1267: np.random.random_sample is not available

from phylanx import Phylanx
import numpy as np


@Phylanx
def sample():
    return np.random.random_sample()


result = sample()
assert result >= 0.0 and result < 1.0, result
