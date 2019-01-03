#  Copyright (c) 2017 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx
import phylanx.ast as ast
import phylanx.util as util

###############################################################################

phylanx.PhylanxSession.init(1)


def test_serialization(in_ast):
    data = util.serialize(in_ast)
    out_ast = util.unserialize_expr(data)
    assert (in_ast == out_ast)

    import pickle
    pickled_data = pickle.dumps(in_ast, pickle.DEFAULT_PROTOCOL)
    out_ast = pickle.loads(pickled_data)
    assert (in_ast == out_ast)


###############################################################################


def test_identifier():
    ident1 = ast.identifier('ident1')
    assert (ident1.name == 'ident1')

    ident2 = ast.identifier('ident1')
    assert (ident2.name == 'ident1')
    assert (ident1 == ident2)

    ident3 = ast.identifier('ident3')
    assert (ident3.name == 'ident3')
    assert (ident1 != ident3)


def test_primary_expr():
    pe1 = ast.primary_expr(False)
    assert (not pe1.value)

    ident1 = ast.identifier('ident1')
    pe2 = ast.primary_expr(ident1)
    assert (pe2.value == ident1)


def test_operand():
    pe1 = ast.primary_expr(True)
    op1 = ast.operand(pe1)
    assert (op1.value == pe1)
    assert (op1.value.value)

    id2 = ast.identifier('ident')
    pe2 = ast.primary_expr(id2)
    op2 = ast.operand(pe2)
    assert (op2.value == pe2)
    assert (op2.value.value.name == 'ident')


def test_unary_expr():
    pe1 = ast.primary_expr(True)
    op1 = ast.operand(pe1)
    ue1 = ast.unary_expr(ast.optoken.op_not, op1)
    assert (ue1.operator == ast.optoken.op_not)
    assert (ue1.operand == op1)

    id2 = ast.identifier('ident')
    pe2 = ast.primary_expr(id2)
    op2 = ast.operand(pe2)
    ue2 = ast.unary_expr(ast.optoken.op_negative, op2)
    assert (ue2.operator == ast.optoken.op_negative)
    assert (ue2.operand == op2)


def test_operation():
    pe1 = ast.primary_expr(True)
    op1 = ast.operand(pe1)
    op2 = ast.operation(ast.optoken.op_plus, op1)
    assert (op2.operator == ast.optoken.op_plus)
    assert (op2.operand == op1)

    id2 = ast.identifier('ident')
    pe2 = ast.primary_expr(id2)
    op3 = ast.operand(pe2)
    op4 = ast.unary_expr(ast.optoken.op_minus, op3)
    assert (op4.operator == ast.optoken.op_minus)
    assert (op4.operand == op3)


def test_expression():
    pe1 = ast.primary_expr(True)
    op1 = ast.operand(pe1)
    e1 = ast.expression(op1)

    op2 = ast.operation(ast.optoken.op_plus, op1)
    e1.rest.append(op2)
    test_serialization(e1)

    e1.rest.append(op2)
    e1.rest.append(op2)
    test_serialization(e1)


###############################################################################
test_identifier()
test_primary_expr()
test_operand()
test_unary_expr()
test_operation()
test_expression()
