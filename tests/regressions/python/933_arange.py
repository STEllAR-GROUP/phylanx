#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #933: Can't pass eval_wrapper directly to a variable()

import numpy as np
import import_933_arange1 as K
import import_933_arange2 as KNP


WITH_NP = [K, KNP]


def test_arange():
    for test_value in (-20, 0, 1, 10):
        a_list = []
        dtype_list = []
        for k in WITH_NP:
            t = k.arange(test_value)
            a = k.eval(t)
            r = (a, np.arange(test_value), k, test_value)
            assert np.array_equal(a, np.arange(test_value)), r
            dtype_list.append(k.dtype(t))
            a_list.append(a)

        for i in range(len(a_list) - 1):
            assert np.array_equal(a_list[i], a_list[i + 1])

    for start, stop, step in ((0, 5, 1), (-5, 5, 2), (0, 1, 2)):
        a_list = []
        for k in WITH_NP:
            a = k.eval(k.arange(start, stop, step))
            r = (a, np.arange(start, stop, step))
            assert np.array_equal(a, np.arange(start, stop, step)), r
            a_list.append(a)
        for i in range(len(a_list) - 1):
            assert np.array_equal(a_list[i], a_list[i + 1])

    for dtype in ('int32', 'int64', 'float32', 'float64'):
        for k in WITH_NP:
            t = k.arange(10, dtype=dtype)
            assert k.dtype(t) == dtype, (k.dtype(t), dtype)

    start = K.constant(1, dtype='int32')
    t = K.arange(start)
    assert len(K.eval(t)) == 1

    start = K.constant(-1, dtype='int32')
    t = K.arange(start)
    assert len(K.eval(t)) == 0


test_arange()
