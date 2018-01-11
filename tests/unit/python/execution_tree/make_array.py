#  Copyright (c) 2017 Hartmut Kaiser
#  Copyright (c) 2018 R. Tohid
#  Copyright (c) 2018 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
import phylanx
from phylanx.util import *

# Create a vector
x0 = 0
nx = 11
dx = 1.0
et = phylanx.execution_tree
r = et.linspace(x0,(nx-1)*dx,nx)
assert r.num_dimensions()==1
assert r.dimension(0) == nx
sum2 = 0
for i in range(11):
    sum2 += i*i
    assert x0+dx*i == r.get(i)

# Test dot product
assert et.dot(r,r).get(0) == sum2

# Test cross product, need size 3
vx = et.var([1,0,0])
vy = et.var([0,1,0])
vz = et.var([0,0,1])
cr = et.cross(vx,vy)
for i in range(cr.dimension(0)):
    assert cr.get(i) == vz.get(i)

# Create a vector that's all zeros
r = et.zeros(nx)
assert r.num_dimensions()==1
assert r.dimension(0) == nx
for i in range(11):
    assert 0 == r.get(i)

# Create a matrix
nx = 5
ny = 4
x0 = 2
dx = 1
dy = 2
m = et.linearmatrix(nx,ny,x0,dx,dy)
assert m.num_dimensions()==2
assert m.dimension(0) == nx
assert m.dimension(1) == ny
for i in range(nx):
    for j in range(ny):
        assert m.get(i,j) == x0+i*dx+j*dy

# Create a matrix that's all zeros
m = et.zeros(nx,ny)
assert m.num_dimensions()==2
assert m.dimension(0) == nx
assert m.dimension(1) == ny
for i in range(nx):
    for j in range(ny):
        assert m.get(i,j) == 0

m2 = [
 [1.0,2.0,3.0],
 [1.1,2,2,3.2],
 [1.5,2.6,3.7]]
m3 = et.var(m2)
for i in range(len(m2)):
  for j in range(len(m2[0])):
    assert m3.get(i,j) == m2[i][j]
