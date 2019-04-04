#  Copyright (c) 2019-2020 Hartmut Kaiser
#  Copyright (c) 2019 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
from phylanx import Phylanx
import numpy as np

have_keras = False
try:
    import keras.backend as K
    import tensorflow as tf
    # Silence annoying tensorflow messages
    import os
    os.environ['TF_CPP_MIN_LOG_LEVEL'] = '8'
    have_keras = True
except Exception:
    pass


def softmax(x, axis=-1):
    y = np.exp(x - np.max(x, axis=axis, keepdims=True))
    return y / np.sum(y, axis=axis, keepdims=True)


def cat_cross(target, output, from_logits=False, axis=-1):
    if from_logits:
        axis = -1
        output = softmax(output, axis=axis)
    else:
        v = np.sum(output, axis=axis, keepdims=True)
        output /= v
    output = np.clip(output, 1e-7, 1 - 1e-7)
    rep = -target * np.log(output)
    ans = np.sum(rep, axis=axis, keepdims=False)
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
def cc(target, output, from_logits, axis):
    li = categorical_crossentropy(target, output, from_logits, axis)
    return li[0]


# Test 0d
for i in range(1, 10):
    for j in range(1, 10):
        for k in range(2):
            if k == 0:
                assert cc(i, j, True, -1) == cat_cross0(i, j, True)
            else:
                assert cc(i, j, False, -1) == cat_cross0(i, j, False)

# Test 1d
for flag in [True, False]:
    t = np.linspace(1.0e-9, 10e-9, 10)
    t[2] = 10
    o = 3 * np.ones(10)
    o[4] = 2

    v1 = cc(t, o, flag, -1)
    v2 = cat_cross(t, o, flag)
    assert np.all(np.abs(v1 - v2) < 1.0e-13), v1 - v2

    t = np.linspace(1.0e-9, 10e-9, 10)
    t[2] = 10
    o = 3 * np.ones(10)
    o[4] = 2

    v1 = cc(t, o, flag, -1)
    v2 = cat_cross(t, o, flag)
    assert np.all(np.abs(v1 - v2) < 1.0e-13), v1 - v2

# Test 2d
for axis in [-1, 0, 1]:
    for flag in [True, False]:
        t = np.linspace(1, 12, 12)
        t = np.reshape(t, (3, 4))
        o = np.linspace(.01, .12, 12)
        o = np.reshape(o, (3, 4))
        v1 = cc(t, o, flag, axis)
        v2 = cat_cross(t, o, flag, axis)
        assert v1.shape == v2.shape
        assert np.all(np.abs(v1 - v2) < 1.0e-13), v1 - v2

        t = np.linspace(1, 12, 12)
        t = np.reshape(t, (3, 4))
        o = np.linspace(.01, .12, 12)
        o = np.reshape(o, (3, 4))
        v1 = cc(t, o, flag, axis)
        v2 = cat_cross(t, o, flag, axis)
        assert np.all(np.abs(v1 - v2) < 1.0e-13), v1 - v2

# Test 3d
for axis in [-1, 0, 1, 2]:
    for flag in [True, False]:
        t = np.linspace(1, 24, 24)
        t = np.reshape(t, (3, 4, 2))
        o = np.linspace(.1 * 1 + .3, .1 * 24 + .3, 24)
        o = np.reshape(o, (3, 4, 2))
        v2 = cat_cross(t, o, flag, axis)

        t = np.linspace(1, 24, 24)
        t = np.reshape(t, (3, 4, 2))
        o = np.linspace(.1 * 1 + .3, .1 * 24 + .3, 24)
        o = np.reshape(o, (3, 4, 2))
        v1 = cc(t, o, flag, axis)

        assert np.all(np.abs(v1 - v2) < 1.0e-13), v1 - v2

if have_keras:
    server = tf.train.Server.create_local_server()
    sess = tf.Session(server.target)

    for axis in [-1, 0, 1, 2]:
        for flag in [True, False]:
            t = np.linspace(1, 24, 24)
            t = np.reshape(t, (3, 4, 2))
            o = np.linspace(.1 * 1 + .3, .1 * 24 + .3, 24)
            o = np.reshape(o, (3, 4, 2))
            v2 = cat_cross(t, o, flag, axis=axis)

            t = np.linspace(1, 24, 24)
            t = np.reshape(t, (3, 4, 2))
            o = np.linspace(.1 * 1 + .3, .1 * 24 + .3, 24)
            o = np.reshape(o, (3, 4, 2))
            kt = K.constant(t, dtype=np.float64)
            ko = K.constant(o, dtype=np.float64)
            v1 = K.categorical_crossentropy(kt, ko, from_logits=flag, axis=axis)
            v1 = v1.eval(session=sess)

            assert np.all(np.abs(v1 - v2) < 1.0e-13)
