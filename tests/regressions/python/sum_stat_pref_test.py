from phylanx import *
from phylanx.ast import *

import numpy as np
from time import time

def py_run(y, k):
    x = np.zeros(k)
    for i in range(k):
        x[y[i]] += 1.0

@Phylanx
def phy_run(y, k):
    x = constant(0.0, k)
    for i in range(k):
        x[y[i]] += 1.0

if __name__ == "__main__":
    K = 100000
    y = np.random.randint(0, K, size=K)

    func = [ py_run, phy_run ]
    func_nom = [ 'py_run', 'phy_run' ]

    for n, f in zip(func_nom, func):
        print(n)
        s = time()
        f(y, K)
        e = time()
        print(e-s)
