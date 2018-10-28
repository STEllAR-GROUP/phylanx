# Copyright (c) 2018 Maxwell Reeser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import ast


# This needs review
def parse_zeros(call, td, args, keywords):
    """Special function for determining output types of the numpy.zeros
    function given its arguments"""
    args = call.args
    if len(args) > 0:
        if isinstance(args[0], ast.Tuple):
            if len(args[0].elts) == 0:
                return 'scalar', (1, 1)
            elif len(args[0].elts) == 1:
                if args[0].elts[0].n > 1:
                    return 'row_vector', (1, args[0].elts[0].n)
                else:
                    return 'scalar', (1, 1)
            else:
                if isinstance(args[0].elts[0], ast.Num) and isinstance(args[0].elts[1], ast.Num):
                    if args[0].elts[0].n > 1:
                        if args[0].elts[1].n > 1:
                            return 'matrix', (args[0].elts[0].n, args[0].elts[1].n)
                        else:
                            return 'column_vector', (args[0].elts[0].n, args[0].elts[1].n)
                    else:
                        if args[0].elts[1].n > 1:
                            return 'row_vector', (args[0].elts[0].n, args[0].elts[1].n)
                        else:
                            return 'scalar', (args[0].elts[0].n, args[0].elts[1].n)
                else:
                    raise TypeError("numpy function output type not determinable on "
                                    "variable inputs")
    return None


def parse_identity(call, td, args, keywords):
    """Special function for determining output types of the numpy.identity
    function given its arguments"""
    args = call.args
    if len(args) > 0:
        if isinstance(args[0].n, int):
            if args[0].n == 1:
                return 'scalar', (1, 1)
            elif args[0].n >= 2:
                return 'matrix', (args[0].n, args[0].n)
    return None

'''
def parse_determinant(call, td):
    if isinstance(call.func, ast.Name):
        if call.func.id == 'determinant':
            if len(call.args) == 1:  # and isinstance(node.args[0], ast.Name):
                x = TypeDeducer(td.type_deducer_state)
                x.visit(call.args[0])
                if x.dims is not None:
                    if x.dims[0] == x.dims[1]:
                        if x.dims[0] > 2:
                            td.var_type = "matrix"
                            td.dims = (x.dims[0] - 1, x.dims[0] - 1)
                        elif x.dims[0] == 2:
                            td.var_type = "scalar"
                            td.dims = (1, 1)
                        else:
                            raise NotImplementedError("Determinant calls must be"
                                                      " on matrices with detectable, "
                                                      "square dimensions")
                        td.make_var_representation(call)
                        return
            raise NotImplementedError("Determinant calls must be on matrices with detectable, "
                                              "square dimensions")
    raise NotImplementedError(
        "Non-Numpy function calls not supported"
    )
'''


"""Dictionary for determining which numpy parsing function should be called"""
numpy_parsers = {
    'zeros': parse_zeros,
    'ones': parse_zeros,
    'identity': parse_identity # ,
    # 'determinant': parse_determinant
}
