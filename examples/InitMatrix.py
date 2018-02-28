#  Copyright (c) 2017 Hartmut Kaiser
#  Copyright (c) 2018 R. Tohid
#  Copyright (c) 2018 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
import phylanx
from phylanx.util import *
import numpy as np

et = phylanx.execution_tree

# Create a vector of zeros
vz = et.var(np.zeros(10))

print(vz)

mz = et.var(np.zeros((3, 4)))

print(mz)

v = et.var(np.linspace(2.0, 3.4, 5))

print(v)

print("Breast Cancer")

m = et.eval("file_read_csv(\"./breast_cancer.csv\")")
print(m)

print(m)
print(et.eval("slice", m, et.var(0), et.var(3), et.var(0), et.var(3)))

three = et.eval("3")
four = et.eval("4")

print("The answer is 42")
print(et.eval("block(42.0)")[0])
print(et.eval("block(define(x,42),x)")[0])
print(
    et.eval("""
    block(
        define(arg0,10),
        define(fact,arg0,
            if(arg0 <= 1,
                1,
                arg0 *fact(arg0-1)
            )
        ),
        fact(arg0)
    )""")[0])
print("3=", three[0])
print(
    et.eval("""
    block(
        define(fact,arg0,
            if(arg0 <= 1,
                1,
                arg0 *fact(arg0-1)
            )
        ),
        fact
    )""", three)[0])
print(et.eval("block(define(foo,arg0,slice(arg0,0,3,0,3)),foo)", m))
print(
    et.eval("block(define(addme,arg0,arg1,arg0+arg1),addme)", three, four))
et.eval('cout("Hello ",3," - ",4.1-2.9)')
et.eval("""
    block(
        define(i,0),
        while(i<10,
            block(
                store(i,i+1)
            )
        ),
        cout(i))""")
