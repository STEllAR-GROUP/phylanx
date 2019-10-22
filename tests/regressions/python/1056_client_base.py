#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #1056: RuntimeError: this client_base has no valid shared state: HPX(no_state)


from phylanx import Phylanx, PhylanxSession, execution_tree
import numpy as np


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


@Phylanx
def sum_eager(x, axis, keepdims):
    return np.sum(x, axis, keepdims)


def sum(x, axis=None, keepdims=False):
    return sum_eager.lazy(x, axis, keepdims)


@Phylanx
def reshape_eager(x, shape):
    return np.reshape(x, shape)


def reshape(x, shape):
    return reshape_eager.lazy(x, shape)


@Phylanx
def squeeze_eager(x, axis):
    return np.squeeze(x, axis)


def squeeze(x, axis):
    return squeeze_eager.lazy(x, axis)


y_test = variable(np.array([0, 0, 2, 0]))
assert(np.all(eval(y_test) == [0, 0, 2, 0]))

y_test = reshape(y_test, (4, 1))
y_test = squeeze(y_test, 1)
assert(np.all(eval(y_test) == [0, 0, 2, 0]))

sum_y_test = sum(y_test)
assert(eval(sum_y_test) == 2)
