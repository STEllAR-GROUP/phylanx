import phylanx
from phylanx.ast import *

@Phylanx()
def f():
    a = None
    return a

assert f.__src__ == \
    "define$5$0(f$5$0, block$5$0(define$6$4(a$6$4, nil$6$8), a$7$11))"

#@Phylanx(debug=True)
#def f():
#    a = True
#    return a

#assert f.__src__ == \
#    "define$13$0(f$13$0, block$13$0(define$14$4(a$14$4, true$14$8), a$15$11))"

#@Phylanx(debug=True)
#def f():
#    a = False
#    return a

#assert f.__src__ == \
#    "define$13$0(f$21$0, block$21$0(define$22$4(a$22$4, false$22$8), a$23$11))"
