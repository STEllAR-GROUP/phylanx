#  Copyright (c) 2018 Shahrzad Shirzad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx
from phylanx.ast import *
import numpy as np


@Phylanx
def ALS(filename, row_stop, col_stop, regularization, num_factors, iterations, alpha,
        enable_output):
    data = file_read_csv(filename)
    ratings = data[0:row_stop, 0:col_stop]
    num_users = shape(ratings, 0)
    num_items = shape(ratings, 1)
    conf = alpha * ratings
    conf_u = constant(0.0, make_list(num_items))
    conf_i = constant(0.0, make_list(num_users))
    c_u = constant(0.0, make_list(num_items, num_items))
    c_i = constant(0.0, make_list(num_users, num_users))
    p_u = constant(0.0, make_list(num_items))
    p_i = constant(0.0, make_list(num_users))
    set_seed(0)
    X = random(make_list(num_users, num_factors))
    Y = random(make_list(num_items, num_factors))
    I_f = identity(num_factors)
    I_i = identity(num_items)
    I_u = identity(num_users)
    i = 0
    u = 0
    k = 0
    XtX = constant(0.0, make_list(num_factors, num_factors))
    YtY = constant(0.0, make_list(num_factors, num_factors))
    A = constant(0.0, make_list(num_factors, num_factors))
    b = constant(0.0, make_list(num_factors))
    while k < iterations:
        if enable_output:
            print("iteration: ", k)
            print("X :", X)
            print("Y :", Y)
        YtY = dot(transpose(Y), Y) + regularization * I_f
        XtX = dot(transpose(X), X) + regularization * I_f
        while u < num_users:
            conf_u = slice_row(conf, u)
            c_u = diag(conf_u)
            p_u = conf_u != 0
            A = YtY + dot(dot(transpose(Y), c_u), Y)
            b = dot(dot(transpose(Y), c_u + I_i), transpose(p_u))
            set_row(X, u, u + 1, 1, dot(inverse(A), b))
            u = u + 1
        while i < num_items:
            conf_i = slice_column(conf, i)
            c_i = diag(conf_i)
            p_i = conf_i != 0
            A = XtX + dot(dot(transpose(X), c_i), X)
            b = dot(dot(transpose(X), c_i + I_u), transpose(p_i))
            set_row(Y, i, i + 1, 1, dot(inverse(A), b))
            i = i + 1
        u = 0
        i = 0
        k = k + 1
    return make_list(X, Y)


(a, b) = ALS("MovieLens.csv", 10, 10, 0.1, 3, 1, 40, True)
print(a)
print(b)
