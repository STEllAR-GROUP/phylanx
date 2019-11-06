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
        epsilon = 1e-7
        output = np.clip(output, epsilon, 1 - epsilon)
        bce = target * np.log(output + epsilon)
        bce += (1 - target) * np.log(1 - output + epsilon)
        return -bce
    return (target * -np.log(sigmoid(output)) + \
            (1 - target) * -np.log(1 - sigmoid(output)))


# Quiet flake8
def binary_crossentropy(target, output, from_logits):
    pass


@Phylanx
def bc(target, output, from_logits):
    li = binary_crossentropy(target, output, from_logits)
    return li[0]


def allnan(v):
    return np.all(np.isnan(v))


def fixnan(v):
    v[np.isnan(v)] = 666.666


dimslist = [(10,), (4, 5), (3, 4, 2)]

if have_keras:
    server = tf.train.Server.create_local_server()
    sess = tf.Session(server.target)

    for dims in dimslist:
        for flag in [True, False]:
            good = 0
            while good < 10:
                t = np.random.random(dims)
                o = np.random.random(dims)

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

# Test 1d, 2d, 3d
for dims in dimslist:
    for flag in [True, False]:
        good = 0
        while good < 10:
            t = np.random.random(dims)
            o = np.random.random(dims)
            v1 = bc(t, o, flag)
            v2 = bin_cross(t, o, flag)
            if allnan(v1) and allnan(v2):
                continue
            fixnan(v1)
            fixnan(v2)
            assert np.all(np.abs(v1 - v2) < 1.0e-13), v1 - v2
            good += 1
