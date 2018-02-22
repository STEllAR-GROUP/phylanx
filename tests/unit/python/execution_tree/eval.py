#  Copyright (c) 2017-2018 Hartmut Kaiser
#  Copyright (c) 2018 Steven R. Brandt
#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx
from phylanx.ast import *
from phylanx.ast.utils import printout

et = phylanx.execution_tree
import numpy as np

fib10 = et.eval("""
block(
    define(fib,n,
    if(n<2,n,
        fib(n-1)+fib(n-2))),
    fib)""",et.var(10))

assert fib10[0] == 55.0

sum10 = et.eval("""
block(
    define(sum10,
        block(
            define(i,0),
            define(n,0),
            while(n < 10,
                block(
                  store(n,n+1),
                  store(i,i+n)
                )),
            i)),
    sum10)""")

assert sum10[0] == 55.0

@Phylanx("PhySL")
def fib(n):
    if n < 2:
        return n
    else:
        return fib(n-1)+fib(n-2)

assert fib.__physl_src__ == \
    'block$1$0(define$1$0(fib$1$0, n$1$8, ' + \
        'if(n$2$7 < 2, n$3$15, ' + \
            '(fib((n$5$19 - 1)) + fib((n$5$28 - 2))))' + \
        '), fib$1$0)\n'

assert fib(10)[0] == 55.0

@Phylanx("PhySL")
def pass_str(a):
    return a

assert pass_str.__physl_src__ == \
    'block$1$0(define$1$0(pass_str$1$0, a$1$13, a$2$11), pass_str$1$0)\n'

assert "foo" == str(pass_str("foo"))

@Phylanx("PhySL")
def test_slice(a):
    return a[1:3,1:4]

two = np.reshape(np.arange(30),(5,6))
r1 = test_slice(two)
r2 = two[1:3,1:4]

assert r1 == r2

@Phylanx("PhySL")
def test_slice1(a):
    return a[2:4]

v1 = np.arange(10)
assert test_slice1(v1) == v1[2:4]
