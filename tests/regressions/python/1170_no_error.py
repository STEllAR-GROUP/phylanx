#  Copyright (c) 2020 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #1170: Error messages for bad PhySL code not shown

from phylanx import Phylanx, PhylanxSession


@Phylanx(doc_src=True)
def foo():
    """
    define(foo, lambda(cout("Worked!"))
    """
    pass


def main():
    caught_exception = False
    try:
        foo()

    except Exception as e:
        caught_exception = True
        assert(type(e) == RuntimeError and "Incomplete parse" in str(e))

    assert(caught_exception)


if __name__ == "__main__":
    main()
