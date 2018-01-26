#  Copyright (c) 2017 Chris Taylor
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# dot product as a phlyanx dag
#
import phylanx as p

def dotprod(a, b):
   # a is a p.vector(float) or p.ndarray(float)
   # b is a p.vector(float) or p.ndarray(float)
   # a * b creates an element-wise multiplication node
   # p.reduce creates a addition reduction node
   assert(a.shape == b.shape) # shape method returns n-ary tuples
   c = p.reduce(a * b)
   return c

if __name__ == "__main__":
   c = p.context()
   N = 10
   a = c.vector(float, N, 1.0)
   b = c.ndarray(float, N, 1.0)

   A = dotprod(a)
   A.write(p.stdout)
   B = dotprod(b)
   B.write(p.stdout)

   p.compute(c)

