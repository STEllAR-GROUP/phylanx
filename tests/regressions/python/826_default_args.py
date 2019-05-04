#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #826: Optional arguments don't work in Python front-end

from phylanx import Phylanx


@Phylanx
def fx(shape, minval=0.25, maxval=1.0):
    return [minval, maxval]


assert [0.25, 1.0] == fx(0)
