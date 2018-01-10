import phylanx
et = phylanx.execution_tree

fib10 = et.phylisp_eval("""
block(
    define(fib,n,
    if(n<2,n,
        fib(n-1)+fib(n-2))),
    fib)""",10)

assert fib10.get(0) == 55.0

sum10 = et.phylisp_eval("""
block(
    define(sum10,
        block(
            define(i,0),
            define(n,0),
            while(n < 10,
                block(
                  store(n,n+1),
                  store(i,i+n)
                )),
            i)),
    sum10)""")

assert sum10.get(0) == 55.0
