# Copyright (c) 2017 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx.ast as ast

def operand(x):
    t = type(x)
    if t == ast.primary_expr:
        return ast.operand(x)
    elif t == ast.expression:
        return ast.operand(ast.primary_expr(x))
    elif t == ast.unary_expr:
        return ast.operand(x)
    else:
        raise Exception(str(t))

class ndarray:
    """"Base class for all data objects."""

    def __init__(self, data):
        self.value = ast.primary_expr(data)

    def __str__(self):
        return str(self.value)

    def __add__(self,a):
        expr = ast.expression(operand(a.value))
        operation = ast.operation(ast.optoken.op_plus, operand(self.value))
        expr.append(operation)
        return ndarray(expr)
        
    def __sub__(self,a):
        expr = ast.expression(operand(a.value))
        operation = ast.operation(ast.optoken.op_minus, operand(self.value))
        expr.append(operation)
        return ndarray(expr)

    def __mul__(self,a):
        expr = ast.expression(operand(a.value))
        operation = ast.operation(ast.optoken.op_times, operand(self.value))
        expr.append(operation)
        return ndarray(expr)


a = ndarray(3.14)
b = ndarray(2)
expr = a + b

expr = a + expr
print(expr)
print(a)
print(b)

expr2 = a * b
print(expr2)
