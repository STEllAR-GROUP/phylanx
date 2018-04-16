#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
from phylanx.ast.oscop import *
from phylanx.ast.transformation import Phylanx

N = 10
A = [[0. for i in range(N + 1)] for j in range(N + 1)]
B = [[0. for i in range(N + 1)] for j in range(N + 1)]
C = [[0. for i in range(N + 1)] for j in range(N + 1)]

M = 2


# matrix multiply
@Phylanx("OpenSCoP")
def kernel():
    if N > 0:
        for i in range(N):
            for j in range(N):
                C[i][j] = 0.0
                for k in range(N):
                    C[i][j] = C[i][j] + A[i][k] * B[k][j]


print(kernel.__src__)
