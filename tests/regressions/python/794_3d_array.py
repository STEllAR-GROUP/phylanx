# Copyright (c) 2019 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx
import numpy as np


@Phylanx
def one_d():
    return np.array([1, 2])


@Phylanx
def two_d():
    return np.array([[1, 2], [3, 4]])


@Phylanx
def three_d():
    return np.array([[[1, 2], [3, 4]], [[5, 6], [7, 8]]])


@Phylanx
def one_d_float():
    return np.array([1, 2], dtype='float')


def np_one_d():
    return np.array([1, 2])


def np_two_d():
    return np.array([[1, 2], [3, 4]])


def np_three_d():
    return np.array([[[1, 2], [3, 4]], [[5, 6], [7, 8]]])


def np_one_d_float():
    return np.array([1, 2], dtype='float')


assert (one_d() == np_one_d()).all()
assert (two_d() == np_two_d()).all()
assert (three_d() == np_three_d()).all()
assert (one_d_float() == np_one_d_float()).all()
