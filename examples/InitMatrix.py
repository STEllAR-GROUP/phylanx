import phylanx
et = phylanx.execution_tree

def phy_print(m):
  ndim = m.num_dimensions()
  if ndim==1:
    for i in range(m.dimension(0)):
      print(m.get(i))
  elif ndim==2:
    for i in range(m.dimension(0)):
      for j in range(m.dimension(1)):
        print("%10.2f" % m.get(i,j),end=" ")
        if j > 5:
            print("...",end=" ")
            break
      print()
      if i > 5:
        print("%10s" % "...")
        break
  elif ndim==0:
    print(m.get(0))
  else:
    print("ndim=",ndim)

# Create a vector of zeros
vz = et.zeros(10)

phy_print(vz)

mz = et.zeros(3,4)

phy_print(mz)

v = et.linspace(2.0,3.4,5)

phy_print(v)

m = et.linearmatrix(3,4,9.0,1.2,.3)

phy_print(m)

print("Breast Cancer")

m = et.file_read_csv("./examples/algorithms/breast_cancer.csv")

phy_print(m)
