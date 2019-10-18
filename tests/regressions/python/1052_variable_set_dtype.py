#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# 1052: Add support assignment to a variable's dtype

import numpy as np
from phylanx import PhylanxSession, execution_tree


PhylanxSession.init(1)


def variable(value, dtype=None, name=None):
    return execution_tree.variable(
        np.array(value, dtype=dtype), dtype=dtype, name=name)


def dtype_(x):
    return execution_tree.variable(x).dtype


def cast(x, dtype=None):
    x.dtype = np.dtype(dtype)
    return x


def test_dtype():
    # 'float16' is not supported (yet)
    # assert dtype_(variable(1, dtype='float16')) == 'float16'
    assert dtype_(cast(variable(1), dtype='float64')) == 'float64'
    assert dtype_(cast(variable(1), dtype='float32')) == 'float32'
    assert dtype_(cast(variable(1.0), dtype='int32')) == 'int32'
    assert dtype_(cast(variable(1.0), dtype='int64')) == 'int64'


test_dtype()
