#  Copyright (c) 2017 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx

###############################################################################
class traverse_ast:
    def __init__(self):
        pass

    def on_enter(self, ast):
        return True

    # optional, can be ommitted
    def on_exit(self, ast):
        return True

def test_traverse(expr):
    ast = phylanx.ast.generate_ast(expr)

    visitor = traverse_ast()
    phylanx.ast.traverse(ast, visitor)


###############################################################################
test_traverse('A + B')
