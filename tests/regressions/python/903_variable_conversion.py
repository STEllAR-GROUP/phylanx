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
    return execution_tree.variable(
        np.array(value, dtype=dtype), dtype=dtype, name=name)


def eval(func):
    return func.eval()


@Phylanx
def hard_sigmoid_eager(x):
    return hard_sigmoid(x)


def hard_sigmoid(x):
    return hard_sigmoid_eager.lazy(x)


def parse_shape_or_val(shape_or_val):
    return shape_or_val, np.random.random(shape_or_val).astype(np.float32) - 0.5


def test_hard_sigmoid_operation(x_shape_or_val):
    x_val = np.random.random(x_shape_or_val).astype(np.float32) - 0.5
    t = hard_sigmoid(variable(x_val))
    return eval(t)


print(test_hard_sigmoid_operation((4, 2)))
