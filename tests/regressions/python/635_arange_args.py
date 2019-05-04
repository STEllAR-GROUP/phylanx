#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from phylanx import Phylanx


@Phylanx
def test_arange_float():
    arange_float = np.arange(3, dtype='float')
    return arange_float


r = test_arange_float()
assert r.dtype.name == 'float64', r.dtype.name
assert (r == np.arange(3, dtype='float64')).all()


@Phylanx
def test_arange_int():
    arange_int = np.arange(1, 3, dtype='int64')
    return arange_int


r = test_arange_int()
assert r.dtype.name == 'int64', r.dtype.name
assert (r == np.arange(1, 3, dtype='int64')).all()


@Phylanx
def test_cumsum_float(a):
    return np.cumsum(a, dtype=float)


a = np.array([[1.5, 2, 3], [4, 5, 6]])
r = test_cumsum_float(a)
assert r.dtype.name == 'float64', r.dtype.name
assert (r == np.cumsum(a, dtype=float)).all()


@Phylanx
def test_cumsum_int(a):
    return np.cumsum(a, dtype='int')


a = np.array([[1, 2, 3], [4, 5, 6]])
r = test_cumsum_int(a)
assert r.dtype.name == 'int64', r.dtype.name
assert (r == np.cumsum(a, dtype=int)).all()


@Phylanx
def test_stack_bool():
    return np.array([[True], [False], [True]], dtype=bool)


r = test_stack_bool()
assert r.dtype.name == 'bool', r.dtype.name
assert (r == np.array([[True], [False], [True]], dtype=bool)).all()


@Phylanx
def test_stack_float():
    return np.array([[1], [2], [3]], dtype=float)


r = test_stack_float()
assert r.dtype.name == 'float64', r.dtype.name
assert (r == np.array([[1], [2], [3]], dtype=float)).all()


@Phylanx
def test_stack_int():
    return np.array([[1], [2], [3]], dtype='int64')


r = test_stack_int()
assert r.dtype.name == 'int64', r.dtype.name
assert (r == np.array([[1], [2], [3]], dtype='int64')).all()


@Phylanx
def test_zeros():
    return np.zeros((3, 3), dtype='int64')


r = test_zeros()
assert r.dtype.name == 'int64', r.dtype.name
assert (r == np.zeros((3, 3), dtype='int64')).all()


@Phylanx
def test_ones():
    return np.ones((3, 3), dtype=float)


r = test_ones()
assert r.dtype.name == 'float64', r.dtype.name
assert (r == np.ones((3, 3), dtype=float)).all()
