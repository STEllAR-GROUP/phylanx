#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx, PhylanxSession


@Phylanx
def foo():
    a = 2
    return a


def main():
    assert (2 == foo())


if __name__ == "__main__":
    PhylanxSession.init(1)
    main()
