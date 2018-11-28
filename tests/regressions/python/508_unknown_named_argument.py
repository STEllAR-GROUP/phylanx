#  Copyright (c) 2018 Weile Wei
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Fixing #508: @Phylanx not complaining about unknown named arguments.

from phylanx import Phylanx, PhylanxSession

PhylanxSession(1)

try:
    @Phylanx(Foo=True)
    def func_1():
        pass
except NotImplementedError:
    pass


@Phylanx
def func_2():
    pass


@Phylanx(target='PhySL')
def func_3():
    pass


@Phylanx(target='PhySL', debug=True)
def func_4():
    pass
