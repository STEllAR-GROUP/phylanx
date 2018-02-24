# Copyright (c) 2017 Steven R. Brandt
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# A simple way to turn a python3 expression into a data structure (AST)

# First overload some basic operators.
# All the other classes will inherit from this.


class Ops:
    def __add__(self, a):
        return Plus(self, a)

    def __mul__(self, a):
        return Mul(self, a)

    def __sub__(self, a):
        return Sub(self, a)

    def __neg__(self):
        return Neg(self)

# Unary negation


class Neg (Ops):
    def __init__(self, a):
        self.a = a

    def __str__(self):
        return "Neg(" + str(self.a) + ")"

    def size(self):
        return self.a.size()

# The "+" operator


class Plus (Ops):
    def __init__(self, a, b):
        self.a = a
        self.b = b
        ar1 = a.size()
        ar2 = b.size()
        if ar1[0] != ar2[0] or ar1[1] != ar2[1]:
            raise Exception("size mismatch")

    def __str__(self):
        return "Add(" + str(self.a) + "," + str(self.b) + ")"

    def size(self):
        return self.a.size()

# The "-" operator


class Sub (Ops):
    def __init__(self, a, b):
        self.a = a
        self.b = b
        ar1 = a.size()
        ar2 = b.size()
        if ar1[0] != ar2[0] or ar1[1] != ar2[1]:
            raise Exception("size mismatch")

    def __str__(self):
        return "Sub(" + str(self.a) + "," + str(self.b) + ")"

    def size(self):
        return self.a.size()

# The "*" operator


class Mul (Ops):
    def __init__(self, a, b):
        self.a = a
        self.b = b
        ar1 = a.size()
        ar2 = b.size()
        if ar1[1] != ar2[0]:
            raise Exception("size mismatch")

    def __str__(self):
        return "Mul(" + str(self.a) + "," + str(self.b) + ")"

    def size(self):
        return [self.a.size()[0], self.b.size()[1]]

# A basic representation of an xs by ys matrix


class Arr (Ops):
    def __init__(self, xs, ys):
        self.xs = xs
        self.ys = ys

    def __str__(self):
        return "Matrix(" + str(self.xs) + "," + str(self.ys) + ")"

    def size(self):
        return [self.xs, self.ys]


# Create some matrices
a1 = Arr(3, 4)
a2 = Arr(4, 5)
a0 = Arr(5, 5)
a3 = Arr(3, 5)
a4 = Arr(3, 5)

# Create an expression
expr = a1 * a2 * -a0 + a3 - a4

# Convert the AST to a string and print it
print(expr)

# Print the size of the result matrix
print(expr.size())
