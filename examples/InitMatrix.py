#  Copyright (c) 2017 Hartmut Kaiser
#  Copyright (c) 2018 R. Tohid
#  Copyright (c) 2018 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
import phylanx
et = phylanx.execution_tree
from phylanx.util import *

# Create a vector of zeros
vz = et.zeros(10)

phy_print(vz)

mz = et.zeros(3, 4)

phy_print(mz)

v = et.linspace(2.0, 3.4, 5)

phy_print(v)

m = et.linearmatrix(3, 4, 9.0, 1.2, .3)

phy_print(m)

print("Breast Cancer")

m = et.file_read_csv("./algorithms/breast_cancer.csv")
print(m)
phy_print(m)
phy_print(et.slice(m, 0, 3, 0, 3))
three = et.phylisp_eval("3")
four = et.phylisp_eval("4")

print("The answer is 42")
print(et.phylisp_eval("block(42.0)").get(0))
print(et.phylisp_eval("block(define(x,42),x)").get(0))
print(et.phylisp_eval("""
    block(
        define(arg0,10),
        define(fact,arg0,
            if(arg0 <= 1,
                1,
                arg0 *fact(arg0-1)
            )
        ),
        fact(arg0)
    )""").get(0))
print("3=", three.get(0))
print(et.phylisp_eval("""
    block(
        define(fact,arg0,
            if(arg0 <= 1,
                1,
                arg0 *fact(arg0-1)
            )
        ),
        fact
    )""", three).get(0))
phy_print(et.phylisp_eval("block(define(foo,arg0,slice(arg0,0,3,0,3)),foo)", m))
phy_print(et.phylisp_eval(
    "block(define(addme,arg0,arg1,arg0+arg1),addme)", three, four))
et.phylisp_eval('cout("Hello ",3," - ",4.1-2.9)')
et.phylisp_eval("""
    block(
        define(i,0),
        while(i<10,
            block(
                store(i,i+1)
            )
        ),
        cout(i))""")


### TEST Primitive Operations ###
# Create a vector of zeros
vz = et.zeros(10)
phy_print(vz)

# Create a matrix of zeros
mz = et.zeros(3, 4)
phy_print(mz)

v = et.linspace(2.0, 3.4, 5)
phy_print(v)

m = et.linearmatrix(3, 4, 9.0, 1.2, .3)
phy_print(m)

# Dot product
m = et.linearmatrix(2, 2, 0, 1, 1)
d = et.dot(m, m)
phy_print(d)

print("Breast Cancer")
m = et.file_read_csv("./algorithms/breast_cancer.csv")
print(m)
phy_print(m)

# Cross product
v1 = et.linspace(1, 2, 3)
v2 = et.linspace(-1, -2, 3)
cross = et.cross(v1, v2)
phy_print(cross)

# Determinant
m = et.linearmatrix(2, 2, 0, 1, 1)
determinant = et.det(m)
phy_print(determinant)

# division
a = et.var(42)
b = et.var(7)
phy_print(et.div(a, b))

# exponential
x = et.linspace(1, 4, 4)
phy_print(et.exp(x))

# inverse NOTE: returns -0.00, is this OK?
m = et.linearmatrix(2, 2, 0, 1, 1)
phy_print(et.inv(m))

# # power
m = et.linearmatrix(2, 2, 0, 1, 1)
phy_print(et.power(m, et.var(2)))

# subtraction
phy_print(et.subtract(et.var(3), et.var(2)))
