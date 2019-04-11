#  Copyright (c) 2019-2020 Hartmut Kaiser
#  Copyright (c) 2019 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
from phylanx import Phylanx
import numpy as np
from math import isnan

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


def sigmoid(x):
    return 1 / (1 + np.exp(-x))


def bin_cross(target, output, from_logits=False):
    if not from_logits:
        output = np.clip(output, 1e-7, 1 - 1e-7)
        output = np.log(output / (1 - output))
    return (target * -np.log(sigmoid(output)) + \
            (1 - target) * -np.log(1 - sigmoid(output)))


# Quiet flake8
def binary_crossentropy(target, output, from_logits):
    pass


@Phylanx
def bc(target, output, from_logits):
    li = binary_crossentropy(target, output, from_logits)
    return li[0]


# Test 0d
for k in [True, False]:
    good = 0
    while good < 10:
        vals = np.random.random(2) * 10
        x = vals[0]
        y = vals[1]
        v1 = bc(x, y, k)
        v2 = bin_cross(x, y, k)
        if isnan(v1) and isnan(v2):
            continue
        assert v1 == v1, "%f != %f" % (v1, v2)
        good += 1


def allnan(v):
    return np.all(np.isnan(v))


def fixnan(v):
    v[np.isnan(v)] = 666.666


# Test 1d
for flag in [True, False]:
    good = 0
    while good < 10:
        t = np.random.random(10) * 10
        o = np.random.random(10) * 10
        v1 = bc(t, o, flag)
        v2 = bin_cross(t, o, flag)
        if allnan(v1) and allnan(v2):
            continue
        fixnan(v1)
        fixnan(v2)
        assert np.all(np.abs(v1 - v2) < 1.0e-13), v1 - v2
        good += 1

# Test 2d
for flag in [True, False]:
    good = 0
    while good < 10:
        t = np.random.random((3, 4)) * 10
        o = np.random.random((3, 4)) * 10
        v1 = bc(t, o, flag)
        v2 = bin_cross(t, o, flag)
        if allnan(v1) and allnan(v2):
            continue
        fixnan(v1)
        fixnan(v2)
        assert np.all(np.abs(v1 - v2) < 1.0e-13), v1 - v2
        good += 1

# Test 3d
for flag in [True, False]:
    good = 0
    while good < 10:
        t = np.random.random((3, 4, 2)) * 10
        o = np.random.random((3, 4, 2)) * 10
        v1 = bc(t, o, flag)
        v2 = bin_cross(t, o, flag)
        if allnan(v1) and allnan(v2):
            continue
        fixnan(v1)
        fixnan(v2)
        assert np.all(np.abs(v1 - v2) < 1.0e-13), v1 - v2
        good += 1

if have_keras:
    server = tf.train.Server.create_local_server()
    sess = tf.Session(server.target)

    for flag in [True, False]:
        good = 0
        while good < 10:
            t = np.random.random((3,))
            o = np.random.random((3,))

            kt = K.constant(np.copy(t), dtype=np.float64)
            ko = K.constant(np.copy(o), dtype=np.float64)

            v1 = K.binary_crossentropy(kt, ko, from_logits=flag)
            v1 = v1.eval(session=sess)
            v2 = bin_cross(t, o, flag)

            if allnan(v1) and allnan(v2):
                continue
            fixnan(v1)
            fixnan(v2)
            assert np.all(np.abs(v1 - v2) < 1.0e-13)
            good += 1
