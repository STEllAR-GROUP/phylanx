#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #936: Add support for aggregation function to execution_tree.variable


import numpy as np
from phylanx import Phylanx, PhylanxSession, execution_tree


PhylanxSession.init(1)


_FLOATX = "float64"


def floatx():
    return _FLOATX


def variable(value, dtype=None, name=None, constraint=None):
    if dtype is None:
        dtype = floatx()
    if isinstance(value, execution_tree.variable):
        return value
    return execution_tree.variable(value, dtype=dtype, name=name)


def eval(x):
    return x.eval()


@Phylanx
def greater_equal_eager(x, y):
    return x >= y


def greater_equal(x, y):
    return greater_equal_eager.lazy(x, y)


@Phylanx
def where_eager(condition, then_expression, else_expression):
    return where(condition, then_expression, else_expression)


def where(condition, then_expression, else_expression):
    return where_eager.lazy(condition, then_expression, else_expression)


def test_switch(val):
    x = variable(val)
    x = variable(where(greater_equal(x, 0.5), x * 0.1, x * 0.2))
    return eval(-x)


v = np.random.random()
expected = v * 0.1 if v >= 0.5 else v * 0.2
r = test_switch(v)
assert r == -expected, (r, -expected)
