#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
import numpy as np

import phylanx
from phylanx import Phylanx, PhylanxSession

PhylanxSession(1)

np_x = np.array([1, 2, 3])
np_y = np.array([4, 5, 6])
np_array = np.reshape(np.arange(4), (2, 2))


@Phylanx
def np_argmax(x):
    return np.argmax(x)


assert (np_argmax(np_array) == np.argmax(np_array)).all
assert np_argmax.__src__ == \
    'define$18$0(np_argmax$18$0, x$18$14, argmax$19$11(x$19$21))'


@Phylanx
def np_argmin(x):
    return np.argmin(x)


assert (np_argmin(np_array) == np.argmin(np_array)).all
assert np_argmin.__src__ == \
    'define$28$0(np_argmin$28$0, x$28$14, argmin$29$11(x$29$21))'


@Phylanx
def np_cross(x, y):
    return np.cross(x, y)


assert (np_cross(np_x, np_y) == np.cross(np_x, np_y)).all
assert np_cross.__src__ == \
    'define$38$0(np_cross$38$0, x$38$13, y$38$16, cross$39$11(x$39$20, y$39$23))'


@Phylanx
def np_determinant(x):
    return np.linalg.det(x)


assert (np_determinant(np_array) == np.linalg.det(np_array)).all
assert np_determinant.__src__ == \
    'define$48$0(np_determinant$48$0, x$48$19, determinant$49$11(x$49$25))'


@Phylanx
def np_diag(x):
    return np.diagonal(x)


assert (np_diag(np_array) == np.diagonal(np_array)).all
assert np_diag.__src__ == \
    'define$58$0(np_diag$58$0, x$58$12, diag$59$11(x$59$23))'


@Phylanx
def np_dot(x, y):
    return np.dot(x, y)


assert (np_dot(np_x, np_y) == np.dot(np_y, np_y)).all
assert np_dot.__src__ == \
    'define$68$0(np_dot$68$0, x$68$11, y$68$14, dot$69$11(x$69$18, y$69$21))'


@Phylanx
def np_exp(x):
    return np.exp(x)


assert (np_exp(np_array) == np.exp(np_array)).all
assert np_exp.__src__ == \
    'define$78$0(np_exp$78$0, x$78$11, exp$79$11(x$79$18))'

# @Phylanx
# def np_hstack(x, y):
#     return np.hstack((x, y))
#
# assert (np_hstack(np_x, np_y) == np.hstack((np_x, np_y))).all


@Phylanx
def np_identity(x):
    return np.identity(x)


assert (np_identity(3) == np.identity(3)).all
assert np_identity.__src__ == \
    'define$94$0(np_identity$94$0, x$94$16, identity$95$11(x$95$23))'

# TODO
# np.inverse = np.linalg.inv
# @Phylanx
# def np_inverse(x):
#     return np.inverse(x)
#
# assert (np_inverse(np_array) == np.inverse(np_array)).all

# @Phylanx
# def np_linearmatrix():
#     return
#
# assert (np_(np_array) == np.(np_array)).all


@Phylanx
def np_linspace(start, stop, steps):
    return np.linspace(2, 3, 5)


assert (np_linspace(2, 3, 5) == np.linspace(2, 3, 5)).all
assert np_linspace.__src__ == \
    'define$118$0(np_linspace$118$0, start$118$16, stop$118$23, steps$118$29, linspace$119$11(2, 3, 5))' # noqa E501


@Phylanx
def np_power(x, p):
    return np.power(x, 2)


assert (np_power(np_array, 2) == np.power(np_array, 2)).all
assert np_power.__src__ == \
    'define$128$0(np_power$128$0, x$128$13, p$128$16, power$129$11(x$129$20, 2))'

# TODO
# @Phylanx
# def np_random():
#     return
#
# assert (np_(np_array) == np.(np_array)).all

# TODO
# @Phylanx
# def np_shape(x):
#     return x.shape
#
# assert (np_shape(np_array) == np_array.shape).all

arr = np.array([0, 1, 2, 3, 4, 5, 6, 7, 8, 9])


# TODO
@Phylanx
def np_slice_01(x):
    return x[1:4]


assert (np_slice_01(arr) == arr[1:4]).all
assert np_slice_01.__src__ == \
    'define$155$0(np_slice_01$155$0, x$155$16, slice$156$11(x$156$11, list(1, 4)))'

a = np.array([1, 4, 9])


@Phylanx
def np_sqrt(x):
    return np.sqrt(x)


assert (np_sqrt(a) == np.sqrt(a)).all
assert np_sqrt.__src__ == \
    'define$167$0(np_sqrt$167$0, x$167$12, sqrt$168$11(x$168$19))'


@Phylanx
def np_transpose(x):
    transx = np.transpose(x)
    return transx


assert (np_transpose(np_array) == np.transpose(np_array)).all
assert np_transpose.__src__ == \
    'define$177$0(np_transpose$177$0, x$177$17, block(define$178$4(transx$178$4, transpose$178$13(x$178$26)), transx$179$11))' # noqa E501

# @Phylanx
# def np_vstack(np_x, np_y):
#     return np.vstack((np_x, np_y))
#
# assert (np_vstack(np_x, np_y) == np.vstack((np_x, np_y))).all

# Please note that even if class::abc did have member function `shape` this code
# would not have worked as, at this point, Phylanx does not support not support
# any member function other than limited list of NumPy functions.
# TODO: This test does not belong here and must be move to more appropriate
# test!
try:

    # Quiet Flake8
    class abc:
        pass

    @Phylanx
    def bad_kernel(a):
        return abc.shape()

    raise Exception("Should fail because abc.shape doesn't exist")

except NotImplementedError:
    pass
