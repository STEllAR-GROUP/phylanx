# dot product as a phlyanx dag
#
import phylanx as p

def dotprod(a, b):
   # a is a p.vector(float) or p.ndarray(float)
   # b is a p.vector(float) or p.ndarray(float)
   # a * b creates an element-wise multiplication node
   # p.reduce creates a addition reduction node
   assert(a.shape == b.shape)
   c = p.reduce(a * b)
   return c

if __name__ == "__main__":
   N = 10
   a = p.vector(float, N, 1.0)
   b = p.ndarray(float, N, 1.0)

   A = dotprod(a)
   p.compute(A)

   B = dotprod(b)
   p.compute(B)

