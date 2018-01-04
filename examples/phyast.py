import phylanx
et = phylanx.execution_tree
import inspect
import ast
import re

def phy_print(m):
  ndim = m.num_dimensions()
  if ndim==1:
    for i in range(m.dimension(0)):
      print(m.get(i))
  elif ndim==2:
    for i in range(m.dimension(0)):
      for j in range(m.dimension(1)):
        print("%10.2f" % m.get(i,j),end=" ")
        if j > 5:
            print("...",end=" ")
            break
      print()
      if i > 5:
        print("%10s" % "...")
        break
  elif ndim==0:
    print(m.get(0))
  else:
    print("ndim=",ndim)

def get_info(a,depth=0):
    nm = a.__class__.__name__
    print("  "*depth,end="")
    if nm == "Num":
        if type(a.n)==int:
            print("%s=%d" % (nm,a.n))
        else:
            print("%s=%f" % (nm,a.n))
    elif nm == "Name":
        print("%s='%s'" %(nm,a.id))
    elif nm == "arg":
        print("%s='%s'" %(nm,a.arg))
    else:
        print(nm)
    for n in ast.iter_child_nodes(a):
        get_info(n,depth+1)

def recompile(a,depth=0):
    nm = a.__class__.__name__
    if nm == "Num":
        return str(a.n)
    elif nm == "Str":
        return '"'+a.s+'"'
    elif nm == "Name":
        return a.id
    elif nm == "For":
        args = [arg for arg in ast.iter_child_nodes(a)]
        argname = args[0].id
        rexpr = args[1]
        body = args[2]
        return "for("+recompile(rexpr)+", lambda "+argname+" : "+recompile(body)+")"
    elif nm == "Expr":
        args = [arg for arg in ast.iter_child_nodes(a)]
        return "("+recompile(args[0])+")"
    elif nm == "FunctionDef":
        args = [arg for arg in ast.iter_child_nodes(a)]
        s = "define("
        s += a.name
        s += ","
        for arg in ast.iter_child_nodes(args[0]):
            s += arg.arg
            s += ","
        s += recompile(args[1])
        s += ")," + a.name
        return s
    elif nm == "BinOp":
        args = [arg for arg in ast.iter_child_nodes(a)]
        s = recompile(args[0])
        for i in range(1,len(args),2):
            nm2 = args[i].__class__.__name__
            if nm2 == "Add":
                s += " + "
            elif nm2 == "Mult":
                s += " * "
            else:
                raise Exception(nm2)
            s += recompile(args[i+1])
        return s
    elif nm == "Call":
        args = [arg for arg in ast.iter_child_nodes(a)]
        if args[0].id == "print":
            args[0].id = "cout"
        s = args[0].id+'('
        for n in range(1,len(args)):
            if n > 1:
                s += ','
            s += recompile(args[n])
        s += ')'
        return s
    elif nm == "Module":
        args = [arg for arg in ast.iter_child_nodes(a)]
        return recompile(args[0])
    elif nm == "Return":
        args = [arg for arg in ast.iter_child_nodes(a)]
        return " "+recompile(args[0])
    else:
        s = "phyeval."+nm
        btw='('
        for n in ast.iter_child_nodes(a):
            s += btw
            btw = ','
            s += recompile(n,depth+1)
        if btw=='(':
            s += '()'
        else:
            s += ')'
        return s

local_namespace = {}
# Create the decorator
class phylanx(object):
    def __init__(self,f):
        global local_namespace
        self.f = f
        # Get the source code
        src = inspect.getsource(f) #getsource(f)
        # Before recompiling the code, take
        # off the decorator from the source.
        src = re.sub(r'^\s*@\w+\s*','',src)

        # Create the AST
        tree = ast.parse(src)
        #astdump.indented(tree)
        #get_info(tree)
        self.new_src = "block(" + recompile(tree)+')\n'
        print('new_src =',self.new_src)
        # Compile the AST
        #tmp_name = "new_func_name"
        #self.f1=compile(tmp_name+"="+new_src,filename='<string>',mode='single')
        # Exec creates the function in the namespace
        #exec(self.f1,{},local_namespace)
        # Get the function from the namespace
        #self.nf = local_namespace[tmp_name]

    def __call__(self,*args):
        #global local_namespace
        #print(local_namespace)
        #print("Recompiled...")
        #return self.nf(*args)
        #print('len=',len(args))
        if len(args)==1:
            return et.phylisp_eval(self.new_src,args[0])
        elif len(args)==2:
            return et.phylisp_eval(self.new_src,args[0],args[1])
        else:
            return et.phylisp_eval(self.new_src,*args)

@phylanx
def hello():
    print("Hello")

@phylanx
def hello1(a):
    print("Hello ",a)

hello()

three = et.phylisp_eval("3")

hello1(three)

@phylanx
def addme(a,b):
    return a+b

four = et.phylisp_eval("4")
hello1(four)

phy_print(addme(three,four))
