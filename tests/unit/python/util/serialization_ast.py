#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx

###############################################################################


def test_list():
    expr_ast = phylanx.ast.generate_ast('A+B')
    expr_serialized = phylanx.util.serialize(expr_ast)
    expr_unserialized = phylanx.util.unserialize(expr_serialized)
    assert expr_ast == expr_unserialized


###############################################################################


def test_expr():
    pe1 = phylanx.ast.primary_expr(True)
    op1 = phylanx.ast.operand(pe1)
    e1 = phylanx.ast.expression(op1)
    data = phylanx.util.serialize(e1)
    data_unserialized = phylanx.util.unserialize_expr(data)
    assert e1 == data_unserialized


###############################################################################

test_expr()
test_list()
