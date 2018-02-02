# Copyright (c) 2017 Hartmut Kaiser
# Copyright (c) 2018 Steven R. Brandt
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

try:
    from _phylanx.util import *

except Exception:
    from _phylanxd.util import *

import phylanx
et = phylanx.execution_tree
import inspect, re, ast

def phy_print(m):
    ndim = m.num_dimensions()
    if ndim == 1:
        for i in range(m.dimension(0)):
            print(m[i])
    elif ndim == 2:
        for i in range(m.dimension(0)):
            for j in range(m.dimension(1)):
                print("%10.2f" % m[i, j], end=" ")
                if j > 5:
                    print("...", end=" ")
                    break
            print()
            if i > 5:
                print("%10s" % "...")
                break
    elif ndim == 0:
        print(m[0])
    else:
        print("ndim=", ndim)

def get_node(node,**kwargs):
    if node == None:
        return None
    args = [arg for arg in ast.iter_child_nodes(node)]
    params = {"name":None,"num":None}
    for k in kwargs:
        if k not in params:
            raise Exception("Invalid argument '%s'" % k)
        else:
            params[k] = kwargs[k]
    name = params["name"]
    num  = params["num"]
    if name != None:
        for i in range(len(args)):
            a = args[i]
            nm = a.__class__.__name__
            if nm == name:
                if num == None or num == i:
                    return a
    elif num != None:
        if num < len(args):
            return args[num]

    # if we can't find what we're looking for...
    return None

def full_node_name(a, name):
    return '%s#%d#%d' % (name, a.lineno, a.col_offset)

def full_name(a):
    return full_node_name(a, a.name)

