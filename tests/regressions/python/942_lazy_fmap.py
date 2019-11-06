#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #942: `fold_left`, `fold_right` and `fmap` do not work with a lazy function


import numpy as np
from phylanx import Phylanx, PhylanxSession, execution_tree


PhylanxSession.init(1)


def variable(value, dtype=None, name=None, constraint=None):
    if dtype is None:
        dtype = "float32"
    if constraint is not None:
        raise TypeError("Constraint is the projection function to be "
                        "applied to the variable after an optimizer update")
    if isinstance(value, execution_tree.variable):
        if dtype is not None:
            value.dtype = dtype
        if name is not None:
            value.name = name
        return value
    return execution_tree.variable(value, dtype=dtype, name=name)


def eval(func):
    return func.eval()


def fmap(fn, elems):
    pass      # make flake happy


@Phylanx
def map_fn_eager(fn, elems, dtype=None):
    return fmap(fn, elems)


def map_fn(fn, elems, dtype=None):
    return map_fn_eager.lazy(fn, elems, dtype)


@Phylanx
def sum_eager(x, axis=None, keepdims=False):
    return np.sum(x, axis, keepdims)


sum = Phylanx.lazy(sum_eager)


def test_map(x):
    return eval(map_fn(sum, variable(x)))


result = test_map(np.array([[1, 2, 3]]))
assert(np.all(result == [6])), result
result = test_map(np.array([1, 2, 3]))
assert(np.all(result == [1, 2, 3])), result
