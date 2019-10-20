#  Copyright (c) 2019 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


import numpy as np
from phylanx import Phylanx, PhylanxSession, execution_tree

PhylanxSession.init(1)


def variable(value, dtype=None, name=None):
    if dtype is None:
        dtype = "int64"
    if isinstance(value, execution_tree.variable):
        return value
    return execution_tree.variable(value, dtype=dtype, name=name)


@Phylanx
def extract_len(x):
    return len(x)


shape = variable((None, 2), np.dtype('int64'))._keras_shape
assert(shape == (None, 2))


x_elem1 = variable(np.array([1, 2, 3, 4], dtype='int64'))
x_elem2 = variable(np.array([[1, 2, 3, 4], [5, 6, 7, 8]]), dtype='int64')
x_elem3 = variable(
    np.array([[[1, 2, 3, 4], [5, 6, 7, 8]], [[1, 2, 3, 4], [5, 6, 7, 8]]]),
    dtype='int64')

inputs = [x_elem1, x_elem2, x_elem3]

lens = []
for x_elem in inputs:
    assert(hasattr(x_elem, '__len__'))
    lens.append(extract_len(x_elem))
assert(lens == [4, 2, 2])


for x_elem in inputs:
    r = None
    for x in x_elem:
        if r is None:
            r = np.array([x.eval()], dtype=x_elem.dtype)
        else:
            r = np.concatenate((r, [x.eval()]), axis=0)
    assert(np.all(r == x_elem.eval()))
