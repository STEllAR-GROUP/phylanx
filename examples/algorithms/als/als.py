# Copyright (c) 2017 Shahrzad Shirzad
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from numpy import genfromtxt
import argparse
import time


def slice_array(a, row_start, row_stop, col_start, col_stop):
    return a[row_start:row_stop, col_start:col_stop]


def ALS(ratings, regularization, num_factors, iterations, alpha, enable_output):
    num_users = np.shape(ratings)[0]
    num_items = np.shape(ratings)[1]

    conf = alpha * ratings
    pref = ratings.copy()
    threshold_condition = pref > 0
    pref[threshold_condition] = 1

    conf_u = np.zeros((num_items, 1))
    conf_i = np.zeros((num_users, 1))

    mse = np.zeros((iterations, 1))
    mse_average = np.zeros((iterations, 1))

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
        while u < num_users:
            conf_u = conf[u, :]
            c_u = np.diag(conf_u)
            p_u = conf_u.copy()
            p_u[p_u != 0] = 1
            A = YtY + np.dot(np.dot(np.transpose(Y), c_u), Y)
            b = np.dot(np.dot(np.transpose(Y), c_u + I_i), np.transpose(p_u))
            X[u, :] = np.dot(np.linalg.inv(A), b)
            u = u + 1
        XtX = np.dot(np.transpose(X), X) + regularization * I_f
        while i < num_items:
            conf_i = conf[:, i]
            c_i = np.diag(conf_i)
            p_i = conf_i.copy()
            p_i[p_i != 0] = 1
            A = XtX + np.dot(np.dot(np.transpose(X), c_i), X)
            b = np.dot(np.dot(np.transpose(X), c_i + I_u), np.transpose(p_i))
            Y[i, :] = np.dot(np.linalg.inv(A), b)
            i = i + 1
        mse[k] = regularization *
                        (np.sum(np.power(X, 2)) + np.sum(np.power(Y, 2)))
        mse[k] += np.sum(np.multiply(conf,
                            np.power((pref - np.dot(X, np.transpose(Y))), 2)))
        mse_average[k] = mse[k]/(num_users * num_items)
        u = 0
        i = 0
        k = k + 1
    return [X, Y, mse, mse_average]


if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='Non-distributed ALS')
    parser.add_argument('-i', '--iterations', type=int, default=3,
                        help='number of iterations')
    parser.add_argument('-f', '--num_factors', type=int, default=10,
                        help='number of factors')
    parser.add_argument('--row_start', type=int, default=0,
                        help='row_start')
    parser.add_argument('--row_stop', type=int, default=10,
                        help='row_sop')
    parser.add_argument('--col_start', type=int, default=0,
                        help='col_start')
    parser.add_argument('--col_stop', type=int, default=20,
                        help='col_sop')
    parser.add_argument('-r', '--regularization', type=float, default=0.1,
                        help='regularization')
    parser.add_argument('-a', '--alpha', type=float, default=40,
                        help='alpha')
    parser.add_argument('-e', '--enable_output', type=bool, default=False,
                        help='enable_output')
    parser.add_argument('--data_csv', required=True,
                        help='file name for reading data')
    args = parser.parse_args()

    data_csv = genfromtxt(args.data_csv, delimiter=',')
    ratings = slice_array(data_csv, args.row_start, args.row_stop,
                          args.col_start, args.col_stop)

    t0 = time.perf_counter()

    result = ALS(ratings, args.regularization, args.num_factors,
                 args.iterations, args.alpha, args.enable_output)

    t1 = time.perf_counter() - t0

    print("X = ", result[0])
    print("Y = ", result[1])
    print("training loss = ", result[2])
    print("average training loss = ", result[3])
    print("elapsed time is: ", t1)
