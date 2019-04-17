#  Copyright (c) 2019 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np

import phylanx
from phylanx import Phylanx

arr = []

arr.append(np.array([1, -2, 3]))
arr.append(np.array([[1, 2], [4, -3]]))
arr.append(np.array([[[1, 2], [4, 3]], [[11, 22], [44, -1]]]))

@Phylanx
def physl_argsort(arr, axis):
    return argsort(arr, axis)

def test_argsort(arr, axis):
    assert (physl_argsort(arr, axis) == np.argsort(arr, axis)).all()

max_num_dimensions = 3
for dim in range(max_num_dimensions):
    for axis in range (-dim-1, dim+1):
        test_argsort(arr[dim], axis )
    # test flatten
    test_argsort(arr[dim], None )

