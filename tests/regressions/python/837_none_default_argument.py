#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #837: Using 'None' as default argument fails

from phylanx import Phylanx
from phylanx.util import debug_output


def debug(s):   # silence flake
    pass


@Phylanx
def fx(shape, minval=None):
    debug(minval)
    return 0


fx(0)


s = debug_output()
assert s == "None\n", s
