# Copyright (c) 2018 Parsa Amini
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Fixing #410: Different results from numpy when performing arithmetic
# operations between a vector and a matrix with only one column

import numpy as np
from phylanx import Phylanx, PhylanxSession

PhylanxSession(1)

###############################################################################
# Add
###############################################################################


@Phylanx
def add_op(lhs, rhs):
    return lhs + rhs


def compare_add_with_numpy(l, r):
    lhs = np.array(l)
    rhs = np.array(r)

    expected = lhs + rhs
    actual = add_op(lhs, rhs)
    return np.array_equal(expected, actual)


# 1d-1d
###############################################################################
# (1)+(1)
assert compare_add_with_numpy([1.], [2.])
# (1)+(2)
assert compare_add_with_numpy([1.], [2., 3.])
# (2)+(1)
assert compare_add_with_numpy([1., 2.], [3.])
# 1d-2d
###############################################################################
# (1)+(1x1)
assert compare_add_with_numpy([1.], [[2.]])
# (1)+(1x2)
assert compare_add_with_numpy([3.], [[1., 2.]])
# (2)+(1x1)
assert compare_add_with_numpy([1., 2.], [[3.]])
# (2)+(1x2)
assert compare_add_with_numpy([3., 3.], [[1., 2.]])
# (2)+(2x1)
assert compare_add_with_numpy([1., 2.], [[1.], [2.]])
# 2d-1d
###############################################################################
# (1x2)+(2)
assert compare_add_with_numpy([[1., 2.]], [1., 2.])
# (1x2)+(1)
assert compare_add_with_numpy([[1., 2.]], [3.])
# 2d-2d
###############################################################################
# (2x1)+(1x2)
assert compare_add_with_numpy([[1.], [2.]], [[1., 2.]])
# (1x2)+(2x1)
assert compare_add_with_numpy([[1., 2.]], [[1.], [2.]])
# (1x2)+(1x1)
assert compare_add_with_numpy([[1., 2.]], [[3.]])
# (1x1)+(1x2)
assert compare_add_with_numpy([[3.]], [[1., 2.]])
# (1x1)+(1x2)
assert compare_add_with_numpy([[3.]], [[1., 2.]])
# (2x1)+(1x1)
assert compare_add_with_numpy([[1.], [2.]], [[3.]])
# (1x1)+(2x1)
assert compare_add_with_numpy([[3.]], [[1.], [2.]])

###############################################################################
# Subtract
###############################################################################


@Phylanx
def sub_op(lhs, rhs):
    return lhs - rhs


def compare_sub_with_numpy(l, r):
    lhs = np.array(l)
    rhs = np.array(r)

    expected = lhs - rhs
    actual = sub_op(lhs, rhs)
    return np.array_equal(expected, actual)


# 1d-1d
###############################################################################
# (1)-(1)
assert compare_sub_with_numpy([1.], [2.])
# (1)-(2)
assert compare_sub_with_numpy([1.], [2., 3.])
# (2)-(1)
assert compare_sub_with_numpy([1., 2.], [3.])
# 1d-2d
###############################################################################
# (1)-(1x1)
assert compare_sub_with_numpy([1.], [[2.]])
# (1)-(1x2)
assert compare_sub_with_numpy([3.], [[1., 2.]])
# (2)-(1x1)
assert compare_sub_with_numpy([1., 2.], [[3.]])
# (2)-(1x2)
assert compare_sub_with_numpy([3., 3.], [[1., 2.]])
# (2)-(2x1)
assert compare_sub_with_numpy([1., 2.], [[1.], [2.]])
# 2d-1d
###############################################################################
# (1x2)-(2)
assert compare_sub_with_numpy([[1., 2.]], [1., 2.])
# (1x2)-(1)
assert compare_sub_with_numpy([[1., 2.]], [3.])
# 2d-2d
###############################################################################
# (2x1)-(1x2)
assert compare_sub_with_numpy([[1.], [2.]], [[1., 2.]])
# (1x2)-(2x1)
assert compare_sub_with_numpy([[1., 2.]], [[1.], [2.]])
# (1x2)-(1x1)
assert compare_sub_with_numpy([[1., 2.]], [[3.]])
# (1x1)-(1x2)
assert compare_sub_with_numpy([[3.]], [[1., 2.]])
# (1x1)-(1x2)
assert compare_sub_with_numpy([[3.]], [[1., 2.]])
# (2x1)-(1x1)
assert compare_sub_with_numpy([[1.], [2.]], [[3.]])
# (1x1)-(2x1)
assert compare_sub_with_numpy([[3.]], [[1.], [2.]])

