#!/usr/bin/env python
# Copyright (c) 2018 Parsa Amini
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# flake8: noqa

# ## Synopsis
# ```
# usage: flake_to_junit.py [-h] [source] [destination]
#
# Generate JUnit XML report from flake8 output
#
# positional arguments:
#   source       File path to read flake8 output from
#   destination  File path to write JUnit XML report to
#
# optional arguments:
#   -h, --help   show this help message and exit
# ```

import argparse
import sys
from xml.dom import minidom
from collections import namedtuple

error_item = namedtuple('error_item', 'filename line col code message')


def parse_flake8_log(fh):
    errors = []

    for line in fh:
        parts = line.split(":", 3)

        # Skip invalid lines
        if len(parts) == 4:
            error = error_item(filename=parts[0].strip(),
                               line=parts[1].strip(),
                               col=parts[2].strip(),
                               code=parts[3].strip()[:4],
                               message=parts[3].strip())

            errors.append(error)

    return errors


def convert(flake8_log_fh):
    errors = parse_flake8_log(flake8_log_fh)

    doc = minidom.Document()
    suite = doc.createElement('testsuite')
    doc.appendChild(suite)
    suite.setAttribute('name', 'flake8')
    suite.setAttribute('errors', str(len(errors)))
    suite.setAttribute('failures', '0')
    suite.setAttribute('tests', str(len(errors)))

    if len(errors) == 0:
        case = doc.createElement('testcase')
        case.setAttribute('name', 'flake8')
        case.setAttribute('time', '')
        suite.appendChild(case)

    for error in errors:
        case = doc.createElement('testcase')
        case.setAttribute('name', '{0}:{1}:{2}'.format(error.filename,
                                                       error.line,
                                                       error.col))
        case.setAttribute('time', '')
        suite.appendChild(case)

        failure = doc.createElement('failure')
        case.appendChild(failure)

        failure.setAttribute('file', error.filename)
        failure.setAttribute('line', error.line)
        failure.setAttribute('col', error.col)
        failure.setAttribute('message', error.message)
        failure.setAttribute('type', error.code)
        message = doc.createTextNode('{0}:{1}:{2} {3}'.format(
            error.filename,
            error.line,
            error.col,
            error.message))
        failure.appendChild(message)

    return doc


def main():
    parser = argparse.ArgumentParser(
        description='Generate JUnit XML report from flake8 output')
    parser.add_argument('source',
                        type=argparse.FileType('r'),
                        nargs='?',
                        default=sys.stdin,
                        help='File path to read flake8 output from')
    parser.add_argument('destination',
                        type=argparse.FileType('w'),
                        nargs='?',
                        default=sys.stdout,
                        help='File path to write JUnit XML report to')

    args = parser.parse_args()
    report = convert(args.source)
    report.writexml(args.destination,
                    addindent='    ', newl='\n', encoding='utf-8')


if __name__ == '__main__':
    main()
