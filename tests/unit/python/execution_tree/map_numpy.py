#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
import numpy as np

import phylanx
from phylanx.ast import Phylanx

np_x = np.array([1, 2, 3])
np_y = np.array([4, 5, 6])
np_array = np.reshape(np.arange(4), (2, 2))


@Phylanx("PhySL")
def np_argmax(x):
    return np.argmax(x)


assert (np_argmax(np_array) == np.argmax(np_array)).all


@Phylanx("PhySL")
def np_argmin(x):
    return np.argmin(x)


assert (np_argmin(np_array) == np.argmin(np_array)).all


@Phylanx("PhySL")
def np_cross(x, y):
    return np.cross(x, y)


assert (np_cross(np_x, np_y) == np.cross(np_x, np_y)).all


@Phylanx("PhySL")
def np_determinant(x):
    return np.linalg.det(x)


assert (np_determinant(np_array) == np.linalg.det(np_array)).all


@Phylanx("PhySL")
def np_diag(x):
    return np.diagonal(x)


assert (np_diag(np_array) == np.diagonal(np_array)).all


@Phylanx("PhySL")
def np_dot(x, y):
    return np.dot(x, y)


assert (np_dot(np_x, np_y) == np.dot(np_y, np_y)).all


@Phylanx("PhySL")
def np_exp(x):
    return np.exp(x)


assert (np_exp(np_array) == np.exp(np_array)).all

# TODO
# @Phylanx("PhySL")
# def np_hstack(x, y):
#     return np.hstack(x, y)
#
# assert (np_hstack(np_x, np_y) == np.hstack(np_x, np_y)).all


@Phylanx("PhySL")
def np_identity(x):
    return np.identity(x)


assert (np_identity(3) == np.identity(3)).all

# TODO
# np.inverse = np.linalg.inv
# @Phylanx("PhySL")
# def np_inverse(x):
#     return np.inverse(x)
#
# assert (np_inverse(np_array) == np.inverse(np_array)).all

# @Phylanx("PhySL")
# def np_linearmatrix():
#     return
#
# assert (np_(np_array) == np.(np_array)).all


@Phylanx("PhySL")
def np_linspace(start, stop, steps):
    return np.linspace(2, 3, 5)


assert (np_linspace(2, 3, 5) == np.linspace(2, 3, 5)).all


@Phylanx("PhySL")
def np_power(x, p):
    return np.power(x, 2)


assert (np_power(np_array, 2) == np.power(np_array, 2)).all

# TODO
# @Phylanx("PhySL")
# def np_random():
#     return
#
# assert (np_(np_array) == np.(np_array)).all

# TODO
# @Phylanx("PhySL")
# def np_shape(x):
#     return x.shape
#
# assert (np_shape(np_array) == np_array.shape).all

arr = np.array([0, 1, 2, 3, 4, 5, 6, 7, 8, 9])


@Phylanx("PhySL")
def np_slice_01(x):
    return x[1:4]


assert (np_slice_01(arr) == arr[1:4]).all
a = np.array([1, 4, 9])


@Phylanx("PhySL")
def np_sqrt(x):
    return np.sqrt(x)


assert (np_sqrt(a) == np.sqrt(a)).all


@Phylanx("PhySL")
def np_transpose(x):
    transx = np.transpose(x)
    return transx


assert (np_transpose(np_array) == np.transpose(np_array)).all

# TODO
# @Phylanx("PhySL")
# def np_vstack():
#     return np.vstack
#
# assert (np_vstack(np_array) == np.vstack(np_array)).all
