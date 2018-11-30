# Copyright (c) 2018 Christopher Taylor
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
'''
prange primitive taken from numba and reflected into hpat
allows users to explicitly define a parallel range to process
over - might be a nice place to add in hpx-smart-executor logic

https://github.com/numba/numba/blob/master/numba/special.py
'''


class prange(object):
    def __init__(self, *args):
        # remember range(0, n) iteration space
        # remember range(0, n, k) iteration broken into chunks
        #
        self.iterspace = range(*args)

    def __iter__(self):
        for i in self.iterspace:
            yield i

    def __next__(self):
        for i in self.iterspace:
            yield i