class Recompiler:
    def __init__(self):
        self.defs = {}
        self.priority = 0
        self.groupAggressively = True
    def recompile(self,a,allowreturn=False):
        nm = a.__class__.__name__
        if nm == "Num":
            return str(a.n)
        elif nm == "Str":
            return '"' + a.s + '"'
        elif nm == "Name":
            return full_node_name(a, a.id)
        elif nm == "Expr":
            args = [arg for arg in ast.iter_child_nodes(a)]
            s = ""
            if len(args)==1:
                s += self.recompile(args[0])
            else:
                raise Exception('unexpected: expression has more than one sub-expression')
            return s
        elif nm == "Subscript":
            e = get_node(a,name="ExtSlice")
            s0 = get_node(e,name="Slice",num=0)
            s1 = get_node(e,name="Slice",num=1)
            if s0 != None and s1 != None:
                xlo = s0.lower
                xhi = s0.upper
                ylo = s1.lower
                yhi = s1.upper
                sname = self.recompile(get_node(a,num=0))
                s = "slice("
                s += sname
                s += ","
                if xlo == None:
                    s += "0"
                else:
                    s += self.recompile(xlo)
                s += ","
                if xhi == None:
                    s += "shape(" + sname + ",0)"
                else:
                    s += self.recompile(xhi)
                s += ","
                if ylo == None:
                    s += "0"
                else:
                    s += self.recompile(ylo)
                s += ","
                if yhi == None:
                    s += "shape(" + sname + ",1)"
                else:
                    s += self.recompile(yhi)
                s += ")"
                return s
            else:
                raise Exception("Unsupported slicing: line=%d"%a.lineno)
        elif nm == "FunctionDef":
            args = [arg for arg in ast.iter_child_nodes(a)]
            s = ""
            s += '%s(' % full_node_name(a, 'define')
            s += full_name(a)
            s += ", "
            for arg in ast.iter_child_nodes(args[0]):
                s += full_node_name(arg, arg.arg)
                s += ", "
            if len(args)==2:
                s += self.recompile(args[1], True).strip(' ')
            else:
                s += '%s(' % full_node_name(a, 'block')
                #for aa in args[1:]:
                sargs = args[1:]
                for i in range(len(sargs)):
                    aa = sargs[i]
                    if i+1 == len(sargs):
                        s += self.recompile(aa,True)
                    else:
                        s += self.recompile(aa,False)
                        s += ", "
                s += ")"
            s += "), " + full_name(a)
            return s
        elif nm == "BinOp":
            args = [arg for arg in ast.iter_child_nodes(a)]
            s = ""
            save = self.priority
            nm2 = args[1].__class__.__name__
            priority = 2
            if nm2 == "Add":
                op = " + "
                priority = 1
            elif nm2 == "Sub":
                op = " - "
                priority = 1
            elif nm2 == "Mult":
                op = " * "
            elif nm2 == "Div":
                op = " / "
            elif nm2 == "Mod":
                op = " % "
            elif nm2 == "Pow":
                op = " ** "
                priority = 3
            else:
                raise Exception('binary operation not supported: %s' % nm2)
            self.priority = priority
            term1 = self.recompile(args[0])
            self.priority = priority
            term2 = self.recompile(args[2])
            if priority < save or self.groupAggressively:
                s += "(" + term1 + op + term2 + ")"
            else:
                s += term1 + op + term2
            return s
        elif nm == "Call":
            args = [arg for arg in ast.iter_child_nodes(a)]
            if args[0].id == "print":
                args[0].id = "cout"
            s = args[0].id+'('
            for n in range(1,len(args)):
                if n > 1:
                    s += ', '
                s += self.recompile(args[n])
            s += ')'
            return s
        elif nm == "Module":
            args = [arg for arg in ast.iter_child_nodes(a)]
            return self.recompile(args[0])
        elif nm == "Return":
            if not allowreturn:
                raise Exception("Return only allowed at end of function: line=%d\n" % a.lineno)
            args = [arg for arg in ast.iter_child_nodes(a)]
            return " "+self.recompile(args[0])
        elif nm == "Assign":
            args = [arg for arg in ast.iter_child_nodes(a)]
            s = ""
            if args[0].id in self.defs:
                s += "store"
            else:
                s += "define"
                self.defs[args[0].id]=1
            s += "("+args[0].id+","+self.recompile(args[1])+")"
            return s
        elif nm == "AugAssign":
            args = [arg for arg in ast.iter_child_nodes(a)]
            sym = "?"
            nn = args[1].__class__.__name__
            if nn == "Add":
                sym = "+"
            return "store("+args[0].id+","+args[0].id + sym + self.recompile(args[2])+")"
        elif nm == "While":
            args = [arg for arg in ast.iter_child_nodes(a)]
            s = "while("+self.recompile(args[0])+","
            if len(args)==2:
                s += self.recompile(args[1])
            else:
                tw = "block("
                for aa in args[1:]:
                    s += tw
                    tw = ", "
                    s += self.recompile(aa)
                s += ")"
            s += ")"
            return s
        elif nm == "If":
            s = "if("+self.recompile(a.test)+","
            if len(a.body)>1:
                s += "block("
                for j in range(len(a.body)):
                    aa = a.body[j]
                    if j > 0:
                        s += ", "
                    if j + 1 == len(a.body):
                        s += self.recompile(aa,allowreturn)
                    else:
                        s += self.recompile(aa)
                s += ")"
            else:
                s += self.recompile(a.body[0],allowreturn)
            s += ","
            if len(a.orelse)>1:
                s += "block("
                for j in range(len(a.orelse)):
                    aa = a.orelse[j]
                    if j > 0:
                        s += ", "
                    if j + 1 == len(a.orelse):
                        s += self.recompile(aa,allowreturn)
                    else:
                        s += self.recompile(aa)
                s += ")"
            elif len(a.orelse)==1:
                s += self.recompile(a.orelse[0],allowreturn)
            else:
                s += "block()"
            s += ")"
            return s
        elif nm == "Compare":
            args = [arg for arg in ast.iter_child_nodes(a)]
            sym = "?"
            nn = args[1].__class__.__name__
            if nn == "Lt":
                sym = " < "
            elif nn == "Gt":
                sym = " > "
            elif nn == "LtE":
                sym = " <= "
            elif nn == "GtE":
                sym = " >= "
            elif nn == "NotEq":
                sym = " != "
            elif nn == "Eq":
                sym = " == "
            else:
                raise Exception('boolean operation not supported: %s' % nn)
            return self.recompile(args[0])+sym+self.recompile(args[2])
        elif nm == "UnaryOp":
            args = [arg for arg in ast.iter_child_nodes(a)]
            nm2 = args[0].__class__.__name__
            nm3 = args[1].__class__.__name__
            if nm2 == "USub":
                if nm3 == "BinOp" or self.groupAggressively:
                    return "-(" + self.recompile(args[1])+")"
                else:
                    return "-" + self.recompile(args[1])
            else:
                raise Exception(nm2)
        else:
            raise Exception('unsupport AST node type: %s' % nm)

def convert_to_phylanx_type(v):
    t = type(v)
    try:
        import numpy
        if t == numpy.ndarray:
            return et.var(v)
    except:
          pass
    if t == int or t == float or t == list or t == str:
        return et.var(v)
    else:
        return v

# Create the decorator
class phyfun(object):
    def __init__(self,f):
        self.f = f
        # Get the source code
        src = inspect.getsource(f) #getsource(f)
        # Before recompiling the code, take
        # off the decorator from the source.
        src = re.sub(r'^\s*@\w+\s*','',src)

        # Create the AST
        tree = ast.parse(src)
        r = Recompiler()

        assert len(tree.body) == 1

        self.__physl_src__ = '%s(%s)\n' % (
            full_node_name(tree.body[0], 'block'), r.recompile(tree))

    def __call__(self,*args):
        nargs = tuple(convert_to_phylanx_type(a) for a in args)
        return et.eval(self.__physl_src__,*nargs)
