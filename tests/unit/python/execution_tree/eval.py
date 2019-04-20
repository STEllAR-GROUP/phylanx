#  Copyright (c) 2017-2018 Hartmut Kaiser
#  Copyright (c) 2018 Steven R. Brandt
#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx
from phylanx import Phylanx, PhylanxSession, execution_tree
import numpy as np

PhylanxSession.init(1)

et = phylanx.execution_tree
cs = et.compiler_state(__name__)

fib10 = et.eval(cs, """
block(
    define(fib,n,
    if(n<2,n,
        fib(n-1)+fib(n-2))),
    fib)""", 10)

assert fib10 == 55.0

sum10 = et.eval(cs, """
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

assert sum10 == 55.0


@Phylanx
def fib(n):
    if n < 2:
        return n
    else:
        return fib(n - 1) + fib(n - 2)


assert fib.__src__ == \
    'define$44$0(fib$44$0, n$44$8, if$45$4(__lt$45$7(n$45$7, 2), n$46$15, __add$48$15(fib$48$15(__sub$48$19(n$48$19, 1)), fib$48$28(__sub$48$32(n$48$32, 2)))))' # noqa E501
assert "[" + fib.__src__ + "]" == str(fib.generate_ast())
assert fib(10) == 55.0


@Phylanx
def pass_str(a):
    return a


assert pass_str.__src__ == \
    'define$58$0(pass_str$58$0, a$58$13, a$59$11)'
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


@Phylanx
def foo4():
    return foo()


assert foo4() == 3


@Phylanx
def f1():
    a = [1, 2, 3, 4]
    return a[1]


assert f1() == 2


@Phylanx
def f2():
    a = [1, 2, 3, 4]
    return a[1:3]


assert np.all(f2() == np.array([2, 3]))


@Phylanx
def f3():
    a = [1, 2, 3, 4]
    return a[:-1]


assert np.all(f3() == np.array([1, 2, 3]))

np_a = np.array([[1, 2], [3, 4]])


@Phylanx
def f4(a):
    return a[1, 1]


assert f4(np_a) == 4.0

np_a = np.array([[1, 2, 3], [4, 5, 6], [7, 8, 9]])


@Phylanx
def f5(a):
    return a[:2, 1:3]


assert np.all(f5(np_a) == np.array([[2, 3], [5, 6]]))

b = np.array([[1, 2, 3], [4, 5, 6], [7, 8, 9]])

np_b = np.array([[1, 2, 3], [4, 5, 6], [7, 8, 9]])


@Phylanx
def f6(b):
    return b[0:1, 0]


assert np.all(f6(np_b) == np.array(np_b[0:1, 0]))


@Phylanx
def f7(b):
    c = b[0, 0:2]
    return c


assert np.all(f7(np_b) == np.array(b[0, 0:2]))


@Phylanx
def pair():
    return (1, 2)


assert [1, 2] == pair()


@Phylanx
def do_sign(n):
    return np.sign(n)


@Phylanx
def do_square(n):
    return np.square(n)


assert np.sign(2) == do_sign(2)
assert np.sign(-3) == do_sign(-3)
assert np.square(2) == do_square(2)
assert np.square(-3) == do_square(-3)

v = np.array([2, 0, -3])
assert np.all(np.sign(v) == do_sign(v))
assert np.all(np.square(v) == do_square(v))

v = np.array([[1, -2, 3], [4, 0, 6]])
assert np.all(np.sign(v) == do_sign(v))
assert np.all(np.square(v) == do_square(v))

v = np.array([
    [[1, 2], [3, 4]],
    [[-5, 6], [-7, 8]],
    [[9, 0], [1, 2]]
])
assert np.all(np.sign(v) == do_sign(v))
assert np.all(np.square(v) == do_square(v))


def phytype(x):
    return execution_tree.variable(x).dtype


def get_types():
    return [
        phytype(None),
        phytype(0),
        phytype(1.0),
        phytype("x"),
        phytype(np.array([True, False])),
        phytype(np.array([1, 2], dtype='int16')),
        phytype(np.array([1, 2], dtype='int32')),
        phytype(np.array([1, 2], dtype='int64')),
        phytype(np.array([1.0, 2.0], dtype='float32')),
        phytype(np.array([1.0, 2.0])),
        phytype([1.0, "x"]),
        phytype({"x": 1})]


types = get_types()
assert types == [
    np.dtype('O'),
    np.dtype('int64'),
    np.dtype('float64'),
    np.dtype('S'),
    np.dtype('bool'),
    np.dtype('int16'),
    np.dtype('int32'),
    np.dtype('int64'),
    np.dtype('float32'),
    np.dtype('float64'),
    np.dtype('O'),
    np.dtype('O')], types
