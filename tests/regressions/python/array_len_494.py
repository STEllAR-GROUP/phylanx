#  Copyright (c) 2018 Weile Wei
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx.ast import *
import numpy as np


@Phylanx
def phylanx_len_test1(input_para):
    return len(input_para)


@Phylanx
def phylanx_len_test2(input_para):
    local_input_para = input_para
    return len(local_input_para)


def python_len_test(input_para):
    return len(input_para)


a = np.array([1])

assert(python_len_test(a) == phylanx_len_test1(a))
assert(python_len_test(a) == phylanx_len_test2(a))

b = np.array([1, 2, 3])

assert(python_len_test(b) == phylanx_len_test1(b))
assert(python_len_test(b) == phylanx_len_test2(b))

c = np.array([[1, 2, 3], [4, 5, 6]])

assert(python_len_test(c) == phylanx_len_test1(c))
assert(python_len_test(c) == phylanx_len_test2(c))
