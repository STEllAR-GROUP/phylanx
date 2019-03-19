# Copyright (c) 2017 Hartmut Kaiser
# Copyright (c) 2018 Steven R. Brandt
# Copyright (c) 2018 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import ast
import re
import os


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


# source http://bit.ly/2C58jl8
def dump_ast(node, annotate_fields=True, include_attributes=False,
             indent='  '):
    def _format(node, level=0):
        if isinstance(node, ast.AST):
            fields = [(a, _format(b, level)) for a, b in ast.iter_fields(node)]
            if include_attributes and node._attributes:
                fields.extend([(a, _format(getattr(node, a), level))
                               for a in node._attributes])
            return ''.join([
                node.__class__.__name__, '(',
                ', '.join(('%s=%s' % field
                           for field in fields) if annotate_fields else (
                               b for a, b in fields)), ')'
            ])
        elif isinstance(node, list):
            lines = ['[']
            lines.extend((indent * (level + 2) + _format(x, level + 2) + ','
                          for x in node))
            if len(lines) > 1:
                lines.append(indent * (level + 1) + ']')
            else:
                lines[-1] += ']'
            return '\n'.join(lines)
        return repr(node)

    if not isinstance(node, ast.AST):
        raise TypeError('expected AST, got %r' % node.__class__.__name__)
    return _format(node)


def save_to_file(data, ofn, verbose=False):
    d = os.path.dirname(ofn)
    if d != "":
        os.makedirs(d, exist_ok=True)

    with open(ofn, "w") as f:
        print(data, file=f)
    if verbose is True:
        print(ofn, "saved")


def dump_to_file(data, key, kwargs):
    if kwargs[key] is True:
        ofn = key + "_" + kwargs["python_src_tag"] + ".txt"
        save_to_file(data, ofn, kwargs["verbose"])


def print_type(s, o):
    indent = "    "
    print("%-16s      type()      \n%s%s" % (s, indent, type(o)))
    print("")
    print("%-16s      dir()       \n%s%s" % (s, indent, dir(o)))
    print("")
    print("%-16s      __class__   \n%s%s" % (s, indent, o.__class__))
    print("")


def print_python_ast_node(s, node, depth=0):
    # two ways to check the nodes type:
    #   1.  if node.__class__.__name__ == "Num"
    #   2.  if isinstance(node, ast.Num):

    # nodes._fields gives a list of all child nodes

    # print("###################################")
    # print_type("node", node)

    if not isinstance(node, ast.AST):
        raise TypeError('Expected AST, got %r' % node.__class__.__name__)

    indent = "    "

    print(indent * depth, end="")
    print("%-12s" % s, node)
    # print("%-12s" % s, node.__class__)
    # print("%-12s %s" % (s, node.__class__.__name__), node._fields)
    # print("%-12s" % s, node.__class__, node._fields)

    # print(indent * depth, end="")
    # print("# for (fieldname, value) in ast.iter_fields(node):")
    for (fieldname, value) in ast.iter_fields(node):
        print(indent * (depth + 1), end="")
        print("%-12s" % fieldname, value)
    print("")

    # depth += 1
