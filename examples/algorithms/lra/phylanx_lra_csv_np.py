#  Copyright (c) 2018 Steven R. Brandt
#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
import phylanx
import numpy as np
from phylanx import *
from phylanx.ast import *


@Phylanx
def lra(x, y, alpha, iterations, enable_output):
    weights = constant(0.0, shape(x, 1))
    transx = np.transpose(x)
    pred = constant(0.0, shape(x, 0))
    error = constant(0.0, shape(x, 0))
    grad = constant(0.0, shape(x, 1))
    step = 0
    while step < iterations:
        if (enable_output):
            print("step: ", step, ", ", weights)
        pred = 1.0 / (1.0 + np.exp(-np.dot(x, weights)))
        error = pred - y
        grad = np.dot(transx, error)
        weights = weights - (alpha * grad)
        step += 1
    return weights


file_name = "../datasets/breast_cancer.csv"

data = np.genfromtxt(file_name, skip_header=1, delimiter=",")
x = data[:, :-1]
y = data[:, -1:]
y = np.reshape(y, (y.shape[0], ))
res = lra(x, y, 1e-5, 750, 0)
print(res)
