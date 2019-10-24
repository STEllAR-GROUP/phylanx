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
    from phylanx.ast.physl import PhySL
    if isinstance(value, PhySL.eval_wrapper):
        return execution_tree.variable(value.code(), dtype)
    if isinstance(value, execution_tree.variable):
        return value
    return execution_tree.variable(value, dtype=dtype, name=name)


def eval(func):
    return func.eval()


def fmap(fn, elems):
    pass      # make flake happy


@Phylanx
def map_fn_eager(fn, elems, dtype):
    return fmap(fn, elems)


map_fn = Phylanx.lazy(map_fn_eager)


@Phylanx
def sum_eager(x, axis=None, keepdims=False):
    return np.sum(x, axis, keepdims)


sum = Phylanx.lazy(sum_eager)


def test_map(x):
    vx = variable(x)
    kx = eval(map_fn(sum, vx))
    return kx


assert(np.all(test_map(np.array([[1, 2, 3]])) == [6]))
assert(np.all(test_map(np.array([1, 2, 3])) == [1, 2, 3]))
