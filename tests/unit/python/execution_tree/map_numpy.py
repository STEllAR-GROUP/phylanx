#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
import numpy as np

import phylanx
from phylanx import Phylanx

np_x = np.array([1, 2, 3])
np_y = np.array([4, 5, 6])
np_array = np.reshape(np.arange(4), (2, 2))


@Phylanx
def np_argmax(x):
    return np.argmax(x)


assert (np_argmax(np_array) == np.argmax(np_array)).all
assert np_argmax.get_physl_source() == \
    'define$16$0(np_argmax$16$0, x$16$14, argmax$17$11(x$17$21))'


@Phylanx
def np_argmin(x):
    return np.argmin(x)


assert (np_argmin(np_array) == np.argmin(np_array)).all
assert np_argmin.get_physl_source() == \
    'define$26$0(np_argmin$26$0, x$26$14, argmin$27$11(x$27$21))'


@Phylanx
def np_cross(x, y):
    return np.cross(x, y)


assert (np_cross(np_x, np_y) == np.cross(np_x, np_y)).all
assert np_cross.get_physl_source() == \
    'define$36$0(np_cross$36$0, x$36$13, y$36$16, cross$37$11(x$37$20, y$37$23))'


@Phylanx
def np_determinant(x):
    return np.linalg.det(x)


assert (np_determinant(np_array) == np.linalg.det(np_array)).all
assert np_determinant.get_physl_source() == \
    'define$46$0(np_determinant$46$0, x$46$19, determinant$47$11(x$47$25))'


@Phylanx
def np_diag(x):
    return np.diagonal(x)


assert (np_diag(np_array) == np.diagonal(np_array)).all
assert np_diag.get_physl_source() == \
    'define$56$0(np_diag$56$0, x$56$12, diag$57$11(x$57$23))'


@Phylanx
def np_dot(x, y):
    return np.dot(x, y)


assert (np_dot(np_x, np_y) == np.dot(np_y, np_y)).all
assert np_dot.get_physl_source() == \
    'define$66$0(np_dot$66$0, x$66$11, y$66$14, dot$67$11(x$67$18, y$67$21))'


@Phylanx
def np_exp(x):
    return np.exp(x)


assert (np_exp(np_array) == np.exp(np_array)).all
assert np_exp.get_physl_source() == \
    'define$76$0(np_exp$76$0, x$76$11, exp$77$11(x$77$18))'

# @Phylanx
# def np_hstack(x, y):
#     return np.hstack((x, y))
#
# assert (np_hstack(np_x, np_y) == np.hstack(np_x, np_y)).all


@Phylanx
def np_identity(x):
    return np.identity(x)


assert (np_identity(3) == np.identity(3)).all
assert np_identity.get_physl_source() == \
    'define$92$0(np_identity$92$0, x$92$16, identity$93$11(x$93$23))'

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
assert np_linspace.get_physl_source() == \
    'define$116$0(np_linspace$116$0, start$116$16, stop$116$23, steps$116$29, linspace$117$11(2, 3, 5))' # noqa E501


@Phylanx
def np_power(x, p):
    return np.power(x, 2)


assert (np_power(np_array, 2) == np.power(np_array, 2)).all
assert np_power.get_physl_source() == \
    'define$126$0(np_power$126$0, x$126$13, p$126$16, power$127$11(x$127$20, 2))'

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

result = np_slice_01.get_physl_source()
assert result == \
    "define$153$0(np_slice_01$153$0, x$153$16, " + \
    "slice$154$11(x$154$11, list(1, 4)))", \
    result

a = np.array([1, 4, 9])


@Phylanx
def np_sqrt(x):
    return np.sqrt(x)


result = np_sqrt.get_physl_source()
assert (np_sqrt(a) == np.sqrt(a)).all
assert result == \
    'define$169$0(np_sqrt$169$0, x$169$12, sqrt$170$11(x$170$19))', result


@Phylanx
def np_transpose(x):
    transx = np.transpose(x)
    return transx


result = np_transpose.get_physl_source()
assert (np_transpose(np_array) == np.transpose(np_array)).all
assert result == \
    'define$180$0(np_transpose$180$0, x$180$17, block(' + \
    'define$181$4(transx$181$4, transpose$181$13(x$181$26)), ' + \
    'transx$182$11))', result

# @Phylanx
# def np_vstack((np_x, np_y)):
#     return np.vstack((np_x, np_y))
#
# assert (np_vstack((np_x, np_y)) == np.vstack((np_x, np_y))).all

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
