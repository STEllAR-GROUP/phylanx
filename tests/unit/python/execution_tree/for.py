#  Copyright (c) 2018 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
from phylanx.util import *

@phyfun
def sumn():
    n = 0
    sum = 0
    for n in range(4):
        sum += n
    return sum

assert sumn()[0] == 6

@phyfun
def sumn():
    n = 0
    sum = 0
    for n in range(1,4):
        sum += n
    return sum

assert sumn()[0] == 6

@phyfun
def sumn():
    n = 0
    sum = 0
    c = 0
    for n in range(3,0,-1):
        sum += n
        c += 1
    return sum+c

assert sumn()[0] == 9

@phyfun
def sumn():
    n = 0
    sum = 0
    c = 0
    ss = -1
    for n in xrange(3,0,ss):
        sum += n
        c += 1
    return sum+c

assert sumn()[0] == 9
