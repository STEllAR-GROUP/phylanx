# Copyright (c) 2018 Maxwell Reeser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import ast


# This needs review
def parse_zeros(args, keywords):
    """Special function for determining output types of the numpy.zeros
    function given its arguments"""
    print("Inside parse_zeros!")
    if len(args) > 0:
        print(args[0])
        if isinstance(args[0], ast.Tuple):
            print("It's a tuple!")
            if len(args[0].elts) == 0:
                return 'scalar'
            elif len(args[0].elts) == 1:
                if args[0].elts[0].n > 1:
                    return 'vector'
                else:
                    return 'scalar'
            else:
                if args[0].elts[0].n > 1:
                    if args[0].elts[1].n > 1:
                        return 'matrix'
                    else:
                        return 'vector'
                else:
                    if args[0].elts[1].n > 1:
                        return 'vector'
                    else:
                        return 'scalar'
    return None


def parse_identity(args, keywords):
    """Special function for determining output types of the numpy.identity
    function given its arguments"""
    if len(args) > 0:
        if isinstance(args[0], int):
            if args[0] == 1:
                return 'scalar'
            elif args[0] >= 2:
                return 'matrix'
    return None


"""Dictionary for determining which numpy parsing function should be called"""
numpy_parsers = {
    'zeros': parse_zeros,
    'ones': parse_zeros,
    'identity': parse_identity
}
