#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from phylanx import Phylanx, PhylanxSession, execution_tree


PhylanxSession.init(1)


_FLOATX = "float64"


def floatx():
    return _FLOATX


def variable(value, dtype=None, name=None, constraint=None):
    if dtype is None:
        dtype = floatx()
    if constraint is not None:
        raise TypeError("Constraint is the projection function to be "
                        "applied to the variable after an optimizer update")
    from phylanx.ast.physl import PhySL
    if isinstance(value, PhySL.eval_wrapper):
        return execution_tree.variable(value.code(), dtype)
    if isinstance(value, execution_tree.variable):
        return value
    return execution_tree.variable(value, dtype=dtype, name=name)


def dtype(x):
    from phylanx.ast.physl import PhySL
    if isinstance(x, PhySL.eval_wrapper):
        return execution_tree.variable(x.code(), dtype).dtype
    if isinstance(x, execution_tree.variable):
        return x.dtype
    return execution_tree.variable(x, dtype).dtype


@Phylanx
def arange_eager(start, stop, step, dtype):
    return np.arange(start, stop=stop, step=step, dtype=dtype)


def arange(start, stop=None, step=1, dtype='int32'):
    return variable(arange_eager.lazy(start, stop, step, dtype), dtype=dtype)


def eval(x):
    return x.eval()


@Phylanx
def constant_eager(value, dtype, shape):
    return constant(value, shape, dtype=dtype)


def constant(value, dtype=None, shape=None, name=None):
    return constant_eager.lazy(value, dtype, shape)
