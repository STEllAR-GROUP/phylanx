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

def lisp_fmt(src):
    pat = re.compile(r'"[^"]*"|\([^()]*\)|.')
    indent = 0
    for s in re.findall(pat,src):
        if s == '(':
            print(s,end="")
            indent += 1
        elif s == ')':
            indent -= 1
            print("")
            print(" " * indent * 2,end="")
            print(s,end="")
        elif s == ',':
            print(s)
            print(" " * indent * 2,end="")
        else:
            print(s,end="")
    print("")

def get_info(a,depth=0):
    nm = a.__class__.__name__
    print("  "*depth,end="")
    iter_children = True
    if nm == "Num":
        if type(a.n)==int:
            print("%s=%d" % (nm,a.n))
        else:
            print("%s=%f" % (nm,a.n))
    elif nm == "Str":
        print("%s='%s'" % (nm,a.s))
    elif nm == "Name":
        print("%s='%s'" %(nm,a.id))
    elif nm == "arg":
        print("%s='%s'" %(nm,a.arg))
    elif nm == "If":
        iter_children = False
        print(nm)
        for n in a.body:
            get_info(n,depth+1)
        if len(a.orelse)>0:
            print("  "*depth,end="")
            print("Else")
            for n in a.orelse:
                get_info(n,depth+1)
    else:
        print(nm)
    if iter_children:
        for n in ast.iter_child_nodes(a):
            get_info(n,depth+1)

class Recompiler:
    def __init__(self):
        self.defs = {}
        self.priority = 0
    def recompile(self,a,allowreturn=False):
      nm = a.__class__.__name__
      if nm == "Num":
          return str(a.n)
      elif nm == "Str":
          return '"'+a.s+'"'
      elif nm == "Name":
          return a.id
      elif nm == "Expr":
          args = [arg for arg in ast.iter_child_nodes(a)]
          s = ""
          if len(args)==1:
              s += self.recompile(args[0])
          else:
              raise Exception()
          return s
      elif nm == "FunctionDef":
          args = [arg for arg in ast.iter_child_nodes(a)]
          s = ""
          s += "define("
          s += a.name
          s += ","
          for arg in ast.iter_child_nodes(args[0]):
              s += arg.arg
              s += ","
          if len(args)==2:
              s += self.recompile(args[1],True)
          else:
              s += "block("
              #for aa in args[1:]:
              sargs = args[1:]
              for i in range(len(sargs)):
                  aa = sargs[i]
                  if i+1 == len(sargs):
                      s += self.recompile(aa,True)
                  else:
                      s += self.recompile(aa,False)
                      s += ","
              s += ")"
          s += ")," + a.name
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
              raise Exception(nm2)
          self.priority = priority
          term1 = self.recompile(args[0])
          self.priority = priority
          term2 = self.recompile(args[2])
          if priority < save:
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
                  s += ','
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
              s += self.recompile(args[1])+")"
          else:
              tw = "block("
              for aa in args[1:]:
                  s += tw
                  tw = ","
                  s += self.recompile(aa)
              s += ")"
          s += ")"
          return s
      elif nm == "If":
          s = "if("+self.recompile(a.test)+","
          if len(a.body)>1:
              tw = "block("
              for aa in a.body:
                  s += tw
                  tw = ","
                  s += self.recompile(aa)
              s += ")"
          else:
              s += self.recompile(a.body[0])
          s += ","
          if len(a.orelse)>1:
              tw = "block("
              for aa in a.orelse:
                  s += tw
                  tw = ","
                  s += self.recompile(aa)
              s += ")"
          else:
              s += self.recompile(a.orelse[0])
          s += ")"
          return s
      elif nm == "Compare":
          args = [arg for arg in ast.iter_child_nodes(a)]
          sym = "?"
          nn = args[1].__class__.__name__
          if nn == "Lt":
              sym = "<"
          elif nn == "Gt":
              sym = ">"
          elif nn == "LtE":
              sym = "<="
          elif nn == "GtE":
              sym = ">="
          elif nn == "NotEq":
              sym = "!="
          elif nn == "Eq":
              sym = "=="
          return self.recompile(args[0])+sym+self.recompile(args[2])
      elif nm == "UnaryOp":
          args = [arg for arg in ast.iter_child_nodes(a)]
          nm2 = args[0].__class__.__name__
          if nm2 == "USub":
            return "-(" + self.recompile(args[1])+")"
          else:
            raise Exception(nm2)
      else:
          raise Exception(nm)

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
        get_info(tree)
        #globals()["fn"]=
        r = Recompiler()
        self.new_src = "block(" + r.recompile(tree)+')\n'
        print('new_src =',self.new_src)
        lisp_fmt(self.new_src)
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
    print("Hello"," ",a)

hello()

three = et.phylisp_eval("3")

hello1(three)

@phylanx
def addme(a,b):
    return a+b

four = et.phylisp_eval("4")
hello1(four)

phy_print(addme(three,four))

@phylanx
def sum10():
    s=0
    i=0
    while i <= 10:
        s += i
        i += 1
    return s

phy_print(sum10())

@phylanx
def fib(n):
    ret = 0
    if n < 2:
        ret = n
    else:
        ret = fib(n-1)+fib(n-2)
    return ret

ten = et.phylisp_eval("10")

print("fib(10)=",end="")
phy_print(fib(10))

@phylanx
def twof(a):
    if a < 2:
        print("a less than 2")
        print("branch 1")
    elif a==2:
        print("a equals 2")
        print("branch 2")
    else:
        print("a more than 2")
        print("branch 3")

for i in range(1,4):
    print("(",i,")","=" * 20)
    phyi = et.phylisp_eval(str(i))
    twof(phyi)

@phylanx
def cexpr(a,b):
    return a+ -b +3+4*(a-b%4)

one = et.phylisp_eval("1")
#phy_print(cexpr(one,one))
