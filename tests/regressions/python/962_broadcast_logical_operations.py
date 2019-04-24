#  Copyright (c) 2019 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #962: `__eq` operation: comparing n-by-m and n-by-1 arrays


import numpy as np
from phylanx import Phylanx


@Phylanx
def foo():
    arr_2_2 = np.array([[1, 0], [0, 1]])
    arr_2_1 = np.array([[0], [1]])
    return arr_2_2 == arr_2_1


def np_foo():
    arr_2_2 = np.array([[1, 0], [0, 1]])
    arr_2_1 = np.array([[0], [1]])
    return arr_2_2 == arr_2_1


assert (np_foo() == foo()).all()

