#  Copyright (c) 2021 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #1301: Error processing if statement

from phylanx import Phylanx
import numpy as np


def __type(x):      # silence Flake
    pass


@Phylanx
def lshape(x):
    if (__type(x) == __type(list()) and len(x) == 0):
        return (0,)
    else:
        return np.shape(x)


result = lshape(np.linspace(0, 1, 100))
assert result == [100], result
