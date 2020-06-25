from phylanx import Phylanx
import numpy as np


def mod_test():
    v = np.array([5, 8, 3, 7])
    v %= 2
    q = np.array([5, 8, 3, 7])
    q = 2 % q
    return [v, q]


mod_test2 = Phylanx(mod_test)
r1 = mod_test()
r2 = mod_test2()
for i in range(len(r1)):
    if not np.all(r1[i] == r2[i]):
        print("Differences found:", i)
        print(r1[i])
        print(r2[i])
        assert False
