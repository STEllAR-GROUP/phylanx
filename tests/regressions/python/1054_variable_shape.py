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
        dtype = "float64"
    from phylanx.ast.physl import PhySL
    if isinstance(value, PhySL.eval_wrapper):
        return execution_tree.variable(value.code(), dtype)
    if isinstance(value, execution_tree.variable):
        return value
    return execution_tree.variable(value, dtype=dtype, name=name)


def eval(x):
    return x.eval()


shape = variable((None, 2), np.dtype('float64'))._keras_shape
assert(shape == (None, 2))


x_elem1 = variable(np.array([1, 2, 3, 4]))
x_elem2 = variable(np.array([[1, 2, 3, 4], [5, 6, 7, 8]]))
x_elem3 = variable(np.array([[[1, 2, 3, 4], [5, 6, 7, 8]], [[1, 2, 3, 4], [5, 6, 7, 8]]]))

inputs = [x_elem1, x_elem2, x_elem3]

shapes = []
for x_elem in inputs:
    assert(hasattr(x_elem, '_keras_shape'))
    shapes.append(x_elem._keras_shape)


assert(shapes == [(4,), (2, 4), (2, 2, 4)])
