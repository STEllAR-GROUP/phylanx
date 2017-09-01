# algorithm provided by http://math.nist.gov/javanumerics/jama/
#
import phylanx as p

def lu_decomp(A, B):
  m = A.shape[0] # row
  n = A.shape[1] # col
  LU = A.copy() # deep copy of p.matrix into LU

  piv = p.array(m)
  for i in range(m):
    piv[i] = m

  pivsign = p.scalar(1)
  lurowi = p.array(n)
  lucolj = p.array(m)

  for j in range(n):
    for i in range(m):
      lucolj[i] = A[i,j]

    for i in range(m):
      lurowi = A[i,:]

      kmax = min(i,j)

      s = p.scalar(0.0)

      for k in range(kmax):
        s += lurowi[k] * lucolj[k]

      lucolj[i] -= s;
      lurowi[j] = lucolj[i]

    def compute_piv(A, n, p, j, k, piv):
      for k in range(n):
        t = A[p,k]
        A[p, k] = A[j, k)
        A[j, k] = t

      k = piv[p]
      piv[p] = piv[j]
      piv[j] = k
      pivsign = -pivsign
      return piv, pivsign

    def compute_multipliers(A, j, m):
      for i in range(j+1, m):
        A[i,j] /= A[j, j]
      return A

    p = p.scalar(j)
    for i in range(j+1, m):
      p.conditional(p.abs(lucolj[i]) > p.abs(lucolj[p]), p = i)
      p.conditional(p != j, compute_piv(A, n, p, j, k, piv))

    LU.conditional( (LU[i,j] != 0.0) and (j < m), compute_multipliers(LU, j, m) )

  nx = B.shape[1] # col dim
  Bmat = B[piv, 1, nx]

  for k in range(n):
    for (i, j) in zip(range(k), range(nx)):
      Bmat[k,j] /= LU[k,k] 
    for (i, j) in zip(range(k+1, n), range(nx)):
      Bmat[i,j] -= Bmat[k,j]*LU[i,k]

  return Bmat, LU

def get_l(LU):
  m = LU.shape[0] # row
  n = LU.shape[1] # col
  L = p.matrix(m, n)

  for (i,j) in zip(range(m), range(n)):
    if i > j:
      L[i,j] = LU[i,j]
    elif i == j:
      L[i,j] = 1.0
    else
      L[i,j] = 0.0

   return L

def get_u(LU):
  m = LU.shape[0] # row
  n = LU.shape[1] # col
  U = p.matrix(n, n)

  for (i,j) in zip(range(n), range(n)):
    if i<=j:
      U[i,j] = LU[i,j]
    else:
      U[i,j] = 0.0

  return U

if __name__ == "__main__":

  A = p.matrix(100,100)
  p.random(A)

  Bmat, LU = lu_decomp(A)
  L = get_l(LU)
  U = get_u(LU)

  p.compute(Bmat, LU)
  p.compute(L)
  p.compute(U)
  
