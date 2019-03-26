#  Copyright (c) 2019 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# 896: Returned `nil`

from phylanx import Phylanx


@Phylanx
def return_none(x):
    return x


result = return_none(None)
assert result == None, result  # noqa
