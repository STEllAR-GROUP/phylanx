#  Copyright (c) 2020 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #1280: Problem creating an hstack

from phylanx import Phylanx
import numpy as np


@Phylanx
def test():
    return np.diag(np.hstack([[1, 2], [3, 4]]))


result = test()
assert (result == [1, 4]).all(), result
