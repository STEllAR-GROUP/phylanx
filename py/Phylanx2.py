import phylanx.ast as ast
import phylanx.util as util
# A simple way to turn a python3 expression into a data structure (AST)

def oper(x):
    t = type(x)
    if t == ast.primary_expr:
        return ast.operand(x)
    elif t == ast.expression:
        return ast.operand(ast.primary_expr(x))
    elif t == ast.unary_expr:
        return ast.operand(x)
    else:
        raise Exception(str(t))

# First overload some basic operators.
# All the other classes will inherit from this.
class Ops:
    def __add__(self,a):
        return Plus(self,a)
    def __mul__(self,a):
        return Mul(self,a)
    def __sub__(self,a):
        return Sub(self,a)
    def __neg__(self):
        return Neg(self)

    def __init__(self,a,b):
        self.a = a
        self.b = b
        e1 = ast.expression(oper(a.value))
        op2 = oper(b.value)
        #e1.rest.append([self.code(), op2])
        op3 = ast.operation(self.code(),op2)
        util.append_operation(e1,op3)
        self.value = e1

    def __str__(self):
        return str(self.value) 

# Unary negation
class Neg (Ops):
    def __init__(self,a):
        self.a = a
        self.value = ast.unary_expr(ast.optoken.op_negative,oper(a.value))

# The "+" operator
class Plus (Ops):
    def code(self):
        return ast.optoken.op_plus

# The "-" operator
class Sub (Ops):
    def code(self):
        return ast.optoken.op_minus

# The "*" operator
class Mul (Ops):
    def code(self):
        return ast.optoken.op_times

# A basic representation of an xs by ys matrix
class Arr (Ops):
    def __init__(self,ident):
        self.value = ast.primary_expr(ident)
    def __str__(self):
        return str(self.value)+":"+str(self.value.value)

# Create some matrices
a1 = Arr(3.14)
a2 = Arr("b")
a0 = Arr("c")
a3 = Arr("d")
a4 = Arr("e")

# Create an expression
expr = a1*a2*a0+a3*-a4
#expr = (a1 + a2)+a3-a4

# Convert the AST to a string and print it
print(util.stringify(expr.value))
