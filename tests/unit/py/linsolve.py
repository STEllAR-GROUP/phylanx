# Code samples taken from...
#
# http://www.math.umbc.edu/~campbell/Computers/Python/linalg.html#LinAlg-SolveSys

from phylanx import *

# X is a phylanx matrix
#
def gauss_elim1(X):
   row = X.shape[0] # return row size
   col = X.shape[1] # return col size
   for i in range(col):
      for j in range(i+1, row):
         m = X[j, i]/ X[i, i]
         for k in range(col):
            X[j, k] = X[j, k] - m * X[i, k]

   return X

# X is a phylanx matrix
#
def gauss_elim2(X): # Basic row pivoting
   m = X.shape[0]
   n = X.shape[1]

   def compute_pivot(X, m, n, j):
      tmpMat = matrix(j,m)
      for k in range(j,m):
         tmpMat[k,j] = X[k,j]

      ipivot = tmpMat.index(tmpMat.max()) # find index of value 
      tmp = X[j,:] # row select
      X[j,:] = X[ipivot,:]
      X[ipivot, :] = tmp

      # returns result 'end point'/'sink' of computational graph
      return X

   for j in range(min(n,m)):  # for each column on the main diag

      # original code
      #
      #if(X[j,j] == 0): # find a non-zero pivot and swap rows
      #   tmpMat = matrix(j,m)
      #   for k in range(j,m):
      #      tmpMat[k,j] = X[k,j]
      #   ipivot = tmpMat.index(tmpMat.max())
      #   tmp = X[j,:]
      #   X[j,:] = X[ipivot,:]
      #   X[ipivot,:] = tmp
      #
      # phylanx version
      #

      # should if be the name of this method on a matrix node?
      # 
      X.conditional(X[j,j] == 0.0, compute_pivot(X,m, n, j));

      for i in range(j+1,m):
         c = X[i,j]/X[j,j]   # ratio of (i,j) elt by (j,j) (diagonal) elt
         for k in range(n):
            X[i] = X[i,k]-c*X[j,k]

   # returns result or 'end point'/'sink' of the computational graph
   return X 

if __name__ == "__main__":
   A = matrix(float, 100, 100)
   A = randomize(A)
   A = gauss_elim1(A)

  
   B = matrix(float, 100, 100)
   B = randomize(B)
   B = guass_elim2(B)

