# Copyright (c) 2017 Hartmut Kaiser
# Copyright (c) 2018 Steven R. Brandt
# Copyright (c) 2018 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import ast
import re


def physl_fmt(src,tag=4):
    """Pretty print PhySL source code"""
    # Remove line number info
    src = re.sub(r'\$\d+','',src)

    # The regex below matches one of the following three
    # things in order of priority:
    # 1: a quoted string, with possible \" or \\ embedded
    # 2: a set of balanced parenthesis
    # 3: a single character
    pat = re.compile(r'"(?:\\.|[^"\\])*"|\([^()]*\)|.')
    indent = 0
    for s in re.findall(pat,src):
        if s in " \t\r\b\n":
            pass
        elif s == '(':
            print(s)
            indent += 1
            print(" " * indent * tab,end="")
        elif s == ')':
            indent -= 1
            print("",sep="")
            print(" " * indent * tab,end="")
            print(s,end="")
        elif s == ',':
            print(s)
            print(" " * indent * tab,end="")
        else:
            print(s,end="",sep="")
    print("",sep="")


def dump_info(a, depth=0):
    "Print detailed information about an AST"
    nm = a.__class__.__name__
    print("  " * depth, end="")
    iter_children = True
    if nm == "Num":
        if type(a.n) == int:
            print("%s=%d" % (nm, a.n))
        else:
            print("%s=%f" % (nm, a.n))
    elif nm == "Global":
        print("Global:", dir(a))
    elif nm == "Str":
        print("%s='%s'" % (nm, a.s))
    elif nm == "Name":
        print("%s='%s'" % (nm, a.id))
    elif nm == "arg":
        print("%s='%s'" % (nm, a.arg))
    elif nm == "Slice":
        print("Slice:")
        print("  " * depth, end="")
        print("  Upper:")
        if a.upper is not None:
            dump_info(a.upper, depth + 4)
        print("  " * depth, end="")
        print("  Lower:")
        if a.lower is not None:
            dump_info(a.lower, depth + 4)
        print("  " * depth, end="")
        print("  Step:")
        if a.step is not None:
            dump_info(a.step, depth + 4)
    elif nm == "If":
        iter_children = False
        print(nm)
        dump_info(a.test, depth)
        for n in a.body:
            dump_info(n, depth + 1)
        if len(a.orelse) > 0:
            print("  " * depth, end="")
            print("Else")
            for n in a.orelse:
                dump_info(n, depth + 1)
    else:
        print(nm)
    for (f, v) in ast.iter_fields(a):
        if type(f) == str and type(v) == str:
            print("%s:attr[%s]=%s" % ("  " * (depth + 1), f, v))
    if iter_children:
        for n in ast.iter_child_nodes(a):
            dump_info(n, depth + 1)


# based on http://bit.ly/2C58jl8
def dump_ast(node, level=0):
    if isinstance(node, ast.AST):
        if isinstance(node, ast.With):
            for body in node.body:
                fields = [(a, dump_ast(b, level))
                          for a, b in ast.iter_fields(body)]
        fields = [(a, dump_ast(b, level)) for a, b in ast.iter_fields(node)]
        if node._attributes:
            fields.extend([(a, dump_ast(getattr(node, a), level))
                           for a in node._attributes])
        return ''.join([
            node.__class__.__name__, '(', ', '.join(
                ('%s=%s' % field for field in fields)), ')'
        ])
    elif isinstance(node, list):
        indent = '    '
        lines = ['[']
        lines.extend((indent * (level + 2) + dump_ast(x, level + 2) + ','
                      for x in node))
        if len(lines) > 1:
            lines.append(indent * (level + 1) + ']')
        else:
            lines[-1] += ']'
        return '\n'.join(lines)
    return repr(node)


def printout(m):
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


def print_physl(physl_src):
    print(re.sub(r'\$\d+', '', physl_src))


def full_node_name(a, name=''):
    return '%s$%d$%d' % (name, a.lineno, a.col_offset)


def full_name(a):
    return full_node_name(a, a.name)
