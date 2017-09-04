#  Copyright (c) 2017 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

try:
    # first try release version
    import phylanx
except Exception:
    # then try debug version
    import phylanxd as phylanx

###############################################################################
class traverse_ast:
    def __init__(self):
        self.result = ''

    def __call__(self, ast, delimiter = ''):
        self.result += str(ast) + delimiter
        return True

def test_expression(expr, expected, delimier):
    ast = phylanx.ast.generate_ast(expr)
    visitor = traverse_ast()
    phylanx.ast.traverse(ast, visitor, delimier)
    assert(visitor.result == expected)


###############################################################################
test_expression(
    'A + B',
    'expression\n' +
        'operand\n' +
            'primary_expr\n' +
                'identifier: A\n' +
        'operation\n' +
            'op_plus\n' +
        'operand\n' +
            'primary_expr\n' +
                'identifier: B\n',
    '\n')

test_expression(
    'A + B + -C',
    'expression\n' +
        'operand\n' +
            'primary_expr\n' +
                'identifier: A\n' +
        'operation\n' +
            'op_plus\n' +
        'operand\n' +
            'primary_expr\n' +
                'identifier: B\n' +
        'operation\n' +
            'op_plus\n' +
        'operand\n' +
            'unary_expr\n' +
                'op_negative\n' +
                'operand\n' +
                    'primary_expr\n' +
                        'identifier: C\n',
    '\n')
