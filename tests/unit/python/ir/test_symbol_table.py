# Copyright (c) 2019 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from collections import defaultdict
import rhylanx.ir as ir


class TestNameSpace:
    def test_empty_space(self):
        namespace = ir.current_namespace
        assert [] == namespace

    def test_enter_space(self):
        with ir.NameSpace('dummy_space'):
            assert ['dummy_space'] == ir.current_namespace

    def test_exit_space(self):
        with ir.NameSpace('dummy_space'):
            pass
        assert [] == ir.current_namespace


class TestSymbolTable:
    def test_default_table(self):
        expected_table = defaultdict(
            lambda: defaultdict(lambda: defaultdict(lambda: [])))
        assert expected_table == ir.symbol_table

    def test_default_argument(self):
        symtable = ir.symbol_table
        assert defaultdict(lambda: []) == symtable['']['argument']

    def test_default_function(self):
        symtable = ir.symbol_table
        assert defaultdict(lambda: []) == symtable['']['functions']

    def test_default_variable(self):
        symtable = ir.symbol_table
        assert defaultdict(lambda: []) == symtable['']['variable']


class TestAddSymbol:
    def test_add_argument(self):
        symtable = ir.symbol_table
        with ir.NameSpace('dummy_space'):
            namespace = ir.current_namespace[-1]
            arg_info = ('arg', namespace, 123, 4)
            arg = ir.Argument(*arg_info)
            ir.SymbolTable.add_symbol(arg)
        assert [arg] == symtable[namespace]['arguments'][arg.name]

    def test_add_function(self):
        symtable = ir.symbol_table
        with ir.NameSpace('dummy_space'):
            namespace = ir.current_namespace[-1]
            function_info = ('func', 'dummy_space', 123, 4)
            function = ir.Function(*function_info)
            ir.SymbolTable.add_symbol(function)
        assert [function] == symtable[namespace]['functions'][function.name]

    def test_add_variable(self):
        symtable = ir.symbol_table
        with ir.NameSpace('dummy_space'):
            namespace = ir.current_namespace[-1]
            var_info = ('var', 'dummy_space', 123, 4, 'float')
            var = ir.Variable(*var_info)
            ir.SymbolTable.add_symbol(var)
        assert [var] == symtable[namespace]['variables'][var.name]

    def test_add_array(self):
        symtable = ir.symbol_table
        with ir.NameSpace('dummy_space'):
            namespace = ir.current_namespace[-1]
            array_info = ('array', 'dummy_space', 123, 4)
            array = ir.Array(*array_info)
            ir.SymbolTable.add_symbol(array)
        assert [array] == symtable[namespace]['variables'][array.name]


class TestCheckSymbolSingleNamespace:
    def test_check_argument(self):
        ir.symbol_table.clear()

        with ir.NameSpace('dummy_space'):
            arg_info = ('arg', 'dummy_space', 123, 4)
            arg = ir.Argument(*arg_info)
            ir.SymbolTable.add_symbol(arg)
            found_arg = ir.SymbolTable.check_symbol('arg', 'arguments',
                                                    'dummy_space')
        assert [arg] == found_arg

    def test_check_function(self):
        ir.symbol_table.clear()

        with ir.NameSpace('dummy_space'):
            function_info = ('func', 'dummy_space', 123, 4)
            function = ir.Function(*function_info)
            ir.SymbolTable.add_symbol(function)
            found_function = ir.SymbolTable.check_symbol(
                'func', 'functions', 'dummy_space')
        assert [function] == found_function

    def test_check_variable(self):
        with ir.NameSpace('dummy_space'):
            var_info = ('var', 'dummy_space', 123, 4, 'float')
            var = ir.Variable(*var_info)
            ir.SymbolTable.add_symbol(var)
            found_variable = ir.SymbolTable.check_symbol(
                'var', 'variables', 'dummy_space')
        assert [var] == found_variable

    def test_check_array(self):
        with ir.NameSpace('dummy_space'):

            array_info = ('array', 'dummy_space', 123, 4)
            array = ir.Array(*array_info)
            ir.SymbolTable.add_symbol(array)
            found_array = ir.SymbolTable.check_symbol('array', 'variables',
                                                      'dummy_space')
        assert [array] == found_array


