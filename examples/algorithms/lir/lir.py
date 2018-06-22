# Copyright (c) 2018 Tianyi Zhang
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np


def MyLinearRegression(x, y, iterations, alpha):
    w = np.zeros(x.shape[1])
    Transx = x.transpose()
    for step in range(iterations):
        g = np.dot(x, w)
        error = g - y
        gradient = np.dot(Transx, error)
        w = w - alpha * gradient
    return w


X = np.array([[15.04, 16.74], [13.82, 24.49], [12.54, 16.32], [23.09, 19.83],
             [9.268, 12.87], [9.676, 13.14], [12.22, 20.04], [11.06, 17.12],
             [16.3, 15.7], [15.46, 23.95], [11.74, 14.69], [14.81, 14.7],
             [13.4, 20.52], [14.58, 13.66], [15.05, 19.07], [11.34, 18.61],
             [18.31, 20.58], [19.89, 20.26], [12.88, 18.22], [12.75, 16.7],
             [9.295, 13.9], [24.63, 21.6], [11.26, 19.83], [13.71, 18.68],
             [9.847, 15.68], [8.571, 13.1], [13.46, 18.75], [12.34, 12.27],
             [13.94, 13.17], [12.07, 13.44]])


Y = np.array([1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1,
              0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1])


weights = MyLinearRegression(X, Y, iterations=850, alpha=1e-4)
print(weights)
