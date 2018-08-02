#  Copyright (c) 2018 Weile Wei
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx
import numpy as np


@Phylanx(debug=True)
def foo(x):
    local_x = x
    result = []
    for ele in local_x[::-1][:-1]:
        result += ele
    return result


a = np.array([[1, 2], [3, 4], [5, 6], [7, 8]])
assert(foo(a) == [7, 8, 5, 6, 3, 4])
