# Copyright (c) 2018 Maxwell Reeser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# from phylanx import Phylanx
# import sys
# sys.path.append('/home/mreeser/src/repos/phylanx/python/phylanx/ast')
# import physl
import ast
import astpretty
import time
import TypeDeducer
from TypeDeducer import TypeDeducerState

'''
@Phylanx(debug=True)
def tea_func(b : str , d):
  a = numpy.zeros(b, d)
  a += b(d)
  b = a + d
  return b
'''


class LinearAlgebraContainer(object):

    def __init__(self, name, lineno, col_offset, dims):
        self.name = name
        self.lineno = lineno
        self.col_offset = col_offset
        self.dims = dims


def get_var_and_target_list(expr):
    tree = ast.parse(expr)
    astpretty.pprint(tree)
    time.sleep(0.01)
    var_dict = {'dummy': 'me'}
    state: TypeDeducerState = TypeDeducer.TypeDeducerState(variable_dict=var_dict)
    k2 = TypeDeducer.TypeDeducer(state)
    k2.visit(tree)

    print('--------------Variable List-----------------')
    for tmp in k2.type_deducer_state.var_list:
        tmp.print()
    print('---------------Target List------------------')
    for tmp in k2.type_deducer_state.target_list:
        tmp.print()


expr_ = '''
def tea_func(a : row_vector):
  b  = np.ones((2,2))
  c  = np.transpose(a)
  d  = np.dot(a,b)
  
def sec_func(b,d):
  a = b * d
  
  
a = np.ones((1,5))
tea_func(a)
sec_func(a,a)
'''

expr2 = '''
def test_func():
    a : scalar = 1
    a += 1
'''

expr = '''
def test_func():
    b = np.zeros((3,3))
    c = np.zeros((3,3))
    c = determinant(b)
    c = b
    a = np.zeros((4,4)) * c
'''

get_var_and_target_list(expr)


