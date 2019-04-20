#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from phylanx import Phylanx, PhylanxSession, execution_tree


PhylanxSession.init()


def variable(value, dtype=None):
    return execution_tree.variable(np.array(value, dtype), dtype)


@Phylanx
def dot_eager(x, y):
    return np.dot(x, y)


def dot(x, y):
    return dot_eager.lazy(x, y)


def eval(func):
    return func.eval()


def parse_shape_or_val(shape_or_val):
    return shape_or_val, np.random.random(shape_or_val).astype(np.float32) - 0.5


def test_dot_operation(x_shape_or_val, y_shape_or_val):
    x_shape, x_val = parse_shape_or_val(x_shape_or_val)
    y_shape, y_val = parse_shape_or_val(y_shape_or_val)
    t = dot(variable(x_val), variable(y_val))
    z = eval(t)
    return z, np.dot(x_val, y_val)


result, expected = test_dot_operation((4, 2), (2, 4))
assert np.allclose(result, expected), (result, expected)
