#  Copyright (c) 2019 R. Tohid
#  Copyright (c) 2019 Parsa Amini
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
# #979
from phylanx import Phylanx

@Phylanx
def fx_inner():
    return 40 + 2

@Phylanx
def fx():
    return fx_inner()

assert 42 == fx()
