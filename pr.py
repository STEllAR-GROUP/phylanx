import sys
from phylanx import Phylanx
import numpy as np
from random import random
import subprocess as s

@Phylanx
def find_pi():
    sum = 0.0
    sum_in = 0.0
    for i in range(100):
        x = randone()
        y = randone()
        r = x**2+y**2
        if r < 1:
            sum_in += 1.0
        sum += 1.0
    return 4*sum_in/sum

find_pi_src = find_pi.get_physl_source()

@Phylanx
def par(txt):
    f = find_all()
    n = f[-1]+1
    a = np.zeros(n)
    print("a:",a)
    for k in prange(n):
        a[k] = remote_compile(k, txt, True)
    s = 0
    print(a)
    for k in range(n):
        s += a[k]
    s /= n
    print(s)

par_src = par.get_physl_source()

with open("x.p","w") as fd:
    print('define(find_pi_str,"%s")' % find_pi_src,file=fd)
    print('define(par,%s)' % par_src,file=fd)
    print('par(find_pi_str)',file=fd)

if 1 < len(sys.argv):
    procs = sys.argv[1]
else:
    procs = "3"
proc = s.Popen(["mpirun","-np",procs,"build/bin/physl","--hpx:t=2","x.p"])
proc.communicate()
