#  Copyright (c) 2017 Hartmut Kaiser
#  Copyright (c) 2018 R. Tohid
#  Copyright (c) 2018 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
import phylanx
import numpy as np
from phylanx.ast import Phylanx

# Create a vector
x0 = 0
nx = 11
dx = 1.0
et = phylanx.execution_tree
var = et.var(np.linspace(x0, (nx - 1) * dx, nx))
r = var.eval()
assert isinstance(r, np.ndarray)
assert len(r) == nx
sum2 = 0
for i in range(11):
    sum2 += i * i
    assert x0 + dx * i == r[i]

# Test creation from a python array
va = [1, 2, 3, 4, 5]
vx = et.var(va).eval()
for i in range(len(va)):
    assert vx[i] == va[i]

# Create a vector that's all zeros
r = et.var(np.zeros(nx)).eval()
assert isinstance(r, np.ndarray)
assert len(r) == nx
for i in range(11):
    assert 0 == r[i]

# Create a matrix that's all zeros
nx = 5
ny = 4
m = et.var(np.zeros((nx, ny))).eval()
assert isinstance(m, np.ndarray)
assert len(m) == nx
assert len(m[1]) == ny
for i in range(nx):
    for j in range(ny):
        assert m[i][j] == 0

m2 = [[1.0, 2.0, 3.0], [1.1, 2, 2, 3.2], [1.5, 2.6, 3.7]]
m3 = et.var(m2).eval()
for i in range(len(m2)):
    for j in range(len(m2[0])):
        assert m3[i][j] == m2[i][j]


@Phylanx
def mk1():
    return np.array((3, 2, 4))


assert np.all(np.array((3, 2, 4)) == mk1())


@Phylanx
def mk2():
    return np.array([3, 2, 4])


assert np.all(np.array([3, 2, 4]) == mk2())


@Phylanx
def mk3():
    return np.zeros((3, 2))


assert np.all(np.zeros((3, 2)) == mk3())


@Phylanx
def mk4():
    return np.zeros([3, 2])


assert np.all(np.zeros([3, 2]) == mk4())
