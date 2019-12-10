#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Fixing #409: Cannot create lists of arbitrary objects

import phylanx
from phylanx import Phylanx


@Phylanx
def f():
    return [1, [1]]


assert f.get_physl_source() == \
    "define$13$0(f$13$0, lambda$13$0(list$14$11(1, list$14$15(1))))", \
    f.get_physl_source()
