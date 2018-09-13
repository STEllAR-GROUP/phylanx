# Copyright (c) 2017 Shahrzad Shirzad
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np


def ALS(ratings, regularization, num_factors, iterations, alpha, enable_output):
    num_users = np.shape(ratings)[0]
    num_items = np.shape(ratings)[1]

    conf = alpha * ratings
    conf_u = np.zeros((num_items, 1))
    conf_i = np.zeros((num_items, 1))

    c_u = np.zeros((num_items, num_items))
    c_i = np.zeros((num_users, num_users))
    p_u = np.zeros((num_items, 1))
    p_i = np.zeros((num_users, 1))

    I_f = np.identity(num_factors)
    I_i = np.identity(num_items)
    I_u = np.identity(num_users)

    np.random.seed(0)
    X = np.random.rand(num_users, num_factors)
    Y = np.random.rand(num_items, num_factors)

    I_f = np.identity(num_factors)
    I_i = np.identity(num_items)
    I_u = np.identity(num_users)

    i = 0
    u = 0
    k = 0

    XtX = np.zeros((num_factors, num_factors))
    YtY = np.zeros((num_factors, num_factors))
    A = np.zeros([num_factors, num_factors])
    b = np.zeros([num_factors])
    while k < iterations:
        if enable_output:
            print("iteration ", k)
            print("X: ", X)
            print("Y: ", Y)
        YtY = np.dot(np.transpose(Y), Y) + regularization * I_f
        XtX = np.dot(np.transpose(X), X) + regularization * I_f
        while u < num_users:
            conf_u = conf[u, :]
            c_u = np.diag(conf_u)
            p_u = conf_u.copy()
            p_u[p_u != 0] = 1
            A = YtY + np.dot(np.dot(np.transpose(Y), c_u), Y)
            b = np.dot(np.dot(np.transpose(Y), c_u + I_i), np.transpose(p_u))
            X[u, :] = np.dot(np.linalg.inv(A), b)
            u = u + 1
        while i < num_items:
            conf_i = conf[:, i]
            c_i = np.diag(conf_i)
            p_i = conf_i.copy()
            p_i[p_i != 0] = 1
            A = XtX + np.dot(np.dot(np.transpose(X), c_i), X)
            b = np.dot(np.dot(np.transpose(X), c_i + I_u), np.transpose(p_i))
            Y[i, :] = np.dot(np.linalg.inv(A), b)
            i = i + 1
        u = 0
        i = 0
        k = k + 1
    result = np.vstack((X, Y))
    return result


# test example
ratings = np.zeros((10, 5))
ratings[0, 1] = 4
ratings[1, 0] = 1
ratings[1, 2] = 4
ratings[1, 4] = 5
ratings[2, 3] = 2
ratings[3, 1] = 8
ratings[4, 2] = 4
ratings[6, 4] = 2
ratings[7, 0] = 1
ratings[8, 3] = 5
ratings[9, 0] = 1
ratings[9, 3] = 2

regularization = 0.1
alpha = 40
iterations = 5
num_factors = 3
enable_output = False

result = ALS(ratings, regularization, num_factors, iterations, alpha, enable_output)
print("X = ", result[0:np.shape(ratings)[0], :])
print("Y = ", result[0:np.shape(ratings)[1], :])
