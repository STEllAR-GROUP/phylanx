#  Copyright (c) 2017 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx

phylanx.PhylanxSession(1)

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
    assert (len(ast) == 1)

    visitor = traverse_ast()
    phylanx.ast.traverse(ast[0], visitor)

    if visitor.generated_string != expected:
        print(visitor.generated_string, expected)
    assert (visitor.generated_string == expected)


###############################################################################
test_traverse('A + B', '(A$1$1 + B$1$5)\nA$1$1\nB$1$5\n+\n')
test_traverse('A + B - C',
              '(A$1$1 + B$1$5 - C$1$9)\nA$1$1\nB$1$5\n+\nC$1$9\n-\n')
