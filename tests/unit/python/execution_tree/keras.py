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


def categorical_crossentropy(target, output, from_logits=False):
    if from_logits:
        output = softmax(output)
    else:
        output /= output.sum(axis=-1, keepdims=True)
    output = np.clip(output, 1e-7, 1 - 1e-7)
    rep = target * -np.log(output)
    return np.sum(rep, axis=-1, keepdims=False)


@Phylanx
def cc(target, output, from_logits):
    return categorical_crossentropy(target, output, from_logits)


# Generate some random data
t = np.linspace(1, 10, 10)
t[2] = 10
o = 3 * np.ones(10)
o[4] = 2

flag = False
v1 = cc(t, o, flag)
v2 = categorical_crossentropy(t, o, flag)
assert v1 == v2