class TestCheckSymbolAllNamespaces:
    def test_check_argument(self):
        ir.symbol_table.clear()
        namespace_0 = 'dummy_space_0'
        namespace_1 = 'dummy_space_1'
        with ir.NameSpace(namespace_0):
            arg_info = ('arg_0', namespace_0, 123, 4)
            arg_00 = ir.Argument(*arg_info)
            ir.SymbolTable.add_symbol(arg_00)
        with ir.NameSpace(namespace_1):
            arg_info = ('arg_0', namespace_1, 123, 4)
            arg_01 = ir.Argument(*arg_info)
            ir.SymbolTable.add_symbol(arg_01)
        with ir.NameSpace(namespace_0):
            arg_info = ('arg_1', namespace_0, 123, 4)
            arg_10 = ir.Argument(*arg_info)
            ir.SymbolTable.add_symbol(arg_10)
        found_arg = ir.SymbolTable.check_symbol('arg_0', 'arguments')
        expected = [(namespace_0, [arg_00]), (namespace_1, [arg_01])]
        pairs = zip(expected, found_arg)
        for pair in pairs:
            assert all([a[0] == b[0] for a, b in zip(pair[0], pair[1])])

    def test_check_function(self):
        ir.symbol_table.clear()
        namespace_0 = 'dummy_space_0'
        namespace_1 = 'dummy_space_1'
        with ir.NameSpace(namespace_0):
            func_info = ('func_0', namespace_0, 123, 4)
            func_00 = ir.Function(*func_info)
            ir.SymbolTable.add_symbol(func_00)
        with ir.NameSpace(namespace_1):
            func_info = ('func_0', namespace_1, 123, 4)
            func_01 = ir.Function(*func_info)
            ir.SymbolTable.add_symbol(func_01)
        with ir.NameSpace(namespace_0):
            func_info = ('func_1', namespace_0, 123, 4)
            func_10 = ir.Function(*func_info)
            ir.SymbolTable.add_symbol(func_10)
        found_arg = ir.SymbolTable.check_symbol('func_0', 'functions')
        expected = [(namespace_0, [func_00]), (namespace_1, [func_01])]
        pairs = zip(expected, found_arg)
        for pair in pairs:
            assert all([a[0] == b[0] for a, b in zip(pair[0], pair[1])])

    def test_check_variable(self):
        ir.symbol_table.clear()
        namespace_0 = 'dummy_space_0'
        namespace_1 = 'dummy_space_1'
        with ir.NameSpace(namespace_0):
            var_info = ('var_0', namespace_0, 123, 4)
            var_00 = ir.Variable(*var_info)
            ir.SymbolTable.add_symbol(var_00)
        with ir.NameSpace(namespace_1):
            var_info = ('var_0', namespace_1, 123, 4)
            var_01 = ir.Variable(*var_info)
            ir.SymbolTable.add_symbol(var_01)
        with ir.NameSpace(namespace_0):
            var_info = ('var_1', namespace_0, 123, 4)
            var_10 = ir.Variable(*var_info)
            ir.SymbolTable.add_symbol(var_10)
        found_arg = ir.SymbolTable.check_symbol('var_0', 'variables')
        expected = [(namespace_0, [var_00]), (namespace_1, [var_01])]
        pairs = zip(expected, found_arg)
        for pair in pairs:
            assert all([a[0] == b[0] for a, b in zip(pair[0], pair[1])])

    def test_check_array(self):
        ir.symbol_table.clear()
        namespace_0 = 'dummy_space_0'
        namespace_1 = 'dummy_space_1'
        with ir.NameSpace(namespace_0):
            array_info = ('array_0', namespace_0, 123, 4)
            array_00 = ir.Array(*array_info)
            ir.SymbolTable.add_symbol(array_00)
        with ir.NameSpace(namespace_1):
            array_info = ('array_0', namespace_1, 123, 4)
            array_01 = ir.Array(*array_info)
            ir.SymbolTable.add_symbol(array_01)
        with ir.NameSpace(namespace_0):
            array_info = ('array_1', namespace_0, 123, 4)
            array_10 = ir.Array(*array_info)
            ir.SymbolTable.add_symbol(array_10)
        found_arg = ir.SymbolTable.check_symbol('array_0', 'variables')
        expected = [(namespace_0, [array_00]), (namespace_1, [array_01])]
        pairs = zip(expected, found_arg)
        for pair in pairs:
            assert all([a[0] == b[0] for a, b in zip(pair[0], pair[1])])
