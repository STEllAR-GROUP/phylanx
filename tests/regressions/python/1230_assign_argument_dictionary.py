#  Copyright (c) 2020 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #1230: Cannot assign to a dict if the dict is an arg

from phylanx import Phylanx


@Phylanx
def foo(a):
    a['e'] = 1
    return a


a = {}
result = foo(a)
assert result == {'e': 1}, result
