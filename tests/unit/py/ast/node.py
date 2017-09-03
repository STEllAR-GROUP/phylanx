#  Copyright (c) 2017 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

try:
    import phylanx
    import phylanx.ast as ast
except Exception:
    import phylanxd as phylanx
    import phylanxd.ast as ast

###############################################################################
def test_identifier():
    ident1 = ast.identifier('ident1')
    assert(ident1.name == 'ident1')

    ident2 = ast.identifier('ident1')
    assert(ident2.name == 'ident1')
    assert(ident1 == ident2)

    ident3 = ast.identifier('ident3')
    assert(ident3.name == 'ident3')
    assert(ident1 != ident3)

def test_primary_expr():
    pe1 = ast.primary_expr(False)
    assert(pe1.value == False)

    ident1 = ast.identifier('ident1')
    pe2 = ast.primary_expr(ident1)
    assert(pe2.value == ident1)

def test_operand():
    pe1 = ast.primary_expr(True)
    op1 = ast.operand(pe1)
    assert(op1.value == pe1)

###############################################################################
test_identifier()
test_primary_expr()
test_operand()

