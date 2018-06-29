# Copyright (c) 2018 Christopher Taylor
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import *
from phylanx.ast import *

import numpy as np
from time import time

def py_run(y, k):
    x = np.zeros(k)
    for i in range(k):
        x[y[i]] += 1.0

@Phylanx
def phy_run(y, k):
    x = constant(0.0, k)
    for i in range(k):
        x[y[i]] += 1.0

def py_run_mat(y, k):
    x = np.zeros((k, k))
    print(x.shape)
    for i in range(k):
        x[y[i], y[i]] += 1.0

@Phylanx
def phy_run_mat(y, k):
    x = constant(0.0, make_list(k, k))
    for i in range(k):
        x[y[i], y[i]] += 1.0

def py_run_self_update(y, k):
    x = np.zeros(k)
    for i in range(k):
        x[y[i]] = x[y[i]] + 1.0

@Phylanx
def phy_run_self_update(y, k):
    x = constant(0.0, k)
    for i in range(k):
        x[y[i]] = x[y[i]] + 1.0

def py_run_mat_self_update(y, k):
    x = np.zeros((k,k))
    for i in range(k):
        x[y[i], y[i]] = x[y[i], y[i]] + 1.0

@Phylanx
def phy_run_mat_self_update(y, k):
    x = constant(0.0, make_list(k, k))
    for i in range(k):
        x[y[i], y[i]] = x[y[i], y[i]] + 1.0


if __name__ == "__main__":
    K = 10000 # add a 0
    y = np.random.randint(0, K, size=K)

    func = [ py_run_mat
           , phy_run_mat
           , py_run
           , phy_run
           , py_run_self_update
           , phy_run_self_update
           , py_run_mat_self_update
           , phy_run_mat_self_update
    ]

    func_nom = [ 'py_run_mat'
               , 'phy_run_mat'
               , 'py_run'
               , 'phy_run'
               , 'py_run_self_update'
               , 'phy_run_self_update'
               , 'py_run_mat_self_update'
               , 'phy_run_mat_self_update'
    ]

    for n, f in zip(func_nom, func):
        print(n)
        s = time()
        f(y, K)
        e = time()
        print(e-s)
