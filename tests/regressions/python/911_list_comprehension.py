#  Copyright (c) 2019 R. Tohid
#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx


@Phylanx
def fx(x):
    return [[i + 1] for i in x]


assert([[2], [3], [4], [5]] == fx([1, 2, 3, 4]))
