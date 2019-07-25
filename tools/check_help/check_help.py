# Copyright (c) 2018 Steven R. Brandt
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from xml.sax.saxutils import escape
import re
import io
from xml.dom import minidom
from collections import namedtuple
from sys import stdout

from phylanx.util import *
from phylanx import PhylanxSession

PhylanxSession.init(1)

argcount = {}

errors = []

# Get the list of all primitives/plugins
all = phylist()

errors_found = 0
output = io.StringIO()


def add_err(e):
    global errors_found
    errors_found += 1
    return e


for p in all:
    for m in all[p]:

        # We expect a pattern here, e.g. foo(_1, _2)
        # or foo(_1, __arg(_2,...), ...)
        m = re.sub(r'__arg\((\w+)[^\)]*\)', r'\1', m)
        g = re.match(r'(.*)\((.*)\)', m)
        if not g:
            continue

        # Keep the error string in err
        err = None

        # The name part of the pattern, e.g. "foo"
        n = g.group(1)

        # The args part of the pattern, e.g. "(_1, _2)"
        args = g.group(2)

        # The list of args, e.g. ["_1", "_2"]
        argli = re.findall(r'\w+', args)

        # The the phylanx help string for the function
        h = phyhelpex(n)

        # Strip off leading whitespace
        g = re.search(r'^([ \t]*)Args:', h, re.MULTILINE)
        if g:
            wh = g.group(1)
            h = re.sub(r'(^|\n)' + wh, r'\1', h)
            h = re.sub(r'[ \t]+$', '', h, re.MULTILINE)

        if re.search(r'@Deprecated@', h):
            continue

        # Initialize the argcount data structure
        if n not in argcount:
            argcount[n] = {'patterns': {}, "help": h}

        # If we have __1 in the pattern list, we can't
        # match arguments by count.
        if re.search(r'__\d+', args):
            argcount[n]["any"] = 1

        # Keep track of the number of args in this pattern
        argcount[n]["patterns"][len(argli)] = 1

        # The first line of the docstring has the arg names.
        first = h.splitlines()[0]
        first = re.sub(r'^\s*\w+\((.*)\)\s*', r'\1', first)
        argnames = re.findall(r'\w+', first)

        # Find the section of the docstring where args
        # are documented individually.
        g = re.search(r'\nArgs:.*Returns:', h, re.DOTALL)
        if not g:
            err = add_err("Cannot find arg definition region for '%s'" % n)
        else:
            argdefs = g.group(0)
            for argname in argnames:
                if not re.search(r"\n    [* ]?" + argname + r" ?\(", argdefs):
                    err = add_err(
                        "Missing definition or improper indentation " + \
                        "for arg: '%s' in '%s'" % (argname, n))

        # Get the count of the args in the docstring
        argli2 = re.findall(r'\w+', first)
        argcount[n]["max"] = len(argli2)

        # Check that there's an Args: section
        if not re.search(r'^Args:\n\n', h, re.MULTILINE):
            err = add_err("Missing 'Args:' section or 'Args:' not followed " + \
                          "by a blank line in '%s'" % n)

        # Check that there's a Returns: section
        if not re.search(r'\nReturns:', h):
            err = add_err("Missing 'Returns:' section in '%s'" % n)

        # Print error
        if err:
            errors += [{"message": err + "\nhelp was=" + h, "name": m}]

# Check for arg count mismatches
for a in argcount:
    v = argcount[a]
    if "any" in v:
        continue
    if re.search(r"@Deprecated@", v["help"]):
        continue
    if v["max"] not in v["patterns"]:
        print("Error: argument / pattern mismatch", file=output)
        print(a, v["max"], v["patterns"], file=output)
        print(v["help"], file=output)

doc = minidom.Document()
suite = doc.createElement('testsuite')
doc.appendChild(suite)
suite.setAttribute('name', 'check_help')
suite.setAttribute('errors', str(len(errors)))
suite.setAttribute('failures', '0')
suite.setAttribute('tests', str(len(errors)))

if len(errors) == 0:
    case = doc.createElement('testcase')
    case.setAttribute('name', 'check_help')
    case.setAttribute('time', '')
    suite.appendChild(case)

for error in errors:
    case = doc.createElement('testcase')
    case.setAttribute('name', error["name"])
    case.setAttribute('time', '')
    suite.appendChild(case)

    failure = doc.createElement('failure')
    case.appendChild(failure)

    failure.setAttribute('file', '<none>')
    failure.setAttribute('message', error["message"])
    failure.setAttribute('type', 0)
    message = doc.createTextNode(error["message"])
    failure.appendChild(message)

doc.writexml(stdout, addindent='    ', newl='\n', encoding='utf-8')

if errors_found == 0:
    exit(0)
else:
    exit(2)
