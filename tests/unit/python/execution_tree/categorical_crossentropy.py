#  Copyright (c) 2019-2020 Hartmut Kaiser
#  Copyright (c) 2019 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
from phylanx import Phylanx
import numpy as np


def softmax(x, axis=-1):
    y = np.exp(x - np.max(x, axis, keepdims=True))
    return y / np.sum(y, axis, keepdims=True)


def cat_cross(target, output, from_logits=False):
    if from_logits:
        output = softmax(output)
    else:
        v = output.sum(axis=-1, keepdims=True)
        output /= v
    output = np.clip(output, 1e-7, 1 - 1e-7)
    rep = -target * np.log(output)
    ans = np.sum(rep, axis=-1, keepdims=False)
    return ans


def cat_cross0(target, output, from_logits=False):
    if from_logits:
        output = softmax(output)
    else:
        v = output
        output /= v
    output = np.clip(output, 1e-7, 1 - 1e-7)
    ans = -target * np.log(output)
    return ans


# Quiet Flake8
def categorical_crossentropy(t, o, f):
    pass


@Phylanx
def cc(target, output, from_logits):
    return categorical_crossentropy(target, output, from_logits)


# Test 0d
for i in range(1, 10):
    for j in range(1, 10):
        for k in range(2):
            if k == 0:
                assert cc(i, j, True) == cat_cross0(i, j, True)
            else:
                assert cc(i, j, False) == cat_cross0(i, j, False)

# Test 1d
t = np.linspace(1.0e-9, 10e-9, 10)
t[2] = 10
o = 3 * np.ones(10)
o[4] = 2

flag = False
v1 = cc(t, o, flag)
v2 = cat_cross(t, o, flag)
assert np.all(np.abs(v1 - v2) < 1.0e-13), v1 - v2

t = np.linspace(1.0e-9, 10e-9, 10)
t[2] = 10
o = 3 * np.ones(10)
o[4] = 2

flag = True
v1 = cc(t, o, flag)
v2 = cat_cross(t, o, flag)
assert np.all(np.abs(v1 - v2) < 1.0e-13), v1 - v2

# Test 2d
flag = True
t = np.linspace(1, 12, 12)
t = np.reshape(t, (3, 4))
o = np.linspace(.01, .12, 12)
o = np.reshape(o, (3, 4))
v1 = cc(t, o, flag)
v2 = cat_cross(t, o, flag)
assert np.all(np.abs(v1 - v2) < 1.0e-13), v1 - v2

flag = False
t = np.linspace(1, 12, 12)
t = np.reshape(t, (3, 4))
o = np.linspace(.01, .12, 12)
o = np.reshape(o, (3, 4))
v1 = cc(t, o, flag)
v2 = cat_cross(t, o, flag)
assert np.all(np.abs(v1 - v2) < 1.0e-13), v1 - v2

flag = False
t = np.linspace(1, 24, 24)
t = np.reshape(t, (3, 4, 2))
o = np.linspace(.1 * 1 + .3, .1 * 24 + .3, 24)
o = np.reshape(o, (3, 4, 2))
v2 = cat_cross(t, o, flag)

flag = False
t = np.linspace(1, 24, 24)
t = np.reshape(t, (3, 4, 2))
o = np.linspace(.1 * 1 + .3, .1 * 24 + .3, 24)
o = np.reshape(o, (3, 4, 2))
v1 = cc(t, o, flag)

assert np.all(np.abs(v1 - v2) < 1.0e-13), v1 - v2

flag = True
t = np.linspace(1, 24, 24)
t = np.reshape(t, (3, 4, 2))
o = np.linspace(.1 * 1 + .3, .1 * 24 + .3, 24)
o = np.reshape(o, (3, 4, 2))
v2 = cat_cross(t, o, flag)

flag = True
t = np.linspace(1, 24, 24)
t = np.reshape(t, (3, 4, 2))
o = np.linspace(.1 * 1 + .3, .1 * 24 + .3, 24)
o = np.reshape(o, (3, 4, 2))
v1 = cc(t, o, flag)

assert np.all(np.abs(v1 - v2) < 1.0e-13), v1 - v2
