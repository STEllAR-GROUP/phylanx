#  Copyright (c) 2018 Weile Wei
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx
import numpy as np


@Phylanx
def foo(x):
    local_x = x
    result = []
    sliced_local_x = local_x[::-1]
    for ele in sliced_local_x[:-1]:
        result += ele
    return result


a = [[1, 2], [3, 4], [5, 6], [7, 8]]
result = foo(a)
assert result == [7, 8, 5, 6, 3, 4], result
