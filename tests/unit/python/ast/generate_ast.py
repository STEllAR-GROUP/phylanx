#  Copyright (c) 2017 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx

###############################################################################

phylanx.PhylanxSession.init(1)


class traverse_ast:
    def __init__(self):
        self.result = ''

    def __call__(self, ast, delimiter=''):
        self.result += str(ast) + delimiter
        return True


class traverse_ast_enter_exit:
    def __init__(self):
        self.result = ''

    def on_enter(self, ast, delimiter=''):
        self.result += str(ast) + delimiter
        return True

    # optional, can be ommitted
    def on_exit(self, ast, *args):
        return True


def test_expression(expr, expected, delimiter):
    ast = phylanx.ast.generate_ast(expr)
    assert (len(ast) == 1)

    visitor = traverse_ast()
    phylanx.ast.traverse(ast[0], visitor, delimiter)
    if visitor.result != expected:
        print(visitor.result, expected)
    assert (visitor.result == expected)

    visitor = traverse_ast_enter_exit()
    phylanx.ast.traverse(ast[0], visitor, delimiter)
    if visitor.result != expected:
        print(visitor.result, expected)
    assert (visitor.result == expected)


###############################################################################
test_expression('A + B', '(A$1$1 + B$1$5)\n' + 'A$1$1\n' + 'B$1$5\n' + '+\n',
                '\n')

test_expression(
    'A + B + -C', '(A$1$1 + B$1$5 + -C$1$10)\n' + 'A$1$1\n' + 'B$1$5\n' + \
    '+\n' + 'C$1$10\n' + '-\n' + '+\n', '\n')

test_expression(
    'A + B * C', '(A$1$1 + B$1$5 * C$1$9)\n' + 'A$1$1\n' + 'B$1$5\n' + \
    'C$1$9\n' + '*\n' + '+\n', '\n')

test_expression(
    'A * B + C', '(A$1$1 * B$1$5 + C$1$9)\n' + 'A$1$1\n' + 'B$1$5\n' + '*\n' + \
    'C$1$9\n' + '+\n', '\n')

test_expression(
    'func(A, B)', 'func$1$1(A$1$6, B$1$9)\n' + 'func$1$1(A$1$6, B$1$9)\n' + \
    'func$1$1\n' + 'A$1$6\n' + 'A$1$6\n' + 'B$1$9\n' + 'B$1$9\n', '\n')
