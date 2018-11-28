#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from phylanx import Phylanx, PhylanxSession

PhylanxSession(1)


@Phylanx
def test_arange_float():
    arange_float = np.arange(3, dtype='float')
    return arange_float


assert (test_arange_float() == np.arange(3, dtype='int')).all


@Phylanx
def test_arange_int():
    arange_int = np.arange(1, 3, dtype='int')
    return arange_int


assert (test_arange_int() == np.arange(1, 3, dtype='int')).all


@Phylanx
def test_cumsum_float(a):
    return np.cumsum(a, dtype=float)


a = np.array([[1.5, 2, 3], [4, 5, 6]])
assert (test_cumsum_float(a) == np.cumsum(a, dtype=float)).all


@Phylanx
def test_cumsum_int(a):
    return np.cumsum(a, dtype='int')


a = np.array([[1, 2, 3], [4, 5, 6]])
assert (test_cumsum_float(a) == np.cumsum(a, dtype='int')).all


@Phylanx
def test_stack_bool():
    return np.array([[True], [False], [True]], dtype='bool')


assert (test_stack_bool() == np.array([[True], [False], [True]],
                                      dtype='bool')).all


@Phylanx
def test_stack_float():
    return np.array([[1], [2], [3]], dtype='float')


assert (test_stack_float() == np.array([[1], [2], [3]], dtype='float')).all


@Phylanx
def test_stack_int():
    return np.array([[1], [2], [3]], dtype=int)


assert (test_stack_int() == np.array([[1], [2], [3]], dtype=int)).all


@Phylanx
def test_zeros():
    return np.zeros((3, 3), dtype=int)


assert (test_zeros() == np.zeros((3, 3), dtype=int)).all


@Phylanx
def test_ones():
    return np.ones((3, 3), dtype=float)


assert (test_ones() == np.ones((3, 3), dtype=float)).all


@Phylanx
def test_ones_like():
    a = np.zeros((3, 3), dtype=int)
    return np.ones_like(a, dtype=int)


a = np.zeros((3, 3), dtype=int)
assert (test_ones_like() == np.ones_like(a, dtype=int)).all


@Phylanx
def test_zeros_like():
    a = np.ones((3, 3), dtype=int)
    return np.zeros_like(a, dtype=int)


a = np.ones((3, 3), dtype=int)
assert (test_zeros_like() == np.zeros_like(a, dtype=int)).all


@Phylanx
def test_full():
    return np.full((3, 3), 2, dtype=int)


assert (test_full() == np.full((3, 3), 2, dtype=int)).all


@Phylanx
def test_full_like():
    a = np.zeros((3, 3), dtype=int)
    return np.full_like(a, 42, dtype=int)


a = np.zeros((3, 3), dtype=int)
assert (test_full_like() == np.full_like(a, 42, dtype=int)).all
