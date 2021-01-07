#!/usr/bin/env python3
# Copyright (c) 2019 Parsa Amini
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# flake8: noqa

# ## Synopsis
# ```
# usage: check_test_coverage.py [-h] cci_config ctest_out
#
# Check if all tests are run by CTest in the CircleCI configuration
#
# positional arguments:
#   cci_config  File path to read the CircleCI configuration from
#   ctest_out   File path to read the list of tests from
#
# optional arguments:
#   -h, --help  show this help message and exit
# ```

import argparse
import re
import subprocess
import sys


def expand_in_bash(pattern):
    s = subprocess.check_output(['bash', '-c', 'echo -n {}'.format(pattern)])
    return s.decode('utf-8')


def detect_uncovered_tests(cci_config_file, tests_file):
    # List CircleCI configuration
    cci_config = cci_config_file.read()
    # Read test names
    tests = tests_file.read().split()

    # find all arguments passed to ctest ... -R
    test_groups = [
        expand_in_bash(i.group(1))
        for i in re.finditer('ctest\s.+-R\s(\S+)', cci_config)
    ]

    # Initialize dictionary with a key per test and values set to false
    tests_coverage = {key: False for key in tests}

    # For each test case
    for t in tests_coverage.keys():
        # For each test group
        for p in test_groups:
            m = re.match(p, t)
            # Does the test group cover the test case
            if m:
                tests_coverage[t] = True
                break

    return [t for t, v in tests_coverage.items() if not v]


def main():
    parser = argparse.ArgumentParser(
        description='Check if all tests are run by CTest in the CircleCI configuration')
    parser.add_argument('cci_config',
                        type=argparse.FileType('r'),
                        help='File path to read the CircleCI configuration from')
    parser.add_argument('ctest_out',
                        type=argparse.FileType('r'),
                        help='File path to read the list of tests from')

    args = parser.parse_args()
    uncovered_tests = detect_uncovered_tests(args.cci_config, args.ctest_out)

    if (uncovered_tests):
        print('Found {} uncovered tests:'.format(len(uncovered_tests)))
        for i in uncovered_tests:
            print(i)
        sys.exit(1)

    print('All tests are covered.')


if __name__ == '__main__':
    main()
