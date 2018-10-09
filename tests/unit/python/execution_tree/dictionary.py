from phylanx import Phylanx


@Phylanx
def mkdict():
    return {1: 2, 3: 4}


a = mkdict()

assert a[1] == 2
assert a[3] == 4


@Phylanx
def setdict():
    a = mkdict()
    a[1] = 5
    return a


a = setdict()

assert a[1] == 5


@Phylanx
def lookupdict():
    a = mkdict()
    return a[1]


assert lookupdict() == 2


@Phylanx
def newvalue():
    a = mkdict()
    a[2] = 42
    return a[2]


assert newvalue() == 42
