#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #933: Can't pass eval_wrapper directly to a variable()

import numpy as np
from phylanx import Phylanx, PhylanxSession, execution_tree


PhylanxSession.init(1)


def dtype(x, dtype=None):
    from phylanx.ast.physl import PhySL
    if isinstance(x, execution_tree.variable):
        return x.dtype
    return execution_tree.variable(x, dtype).dtype


@Phylanx
def arange_eager(start, stop, step, dtype):
    return np.arange(start, stop=stop, step=step, dtype=dtype)


def arange(start, stop=None, step=1, dtype='int32'):
    return arange_eager.lazy(start, stop, step, dtype)


def test_dtype_2():
    for dt in ('int64', 'float64'):
        t = arange(10, dtype=dt)
        result = dtype(t)
        assert result == dt, (result, dt)
    for dt in ('int32', 'int64', 'float32', 'float64'):
        t = arange(10, dtype=dt)
        result = dtype(t, dtype=dt)
        assert result == dt, (result, dt)


test_dtype_2()
