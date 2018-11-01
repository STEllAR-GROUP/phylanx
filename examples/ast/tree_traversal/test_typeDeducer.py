from unittest import TestCase
from TypeDeducer import TypeDeducer, TypeDeducerState
import ast
import astpretty


class TestTypeDeducer(TestCase):

    def get_td(self, expr, debug=False):
        tree = ast.parse(expr)
        if debug:
            astpretty.pprint(tree)
        tds = TypeDeducerState({})
        td = TypeDeducer(tds)
        td.visit(tree)
        return td

    def test_visit_Assign(self):
        # Constant assignment
        expr = '''
def test_func():
    a = 1
'''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on target list")
        self.assertEqual('scalar', td.type_deducer_state.target_list[0].var_type,
                         "Failed constant value assignment")

        # Ordinary Assignment with previously assigned type
        expr = '''
def test_func(b: scalar):
    a = b
'''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on target list")
        self.assertEqual('scalar', td.type_deducer_state.target_list[0].var_type,
                         "Failed previous-value assignment")

        # Ordinary Assignment without previously assigned type
        expr = '''
def test_func(b):
    a = b
'''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on target list")
        self.assertEqual(None, td.type_deducer_state.target_list[0].var_type,
                         "Failed no-previous-value assignment")

        # Binary Operation assignment
        expr = '''
def test_func(b: row_vector, c: column_vector):
    a = b * c
'''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on target list")
        self.assertEqual('matrix', td.type_deducer_state.target_list[0].var_type,
                         "Failed binary op assignment")

        # Unary operation
        expr = '''
def test_func(b: row_vector):
    a = -b
'''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on target list")
        self.assertEqual('row_vector', td.type_deducer_state.target_list[0].var_type,
                         "Failed unary op assignment")

        # Unary call
        expr = '''
def test_func(b: row_vector):
    a = np.transpose(b)
'''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on target list")
        self.assertEqual('column_vector', td.type_deducer_state.target_list[0].var_type,
                         "Failed unary call assignment")

        # Binary call
        expr = '''
def test_func(b: row_vector, c: column_vector):
    a = np.dot(b,c)
'''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on target list")
        self.assertEqual('scalar', td.type_deducer_state.target_list[0].var_type,
                         "Failed binary call assignment")

    def test_visit_AnnAssign(self):
        # Constant Annotated Assignment
        expr = '''
def test_func():
    a : scalar = 1
        '''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on target list")
        self.assertEqual('scalar', td.type_deducer_state.target_list[0].var_type,
                         "Failed constant value assignment")

        # Annotated Assignment with previously assigned type
        expr = '''
def test_func(b: scalar):
    a : scalar = b
        '''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on target list")
        self.assertEqual('scalar', td.type_deducer_state.target_list[0].var_type,
                         "Failed previous-value assignment")

        # Annotated Assignment without previously assigned type
        expr = '''
def test_func(b):
    a : None = b
        '''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on target list")
        self.assertEqual(None, td.type_deducer_state.target_list[0].var_type,
                         "Failed no-previous-value assignment")

        # Binary Operation annotated assignment
        expr = '''
def test_func(b: row_vector, c: column_vector):
    a : matrix = b * c
        '''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on target list")
        self.assertEqual('matrix', td.type_deducer_state.target_list[0].var_type,
                         "Failed binary op assignment")

        # Unary operation annotated assignment
        expr = '''
def test_func(b: row_vector):
    a : row_vector = -b
        '''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on target list")
        self.assertEqual('row_vector', td.type_deducer_state.target_list[0].var_type,
                         "Failed unary op assignment")

        # Unary call annotated assignment
        expr = '''
def test_func(b: row_vector):
    a : column_vector = np.transpose(b)
        '''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on target list")
        self.assertEqual('column_vector', td.type_deducer_state.target_list[0].var_type,
                         "Failed unary call assignment")

        # Binary call annotated assignment
        expr = '''
def test_func(b: row_vector, c: column_vector):
    a : scalar = np.dot(b,c)
        '''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on target list")
        self.assertEqual('scalar', td.type_deducer_state.target_list[0].var_type,
                         "Failed binary call assignment")

    def test_visit_AugAssign(self):
        # Constant augmented assignment
        expr = '''
def test_func():
    a  = 1
    a += 1
        '''
        td = self.get_td(expr)
        self.assertEqual(2, len(td.type_deducer_state.target_list),
                         "Wrong length on target list")
        self.assertEqual('scalar', td.type_deducer_state.target_list[0].var_type,
                         "Failed constant value assignment")

        # Augmented Assignment with previously assigned type
        expr = '''
def test_func(b: scalar):
    a : scalar = b
    a += 1
        '''
        td = self.get_td(expr)
        self.assertEqual(2, len(td.type_deducer_state.target_list),
                         "Wrong length on target list")
        self.assertEqual('scalar', td.type_deducer_state.target_list[0].var_type,
                         "Failed previous-value assignment")

        # Augmented Assignment without previously assigned type
        expr = '''
def test_func(a):
    a += 1
        '''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on target list")
        self.assertEqual(None, td.type_deducer_state.target_list[0].var_type,
                         "Failed no-previous-value assignment")

        # Binary Operation augmented assignment
        expr = '''
def test_func(b: row_vector, c: column_vector):
    b += b * c
        '''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on target list")
        self.assertEqual('matrix', td.type_deducer_state.target_list[0].var_type,
                         "Failed binary op assignment")

    def test_visit_Name(self):
        # Assignment on previously undeclared item
        expr = '''
def test_func():
    a = b
        '''
        self.assertRaises(Exception, self.get_td, expr)

        # Assignment on previously declared no-type item
        expr = '''
def test_func(b):
    a = b
        '''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on target list")
        self.assertEqual(None, td.type_deducer_state.target_list[0].var_type,
                         "Failed assignment on item with no type")

        # Assignment on previously declared item
        expr = '''
def test_func(b : scalar):
    a = b
        '''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on target list")
        self.assertEqual('scalar', td.type_deducer_state.target_list[0].var_type,
                         "Failed assignment on item with type")

    def test_visit_arg(self):
        # Assignment on previously declared item
        expr = '''
def test_func(b : scalar):
    a = 1
        '''
        td = self.get_td(expr)
        self.assertEqual(2, len(td.type_deducer_state.var_list),
                         "Wrong length on variable list")
        self.assertEqual('scalar', td.type_deducer_state.var_list[0].var_type,
                         "Failed assignment on item with type")

        # Assignment on previously undeclared item
        expr = '''
def test_func(b):
    a = 1
        '''
        td = self.get_td(expr)
        self.assertEqual(2, len(td.type_deducer_state.var_list),
                         "Wrong length on variable list")
        self.assertEqual(None, td.type_deducer_state.var_list[0].var_type,
                         "Failed assignment on item with type")

        # Assignment on two annotated parameters
        expr = '''
def test_func(b : scalar, c : matrix):
    a = 1
        '''
        td = self.get_td(expr)
        self.assertEqual(3, len(td.type_deducer_state.var_list),
                         "Wrong length on variable list")
        self.assertEqual('scalar', td.type_deducer_state.var_list[0].var_type,
                         "Failed assignment on item with type")
        self.assertEqual('matrix', td.type_deducer_state.var_list[1].var_type,
                         "Failed assignment on item with type")

        # Assignment on one annotated and one unannotated parameter
        expr = '''
def test_func(b, c : matrix):
    a = 1
        '''
        td = self.get_td(expr)
        self.assertEqual(3, len(td.type_deducer_state.var_list),
                         "Wrong length on variable list")
        self.assertEqual(None, td.type_deducer_state.var_list[0].var_type,
                         "Failed assignment on item with type")
        self.assertEqual('matrix', td.type_deducer_state.var_list[1].var_type,
                         "Failed assignment on item with type")

        # Assignment on one annotated and one unannotated parameter
        expr = '''
def test_func(b : scalar, c):
    a = 1
        '''
        td = self.get_td(expr)
        self.assertEqual(3, len(td.type_deducer_state.var_list),
                         "Wrong length on variable list")
        self.assertEqual('scalar', td.type_deducer_state.var_list[0].var_type,
                         "Failed assignment on item with type")
        self.assertEqual(None, td.type_deducer_state.var_list[1].var_type,
                         "Failed assignment on item with type")

    def test_visit_BinOp(self):
        # BinOp on one annotated and one unannotated parameter
        expr = '''
def test_func(b : scalar, c):
    a = b + c
        '''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on variable list")
        self.assertEqual(None, td.type_deducer_state.target_list[0].var_type,
                         "Failed BinOp assignment with one annotated and"
                         " one unannotated parameter")

        # BinOp on one annotated and one unannotated parameter
        expr = '''
def test_func(b, c : matrix):
    a = b + c
        '''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on variable list")
        self.assertEqual(None, td.type_deducer_state.target_list[0].var_type,
                         "Failed BinOp assignment with one annotated and"
                         " one unannotated parameter")

        # BinOp on two annotated parameter
        expr = '''
def test_func(b : row_vector, c : row_vector):
    a = b + c
        '''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on variable list")
        self.assertEqual('row_vector', td.type_deducer_state.target_list[0].var_type,
                         "Failed BinOp assignment with two annotated parameters")

    def test_visit_UnaryOp(self):
        # UnaryOp on one annotated parameter
        expr = '''
def test_func(b : matrix):
    a = ~b
        '''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on variable list")
        self.assertEqual('matrix', td.type_deducer_state.target_list[0].var_type,
                         "Failed UnaryOp assignment with one annotated parameter")

        # UnaryOp on one unannotated parameter
        expr = '''
def test_func(b):
    a = ~b
        '''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on variable list")
        self.assertEqual(None, td.type_deducer_state.target_list[0].var_type,
                         "Failed UnaryOp assignment with one unannotated parameter")

    def test_visit_Call(self):
        # Call on two variables
        expr = '''
def test_func(b, c):
    a = np.zeros((b, c))
        '''
        self.assertRaises(TypeError, self.get_td, expr)

        # Call on two variables
        expr = '''
def test_func(b, c):
    a = np.zeros((1, c))
        '''
        self.assertRaises(TypeError, self.get_td, expr)

        # Call on two variables
        expr = '''
def test_func(b, c):
    a = np.zeros((b, 2))
        '''
        self.assertRaises(TypeError, self.get_td, expr)

        # UnaryOp on two constants
        expr = '''
def test_func():
    a = np.zeros((1,2))
        '''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on variable list")
        self.assertEqual('row_vector', td.type_deducer_state.target_list[0].var_type,
                         "Failed Call with two constants meant to obtain row_vector")

        # UnaryOp on two constants
        expr = '''
def test_func():
    a = np.zeros((2,1))
        '''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on variable list")
        self.assertEqual('column_vector', td.type_deducer_state.target_list[0].var_type,
                         "Failed Call with two constants meant to obtain column_vector")

        # UnaryOp on two constants
        expr = '''
def test_func():
    a = np.zeros((1,1))
        '''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on variable list")
        self.assertEqual('scalar', td.type_deducer_state.target_list[0].var_type,
                         "Failed Call with two constants meant to obtain scalar")

        # UnaryOp on two constants
        expr = '''
def test_func():
    a = np.zeros((2,2))
        '''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on variable list")
        self.assertEqual('matrix', td.type_deducer_state.target_list[0].var_type,
                         "Failed Call with two constants meant to obtain matrix")

        # Determinant on r-value
        expr = '''
def test_func():
    a = determinant(np.zeros((3,3)))
        '''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on variable list")
        self.assertEqual('matrix', td.type_deducer_state.target_list[0].var_type,
                         "Failed call to determinant on 3x3 matrix")
        self.assertEqual((2, 2), td.type_deducer_state.target_list[0].dims,
                         "Failed call to determinant on 3x3 matrix")

        # Determinant on l-value
        expr = '''
def test_func():
    b = np.zeros((3,3))
    a = determinant(b)
        '''
        td = self.get_td(expr)
        self.assertEqual(2, len(td.type_deducer_state.target_list),
                         "Wrong length on variable list")
        self.assertEqual('matrix', td.type_deducer_state.target_list[1].var_type,
                         "Failed call to determinant on 3x3 matrix")
        self.assertEqual((2, 2), td.type_deducer_state.target_list[1].dims,
                         "Failed call to determinant on 3x3 matrix")

        # Determinant on r-value drop to scalar
        expr = '''
def test_func():
    a = determinant(np.zeros((2,2)))
            '''
        td = self.get_td(expr)
        self.assertEqual(1, len(td.type_deducer_state.target_list),
                         "Wrong length on variable list")
        self.assertEqual('scalar', td.type_deducer_state.target_list[0].var_type,
                         "Failed call to determinant on 3x3 matrix")
        self.assertEqual((1, 1), td.type_deducer_state.target_list[0].dims,
                         "Failed call to determinant on 3x3 matrix")

        # Determinant on l-value drop to scalar
        expr = '''
def test_func():
    b = np.zeros((2,2))
    a = determinant(b)
            '''
        td = self.get_td(expr)
        self.assertEqual(2, len(td.type_deducer_state.target_list),
                         "Wrong length on variable list")
        self.assertEqual('scalar', td.type_deducer_state.target_list[1].var_type,
                         "Failed call to determinant on 3x3 matrix")
        self.assertEqual((1, 1), td.type_deducer_state.target_list[1].dims,
                         "Failed call to determinant on 3x3 matrix")

        # Determinant on l-value drop to scalar
        expr = '''
def test_func():
    b = np.zeros((1, 1))
    a = determinant(b)
            '''
        self.assertRaises(NotImplementedError, self.get_td, expr)

    def test_visit_BoolOp(self):
        self.fail()