#  Copyright (c) 2018 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #466: ast compiler hangs and gives the wrong answer

import numpy as np
from phylanx import Phylanx, PhylanxSession

PhylanxSession.init(1)


@Phylanx
def kernel1(a, b):
    for i in range(1, np.shape(a)[0] - 1):
        for j in range(1, np.shape(a)[1] - 1):
            v1 = b[i + 1, j] + b[i - 1, j]
            v2 = b[i, j + 1] + b[i, j - 1]
            a[i, j] = v1 + v2
    return a


@Phylanx
def kernel2(a, b):
    for i in range(1, np.shape(a)[0] - 1):
        for j in range(1, np.shape(a)[1] - 1):
            a[i, j] = b[i + 1, j] + b[i - 1, j] + b[i, j + 1] + b[i, j - 1]
    return a


@Phylanx
def kernel3(a, b):
    result = a
    for i in range(1, np.shape(a)[0] - 1):
        for j in range(1, np.shape(a)[1] - 1):
            result[i, j] = b[i + 1, j] + b[i - 1, j] + b[i, j + 1] + b[i, j - 1]
    return result


expected = np.array([[1, 2, 3, 4],
                     [2, 12, 8, 1],
                     [3, 8, 12, 2],
                     [4, 1, 2, 3]])

a = np.array([[1, 2, 3, 4],
              [2, 3, 4, 1],
              [3, 4, 1, 2],
              [4, 1, 2, 3]])
b = np.array([[1, 2, 3, 4],
              [2, 3, 4, 1],
              [3, 4, 1, 2],
              [4, 1, 2, 3]])

assert((kernel1(a, b) == expected).all())
assert((kernel2(a, b) == expected).all())
assert((kernel3(a, b) == expected).all())
