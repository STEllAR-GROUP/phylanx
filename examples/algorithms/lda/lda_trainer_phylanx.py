#  Copyright (c) 2020 Christopher Taylor
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Algorithm from the following references
#
# D. Newman, A. Asuncion, P. Smyth, M. Welling.
#   "Distributed Algorithms for Topic Models." JMLR 2009.
# D. Newman, A. Asuncion, P. Smyth, M. Welling.
#   "Distributed Inference for Latent Dirichlet Allocation." NIPS 2007.
#


from random import randint, random as rnd
from numpy import zeros, ones
import numpy as np
from phylanx import Phylanx
from phylanx import *
from phylanx.ast import *


@Phylanx
def gibbs(wdc, a, b, z, wp, dp, ztot):
    W = wdc.shape[1]
    D = wdc.shape[0]
    T = ztot.shape[0]

    wbeta = float(T) * b

    lhs = zeros(T)
    rhs = zeros(T)
    zprob = zeros(T)
    probs = zeros(T)

    n = 0

    for d in range(D):
        for w in range(W):

            wdf = wdc[d, w]

            # skip over zero entries
            #
            if wdf < 1.0:
                continue

            F = 0
            for f in range(int(wdf)):
                F += f
                t = int(z[n])
                ztot[t] -= 1.0
                wp[w, t] -= 1.0
                dp[d, t] -= 1.0

                lhs = wp[w, :] + b
                rhs = dp[d, :] + a
                zprob = ztot[:] + wbeta
                probs = lhs * (rhs / zprob)

                # inspired by: https://www.youtube.com/watch?v=aHLslaWO-AQ
                #
                # maxprob = abs(drand.drand()) * max(probs) * 2.0
                # t = int(abs(drand.drand()) * float(T))
                maxprob = abs(rnd()) * max(probs) * 2.0
                t = int(abs(rnd()) * float(T))
                while maxprob > 1e-10:
                    maxprob -= probs[t]
                    t = ((t + 1) % T)

                z[n] = t
                ztot[t] += 1.0
                wp[w, t] += 1.0
                dp[d, t] += 1.0
                n += 1

    return (z, ztot, wp, dp)


@Phylanx
def lda_trainer(wdc, T, a=0.1, b=0.01, iters=500):
    alpha, beta = a, b
    D, W = wdc.shape[0], wdc.shape[1]
    N = int(np.sum(wdc))

    z = zeros(N)
    wp = zeros((W, T))
    dp = zeros((D, T))

    wfreq = np.sum(wdc, axis=0)
    dfreq = np.sum(wdc, axis=1)

    for n in range(N):
        z[n] = randint(0, T - 1)

    k = 0
    I = 0
    for i in range(W):
        I = int(wfreq[i])
        for j in range(I):
            wp[i, int(z[k])] += 1.0
            k += 1

    k = 0
    J = 0
    for i in range(D):
        for j in range(int(dfreq[i])):
            dp[i, int(z[k])] += 1.0
            k += 1
            J += j

    ztot0 = zeros(T)
    wp0 = zeros(wp.shape)

    for i in range(iters):
        wp0 = wp
        ztot0[:] = np.sum(wp, 0)
        res = gibbs(wdc, alpha, beta, z, wp, dp, ztot0)
        z = res[0]
        ztot0 = res[1]
        wp = res[2]
        dp = res[3]
        wp = wp0 + (wp - wp0)

    return (wp, dp, z)


if __name__ == "__main__":
    wdc = ones((1000, 100))
    res = lda_trainer(wdc, 3, iters=20)
    print(res)
