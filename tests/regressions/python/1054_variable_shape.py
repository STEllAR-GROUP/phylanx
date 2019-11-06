#  Copyright (c) 2019 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #1054: Add support for variable._keras_shape


import numpy as np
from phylanx import PhylanxSession, execution_tree

PhylanxSession.init(1)


def variable(value, dtype=None, name=None):
    if dtype is None:
        dtype = "int64"
    if isinstance(value, execution_tree.variable):
        if dtype is not None:
            value.dtype = dtype
        if name is not None:
            value.name = name
        return value
    return execution_tree.variable(value, dtype=dtype, name=name)


shape = variable((None, 2), np.dtype('int64'))._keras_shape
assert(shape == (None, 2))


x_elem1 = variable(np.array([1, 2, 3, 4], dtype='int64'))
x_elem2 = variable(np.array([[1, 2, 3, 4], [5, 6, 7, 8]]), dtype='int64')
x_elem3 = variable(
    np.array([[[1, 2, 3, 4], [5, 6, 7, 8]], [[1, 2, 3, 4], [5, 6, 7, 8]]]),
    dtype='int64')


inputs = [x_elem1, x_elem2, x_elem3]


shapes = []
for x_elem in inputs:
    assert(hasattr(x_elem, '_keras_shape'))
    shapes.append(x_elem._keras_shape)
assert(shapes == [(4,), (2, 4), (2, 2, 4)])
