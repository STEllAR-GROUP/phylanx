
import numpy as np
from phylanx import Phylanx, PhylanxSession, execution_tree


PhylanxSession.init(1)


_FLOATX = "float64"


def floatx():
    return _FLOATX


def variable(value, dtype=None, name=None, constraint=None):
    if dtype is None:
        dtype = floatx()
    from phylanx.ast.physl import PhySL
    if isinstance(value, PhySL.eval_wrapper):
        return execution_tree.variable(value.code(), dtype)
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


def test_switch():
    val = np.random.random()
    x = variable(val)
    x = variable(where(greater_equal(x, 0.5), x * 0.1, x * 0.2))
#    x += 42.0
    return eval(-x)


print(test_switch())
