#  Copyright (c) 2017 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx

###############################################################################
class traverse_ast:
    def __init__(self):
        self.generated_string = ''
        pass

    def on_enter(self, ast):
        self.generated_string += str(ast) + '\n'
        return True

    # optional, can be ommitted
    def on_exit(self, ast):
        return True

def test_traverse(expr, expected):
    ast = phylanx.ast.generate_ast(expr)
    assert(len(ast) == 1)

    visitor = traverse_ast()
    phylanx.ast.traverse(ast[0], visitor)

    assert(visitor.generated_string == expected)


###############################################################################
test_traverse('A + B', '(A + B)\nA\nB\n+\n')
test_traverse('A + B - C', '(A + B - C)\nA\nB\n+\nC\n-\n')
