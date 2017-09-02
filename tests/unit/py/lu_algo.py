# algorithm provided by http://math.nist.gov/javanumerics/jama/
#
import phylanx as p

def lu_decomp(A, B):
  m = A.shape[0] # row
  n = A.shape[1] # col
  LU = A.copy() # deep copy of p.matrix into LU

  # phylanx problem context
  # makes computing easier to 
  # think about
  #
  c = A.context()
  piv = c.array(m)
  for i in range(m):
    piv[i] = m

  pivsign = c.scalar(1)
  lurowi = c.array(n)
  lucolj = c.array(m)

  for j in range(n):
    for i in range(m):
      lucolj[i] = A[i,j]

    for i in range(m):
      lurowi = A[i,:]

      kmax = min(i,j)

      s = c.scalar(0.0)

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

    pp = c.scalar(j)
    for i in range(j+1, m):
      def set(p, i):
        p = i
        return p
      pp.conditional(pp.abs(lucolj[i]) > pp.abs(lucolj[pp]), set(pp,i))
      pp.conditional(pp != j, compute_piv(A, n, pp, j, k, piv))

    if j < m:
       LU.conditional( (LU[i,j] != 0.0), compute_multipliers(LU, j, m) )

  nx = B.shape[1] # col dim
  Bmat = B[piv, 1, nx]

  for k in range(n):
    for (i, j) in zip(range(k), range(nx)):
      Bmat[k,j] /= LU[k,k] 
    for (i, j) in zip(range(k+1, n), range(nx)):
      Bmat[i,j] -= Bmat[k,j]*LU[i,k]

  return c, Bmat, LU

def get_l(LU):
  m = LU.shape[0] # row
  n = LU.shape[1] # col
  c = LU.context()
  L = c.matrix(float, m, n)

  for (i,j) in zip(range(m), range(n)):
    if i > j:
      L[i,j] = LU[i,j]
    elif i == j:
      L[i,j] = 1.0
    else
      L[i,j] = 0.0

   return c, L

def get_u(LU):
  m = LU.shape[0] # row
  n = LU.shape[1] # col
  c = LU.context()
  U = c.matrix(float, n, n)

  for (i,j) in zip(range(n), range(n)):
    if i<=j:
      U[i,j] = LU[i,j]
    else:
      U[i,j] = 0.0

  return U

if __name__ == "__main__":

  c = p.context()
  A = p.matrix(float, 100,100)
  p.random(A)

  Bmat, LU = lu_decomp(A)
  L = get_l(LU)
  U = get_u(LU)

  [ v.write(p.stdout) for v in [ Bmat, LU, L, U] ]

  p.compute(c)
  
