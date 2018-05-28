#  Copyright (c) 2017-2018 Hartmut Kaiser
#  Copyright (c) 2018 Steven R. Brandt
#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx
from phylanx.ast import Phylanx
import numpy as np

et = phylanx.execution_tree
cs = phylanx.compiler_state()

fib10 = et.eval("""
block(
    define(fib,n,
    if(n<2,n,
        fib(n-1)+fib(n-2))),
    fib)""", cs, 10)

assert fib10 == 55.0

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
    sum10)""", cs)

assert sum10 == 55.0


@Phylanx
def fib(n):
    if n < 2:
        return n
    else:
        return fib(n - 1) + fib(n - 2)


assert fib.__src__ == \
    'define$42$0(fib$42$0, n$42$8, if$43$4((n$43$7 < 2), n$44$15, (fib$46$15((n$46$19 - 1)) + fib$46$28((n$46$32 - 2)))))'  # noqa E501
assert "[" + fib.__src__ + "]" == str(fib.generate_ast())
assert fib(10) == 55.0


@Phylanx
def pass_str(a):
    return a


assert pass_str.__src__ == \
    'define$56$0(pass_str$56$0, a$56$13, a$57$11)'
assert "[" + pass_str.__src__ + "]" == str(pass_str.generate_ast())
assert "foo" == str(pass_str("foo"))


@Phylanx
def test_slice(a):
    return a[1:3, 1:4]


two = np.reshape(np.arange(30), (5, 6))
r1 = test_slice(two)
r2 = two[1:3, 1:4]

assert (r1 == r2).all()


@Phylanx
def test_slice1(a):
    return a[2:4]


v1 = np.arange(10)
assert (test_slice1(v1) == v1[2:4]).all()


@Phylanx
def foo():
    return 3


assert foo() == 3


@Phylanx
def foo2(n):
    if n == 1:
        return 2
    elif n == 3:
        return 4
    else:
        return 5


assert foo2(1) == 2 and foo2(3) == 4 and foo2(5) == 5


@Phylanx
def foo3():
    sumn = 0
    i = 0
    while i < 10:
        sumn += i
        i += 1
    return sumn


assert foo3() == 45


@Phylanx()
def foo4():
    return foo()


assert foo4() == 3


@Phylanx()
def f1():
    a = [1, 2, 3, 4]
    return a[1]


assert f1() == 2


@Phylanx()
def f2():
    a = [1, 2, 3, 4]
    return a[1:3]


assert np.all(f2() == np.array([2, 3]))


@Phylanx()
def f3():
    a = [1, 2, 3, 4]
    return a[:-1]


assert np.all(f3() == np.array([1, 2, 3]))

np_a = np.array([[1, 2], [3, 4]])


@Phylanx()
def f4(a):
    return a[1, 1]


assert f4(np_a) == 4.0

np_a = np.array([[1, 2, 3], [4, 5, 6], [7, 8, 9]])


@Phylanx()
def f5(a):
    return a[:2, 1:3]


assert np.all(f5(np_a) == np.array([[2, 3], [5, 6]]))

b = np.array([[1, 2, 3], [4, 5, 6], [7, 8, 9]])

np_b = np.array([[1, 2, 3], [4, 5, 6], [7, 8, 9]])


@Phylanx()
def f6(b):
    return b[0:1, 0]


assert np.all(f6(np_b) == np.array(np_b[0:1, 0]))


@Phylanx()
def f7(b):
    c = b[0, 0:2]
    return c


assert np.all(f7(np_b) == np.array(b[0, 0:2]))


# @Phylanx(debug=True)
# def f8():
#     a = [1, 2]
#     a[0] = 1
#     return a[0]
