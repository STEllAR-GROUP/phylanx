#  Copyright (c) 2017 Hartmut Kaiser
#  Copyright (c) 2018 R. Tohid
#  Copyright (c) 2018 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
import phylanx
from phylanx.util import phyfun, phy_print
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

@phyfun
def fib(n):
    if n < 2:
        return n
    else:
        return fib(n-1)+fib(n-2)

assert fib(10)[0] == 55.0

@phyfun
def pass_str(a):
    return a

assert "foo" == str(pass_str("foo"))

@phyfun
def test_slice(a):
    return a[1:3,1:4]

two = np.reshape(np.arange(30),(5,6))
r1 = test_slice(two)
r2 = two[1:3,1:4]

assert r2.shape == (r1.dimension(0), r1.dimension(1))
for i in range(r2.shape[0]):
    for j in range(r2.shape[1]):
        assert r1[i,j] == r2[i,j]
