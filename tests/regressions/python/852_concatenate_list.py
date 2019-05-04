#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from phylanx import Phylanx, PhylanxSession, execution_tree


PhylanxSession.init(1)


def variable(value, dtype=None, name=None, constraint=None):
    if constraint is not None:
        raise TypeError("Constraint is the projection function to be "
                        "applied to the variable after an optimizer update")
    return execution_tree.variable(np.array(value, dtype), dtype)


def eval(func):
    return func.eval()


def parse_shape_or_val(shape_or_val):
    return shape_or_val, np.random.random(shape_or_val).astype(np.float32) - 0.5


###############################################################################
@Phylanx
def concatenate_eager(tensors, axis):
    return np.concatenate(tensors, axis)


def concatenate(tensors, axis=-1):
    return concatenate_eager.lazy(tensors, axis)


###############################################################################
def test_concatenate_operation(x_shape_or_val, y_shape_or_val, axis=-1):
    x_shape, x_val = parse_shape_or_val(x_shape_or_val)
    y_shape, y_val = parse_shape_or_val(y_shape_or_val)
    t = concatenate([variable(x_val), variable(y_val)], axis)
    z = eval(t)
    return x_val, y_val, z


x, y, z = test_concatenate_operation((4, 3), (4, 2), axis=-1)
assert np.allclose(z, np.concatenate((x, y), axis=1))