###############################################################################
# Multiply
###############################################################################


@Phylanx
def mul_op(lhs, rhs):
    return lhs * rhs


def compare_mul_with_numpy(l, r):
    lhs = np.array(l)
    rhs = np.array(r)

    expected = lhs * rhs
    actual = mul_op(lhs, rhs)
    return np.array_equal(expected, actual)


# 1d-1d
###############################################################################
# (1)*(1)
assert compare_mul_with_numpy([1.], [2.])
# (1)*(2)
assert compare_mul_with_numpy([1.], [2., 3.])
# (2)*(1)
assert compare_mul_with_numpy([1., 2.], [3.])
# 1d-2d
###############################################################################
# (1)*(1x1)
assert compare_mul_with_numpy([1.], [[2.]])
# (1)*(1x2)
assert compare_mul_with_numpy([3.], [[1., 2.]])
# (2)*(1x1)
assert compare_mul_with_numpy([1., 2.], [[3.]])
# (2)*(1x2)
assert compare_mul_with_numpy([3., 3.], [[1., 2.]])
# (2)*(2x1)
assert compare_mul_with_numpy([1., 2.], [[1.], [2.]])
# 2d-1d
###############################################################################
# (1x2)*(2)
assert compare_mul_with_numpy([[1., 2.]], [1., 2.])
# (1x2)*(1)
assert compare_mul_with_numpy([[1., 2.]], [3.])
# 2d-2d
###############################################################################
# (2x1)*(1x2)
assert compare_mul_with_numpy([[1.], [2.]], [[1., 2.]])
# (1x2)*(2x1)
assert compare_mul_with_numpy([[1., 2.]], [[1.], [2.]])
# (1x2)*(1x1)
assert compare_mul_with_numpy([[1., 2.]], [[3.]])
# (1x1)*(1x2)
assert compare_mul_with_numpy([[3.]], [[1., 2.]])
# (1x1)*(1x2)
assert compare_mul_with_numpy([[3.]], [[1., 2.]])
# (2x1)*(1x1)
assert compare_mul_with_numpy([[1.], [2.]], [[3.]])
# (1x1)*(2x1)
assert compare_mul_with_numpy([[3.]], [[1.], [2.]])

###############################################################################
# Divide
###############################################################################


@Phylanx
def div_op(lhs, rhs):
    return lhs / rhs


def compare_div_with_numpy(l, r):
    lhs = np.array(l)
    rhs = np.array(r)

    expected = lhs / rhs
    actual = div_op(lhs, rhs)
    return np.array_equal(expected, actual)


# 1d-1d
###############################################################################
# (1)*(1)
assert compare_div_with_numpy([1.], [2.])
# (1)*(2)
assert compare_div_with_numpy([1.], [2., 3.])
# (2)*(1)
assert compare_div_with_numpy([1., 2.], [3.])
# 1d-2d
###############################################################################
# (1)*(1x1)
assert compare_div_with_numpy([1.], [[2.]])
# (1)*(1x2)
assert compare_div_with_numpy([3.], [[1., 2.]])
# (2)*(1x1)
assert compare_div_with_numpy([1., 2.], [[3.]])
# (2)*(1x2)
assert compare_div_with_numpy([3., 3.], [[1., 2.]])
# (2)*(2x1)
assert compare_div_with_numpy([1., 2.], [[1.], [2.]])
# 2d-1d
###############################################################################
# (1x2)*(2)
assert compare_div_with_numpy([[1., 2.]], [1., 2.])
# (1x2)*(1)
assert compare_div_with_numpy([[1., 2.]], [3.])
# 2d-2d
###############################################################################
# (2x1)*(1x2)
assert compare_div_with_numpy([[1.], [2.]], [[1., 2.]])
# (1x2)*(2x1)
assert compare_div_with_numpy([[1., 2.]], [[1.], [2.]])
# (1x2)*(1x1)
assert compare_div_with_numpy([[1., 2.]], [[3.]])
# (1x1)*(1x2)
assert compare_div_with_numpy([[3.]], [[1., 2.]])
# (1x1)*(1x2)
assert compare_div_with_numpy([[3.]], [[1., 2.]])
# (2x1)*(1x1)
assert compare_div_with_numpy([[1.], [2.]], [[3.]])
# (1x1)*(2x1)
assert compare_div_with_numpy([[3.]], [[1.], [2.]])
