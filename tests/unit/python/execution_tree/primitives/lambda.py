# Copyright (c) 2018 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx
from phylanx.ast import *

import numpy as np


@Phylanx
def f(a, b):
    return map(lambda x, y: x * y, a, b)


assert f([1, 2], [3, 4]) == [3, 8]

assert(f(np.array([1, 2]), np.array([3, 4])) == np.array([3., 8.])).all()
assert(
    f(np.array([[1, 2], [3, 4]]), np.array([[5, 6], [7, 8]])) ==
    np.array([[5., 12.], [21., 32.]])).all()

assert(f(np.array([1., 2.]), np.array([3., 4.])) == np.array([3., 8.])).all()
assert(
    f(np.array([[1., 2.], [3., 4.]]), np.array([[5, 6], [7, 8]])) ==
    np.array([[5., 12.], [21., 32.]])).all()


@Phylanx
def f(a):
    return map(lambda x: x * 2, a)


assert f([1, 2]) == [2, 4]

assert(f(np.array([1, 2])) == np.array([2, 4])).all()
assert(f(np.array([[1, 2], [3, 4]])) == np.array([[2, 4], [6, 8]])).all()

assert(f(np.array([1., 2.])) == np.array([2., 4.])).all()
assert(
    f(np.array([[1., 2.], [3., 4.]])) ==
    np.array([[2., 4.], [6., 8.]])).all()


@Phylanx
def f(a):
    return map(lambda x: not x, a)


assert f([True, False]) == [False, True]

assert(f(np.array([True, False])) == np.array([False, True])).all()
assert(
    f(np.array([[True, False], [True, False]])) ==
    np.array([[False, True], [False, True]])).all()
