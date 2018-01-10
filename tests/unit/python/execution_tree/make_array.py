import phylanx

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
r = et.linspace(0,10,3)
cr = et.cross(r,r)
for i in range(cr.dimension(0)):
    assert cr.get(i) == 0

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
