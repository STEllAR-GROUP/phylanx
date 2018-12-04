#  Copyright (c) 2018 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx
import re


@Phylanx
def recur1(n):
    while n < 2:
        n = n - 1
    return 5


try:
    @Phylanx
    def recur2(n):
        while n < 2:
            n = n - 1
        # Illegal return
        return 2
        return 5
    raise Exception("Test Failed")
except NotImplementedError as e:
    print(e)
    assert re.match('Illegal return', str(e))


@Phylanx
def recur3(n):
    if n < 2:
        return 1
    else:
        return 2


try:
    @Phylanx
    def recur4(n):
        k = 1
        # return in for loop illegal
        for i in range(10):
            if i == 4:
                return k
    raise Exception("Test Failed")
except NotImplementedError as e:
    assert re.match('Illegal return', str(e))


@Phylanx
def recur5(n):
    k = 1
    if k == 1:
        if n == 4:
            return n


try:
    @Phylanx
    def recur6(n):
        if n < 2:
            return 1
        # multiple returns illegal
        return 2
    raise Exception("Test Failed")
except NotImplementedError as e:
    assert re.match('Illegal return', str(e))


@Phylanx
def recur7(n):
    if n < 2:
        return 1
    else:
        return 2


try:
    @Phylanx
    def recur8(n):
        k = 1
        i = 5
        while n > 0:
            n = n - 1
            if i == 4:
                # return from while loop is illegal
                return k
    raise Exception("Test Failed")
except NotImplementedError as e:
    assert re.match('Illegal return', str(e))


try:
    @Phylanx
    def recur9(n):
        while n > 0:
            n = n - 1
            if n == 4:
                # return from while loop is illegal
                return n
    raise Exception("Test Failed")
except NotImplementedError as e:
    assert re.match('Illegal return', str(e))


@Phylanx
def recur10(n):
    if n == 66:
        return 66
    elif n == 2:
        return 2
    elif n == 3:
        return 4
    else:
        return 8


@Phylanx
def recur11(n):
    if n == 66:
        return 66
    elif n == 2:
        return 2
    elif n == 3:
        return 4
    else:
        return 8
