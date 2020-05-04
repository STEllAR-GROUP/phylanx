#  Copyright (c) 2020 Christopher Taylor
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#

from random import random as rnd, randint
from numpy import *

# https://stackoverflow.com/questions/7287014/is-there-any-drand48-equivalent-in-python-or-a-wrapper-to-it/7287046#7287046
#
class Rand48(object):
    def __init__(self, seed):
        self.n = seed
    def seed(self, seed):
        self.n = seed
    def srand(self, seed):
        self.n = (seed << 16) + 0x330e
    def next(self):
        self.n = (25214903917 * self.n + 11) & (2**48 - 1)
        return self.n
    def drand(self):
        return self.next() / 2**48
    def lrand(self):
        return self.next() >> 17
    def mrand(self):
        n = self.next() >> 16
        if n & (1 << 31):
            n -= 1 << 32
        return n   

def gibbs(wdc, a, b, z, wp, dp, ztot):
    drand = Rand48(0)
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

            wdf = wdc[d,w]

            # skip over zero entries
            #
            if wdf < 1.0:
                continue

            for f in range(int(wdf)):
                t = int(z[n])
                ztot[t]-=1.0
                wp[w, t]-=1.0
                dp[d, t]-=1.0

                lhs[:] = wp[w,:] + b
                rhs[:] = dp[d,:] + a
                zprob[:] = ztot[:] + wbeta
                probs[:] = lhs * ( rhs / zprob )

                # algorithmic flaw, tends to oversample
                # from a single topic
                #
                '''
                maxprob = sum( probs ) * abs(drand.drand())
                #maxprob = 2.0 * max( probs ) * abs(drand.drand())
                curprobs = probs[0];

                t = 0
                while maxprob > curprobs:
                    t = ((t + 1) % T)
                    curprobs += probs[t]
                '''

                # inspired by: https://www.youtube.com/watch?v=aHLslaWO-AQ
                #
                '''
                maxprob = max( probs ) * abs(drand.drand()) * 2.0
                curprobs = 0.0

                t = 0
                while maxprob > curprobs:
                    curprobs += probs[t]
                    t = ((t + 1) % T)
                '''

                # inspired by: https://www.youtube.com/watch?v=aHLslaWO-AQ
                #
                maxprob = abs(drand.drand()) * max(probs) * 2.0
                t = int(abs(drand.drand()) * float(T))
                while maxprob > 1e-10:
                    maxprob -= probs[t]
                    t = ((t+1) % T)

                z[n] = t;
                ztot[t]+=1.0
                wp[w, t]+=1.0
                dp[d, t]+=1.0
                n+=1

    assert N == n, "N != n, (%d, %d)" % (N, n)
    return (z, ztot, wp, dp)

def lda_trainer(wdc, T, a=0.1, b=0.01, iters=500):
    alpha, beta = a, b
    D, W = wdc.shape[0], wdc.shape[1]
    N = int(sum(wdc))

    wbeta = float(W) * beta
    z = zeros(N)
    wp = zeros((W, T))
    dp = zeros((D, T))

    wfreq = sum(wdc, axis=0)
    dfreq = sum(wdc, axis=1)

    for n in range(N):
        z[n] = randint(0, T-1)
    
    k = 0
    for i in range(W):
        for j in range(int(wfreq[i])):
            wp[i, int(z[k])]+=1.0
            k+=1
    
    k = 0
    for i in range(D):
        for j in range(int(dfreq[i])):
            dp[i, int(z[k])]+=1.0
            k+=1

    ztot0 = zeros(T)
    wp0 = zeros(wp.shape)

    for i in range(iters):
        wp0[:,:] = wp[:,:]
        ztot0[:] = sum(wp,0)
        #print(z[4116])
        #print('pg', wp[0, :])
        res = gibbs(wdc, alpha, beta, z, wp, dp, ztot0)
        z[:] = res[0]
        ztot0[:] = res[1]
        wp[:] = res[2]
        #print('ag', wp[0, :])
        dp[:] = res[3]
        #print('*')
        #print(z[4116])
        #print()
        wp[:,:] = wp0 + (wp - wp0)
        #print(i)

    print(wp, dp, z)

if __name__ == "__main__":
    wdc = ones((1000,100))
    res = lda_trainer(wdc, 3, iters=20)
    print(res)

