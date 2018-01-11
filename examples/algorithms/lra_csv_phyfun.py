import phylanx
et = phylanx.execution_tree
from phylanx.util import *

@phyfun
def lra(x, y, alpha, iterations, enable_output):
        weights=constant(0.0, shape(x, 1))
        transx=transpose(x)
        pred=constant(0.0, shape(x, 0))
        error=constant(0.0, shape(x, 0))
        gradient=constant(0.0, shape(x, 1))
        step=0
        while step < iterations:
                if enable_output:
                    print("step: ", step, ", ", weights)
                pred=1.0 / (1.0 + exp(-dot(x, weights)))
                error=pred - y
                gradient=dot(transx, error)
                weights=weights - (alpha * gradient)
                step += 1
        return weights


file_name = "breast_cancer.csv"

data = et.file_read_csv(file_name)
x = et.slice(data, 0, 569, 0, 30)
y = et.slice(data, 0, 569, 30, 31)
res = lra(x,y,1e-5,750,0)
phy_print(res)
