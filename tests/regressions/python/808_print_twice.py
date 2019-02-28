#  Copyright (c) 2019 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


from phylanx import Phylanx
from phylanx.util import debug_output


@Phylanx
def hello():
    debug("Hello")


hello()

assert(debug_output() == "Hello\n")
