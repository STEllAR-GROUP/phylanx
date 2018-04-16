//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2018 R. Tohid
//  Copyright (c) 2018 Steven R. Brandt
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <bindings/binding_helpers.hpp>
#include <bindings/type_casters.hpp>

#include <pybind11/pybind11.h>

#include <cstdint>
#include <string>

///////////////////////////////////////////////////////////////////////////////
// expose AST submodule
void phylanx::bindings::bind_ast(pybind11::module m)
{
    auto ast = m.def_submodule("ast");

    pybind11::enum_<phylanx::ast::optoken>(
            ast, "optoken", pybind11::arithmetic(),
        "an enumerator type representing possible operations with operands")
        .value("op_comma", phylanx::ast::optoken::op_comma)
        .value("op_assign", phylanx::ast::optoken::op_assign)
        .value("op_plus_assign", phylanx::ast::optoken::op_plus_assign)
        .value("op_minus_assign", phylanx::ast::optoken::op_minus_assign)
        .value("op_times_assign", phylanx::ast::optoken::op_times_assign)
        .value("op_divide_assign", phylanx::ast::optoken::op_divide_assign)
        .value("op_mod_assign", phylanx::ast::optoken::op_mod_assign)
        .value("op_bit_and_assign", phylanx::ast::optoken::op_bit_and_assign)
        .value("op_bit_xor_assign", phylanx::ast::optoken::op_bit_xor_assign)
        .value("op_bitor_assign", phylanx::ast::optoken::op_bitor_assign)
        .value("op_shift_left_assign", phylanx::ast::optoken::op_shift_left_assign)
        .value("op_shift_right_assign", phylanx::ast::optoken::op_shift_right_assign)
        .value("op_logical_or", phylanx::ast::optoken::op_logical_or)
        .value("op_logical_and", phylanx::ast::optoken::op_logical_and)
        .value("op_bit_or", phylanx::ast::optoken::op_bit_or)
        .value("op_bit_xor", phylanx::ast::optoken::op_bit_xor)
        .value("op_bit_and", phylanx::ast::optoken::op_bit_and)
        .value("op_equal", phylanx::ast::optoken::op_equal)
        .value("op_not_equal", phylanx::ast::optoken::op_not_equal)
        .value("op_less", phylanx::ast::optoken::op_less)
        .value("op_less_equal", phylanx::ast::optoken::op_less_equal)
        .value("op_greater", phylanx::ast::optoken::op_greater)
        .value("op_greater_equal", phylanx::ast::optoken::op_greater_equal)
        .value("op_shift_left", phylanx::ast::optoken::op_shift_left)
        .value("op_shift_right", phylanx::ast::optoken::op_shift_right)
        .value("op_plus", phylanx::ast::optoken::op_plus)
        .value("op_minus", phylanx::ast::optoken::op_minus)
        .value("op_times", phylanx::ast::optoken::op_times)
        .value("op_divide", phylanx::ast::optoken::op_divide)
        .value("op_mod", phylanx::ast::optoken::op_mod)
        .value("op_positive", phylanx::ast::optoken::op_positive)
        .value("op_negative", phylanx::ast::optoken::op_negative)
        .value("op_pre_incr", phylanx::ast::optoken::op_pre_incr)
        .value("op_pre_decr", phylanx::ast::optoken::op_pre_decr)
        .value("op_compl", phylanx::ast::optoken::op_compl)
        .value("op_not", phylanx::ast::optoken::op_not)
        .value("op_post_incr", phylanx::ast::optoken::op_post_incr)
        .value("op_post_decr", phylanx::ast::optoken::op_post_decr)
        .def("__str__", &phylanx::bindings::as_string<phylanx::ast::optoken>)
        .def("__repr__", &phylanx::bindings::repr<phylanx::ast::optoken>)
        .def(pybind11::pickle(
            &phylanx::bindings::pickle_helper<phylanx::ast::optoken>,
            &phylanx::bindings::unpickle_helper<phylanx::ast::optoken>));

    // phylanx::ast::nil
    pybind11::class_<phylanx::ast::nil>(
        ast, "nil", "AST node representing nil")
        .def(pybind11::init<>(), "initialize nil instance")
        .def(pybind11::self == pybind11::self)
        .def(pybind11::self != pybind11::self)
        .def("__str__", &phylanx::bindings::as_string<phylanx::ast::nil>)
        .def("__repr__", &phylanx::bindings::repr<phylanx::ast::nil>)
        .def(pybind11::pickle(
            &phylanx::bindings::pickle_helper<phylanx::ast::nil>,
            &phylanx::bindings::unpickle_helper<phylanx::ast::nil>));

    // phylanx::ast::identifier
    pybind11::class_<phylanx::ast::identifier>(
        ast, "identifier", "AST node representing an identifier")
        .def(pybind11::init<std::string const&>(),
            "initialize identifier instance with a name")
        .def_readonly("name", &phylanx::ast::identifier::name,
            "the name of the identifier")
        .def(pybind11::self == pybind11::self)
        .def(pybind11::self != pybind11::self)
        .def("__str__", &phylanx::bindings::as_string<phylanx::ast::identifier>)
        .def("__repr__", &phylanx::bindings::repr<phylanx::ast::identifier>)
        .def(pybind11::pickle(
            &phylanx::bindings::pickle_helper<phylanx::ast::identifier>,
            &phylanx::bindings::unpickle_helper<phylanx::ast::identifier>));

    // phylanx::ast::primary_expr
    pybind11::class_<phylanx::ast::primary_expr>(
        ast, "primary_expr", "AST node representing a primary_expr")
        .def(pybind11::init<bool>(),
            "initialize primary_expr instance with a Boolean value")
        .def(pybind11::init<std::int64_t>(),
            "initialize primary_expr instance with an integer value")
        .def(pybind11::init<double>(),
            "initialize primary_expr instance with a floating point value")
        .def(pybind11::init<std::string const&>(),
            "initialize primary_expr instance with an identifier name")
        .def(pybind11::init<phylanx::ast::identifier>(),
            "initialize primary_expr instance with an value")
        .def(pybind11::init<phylanx::ast::expression>(),
            "initialize primary_expr instance with an expression value")
        .def(pybind11::init<phylanx::ast::function_call>(),
            "initialize primary_expr instance with a function call")
        .def_property_readonly("value",
            [](phylanx::ast::primary_expr const& pe)
            ->  phylanx::ast::expr_node_type const&
            {
                return pe;
            },
            "access the current primary_expr's value")
        .def(pybind11::self == pybind11::self)
        .def(pybind11::self != pybind11::self)
        .def("__str__", &phylanx::bindings::as_string<phylanx::ast::primary_expr>)
        .def("__repr__", &phylanx::bindings::repr<phylanx::ast::primary_expr>)
        .def(pybind11::pickle(
            &phylanx::bindings::pickle_helper<phylanx::ast::primary_expr>,
            &phylanx::bindings::unpickle_helper<phylanx::ast::primary_expr>));

    // phylanx::ast::operand
    pybind11::class_<phylanx::ast::operand>(
        ast, "operand", "AST node representing an operand")
        .def(pybind11::init<bool>(),
            "initialize operand instance with a Boolean value")
        .def(pybind11::init<std::int64_t>(),
            "initialize operand instance with an integer value")
        .def(pybind11::init<double>(),
            "initialize operand instance with a floating point value")
        .def(pybind11::init<std::string const&>(),
            "initialize operand instance with a primary_expr (identifier) name")
        .def(pybind11::init<phylanx::ast::identifier>(),
            "initialize operand instance with a primary_expr value")
        .def(pybind11::init<phylanx::ast::primary_expr>(),
            "initialize operand instance with a primary_expr value")
        .def(pybind11::init<phylanx::ast::unary_expr>(),
            "initialize operand instance with a unary_expr value")
        .def_property_readonly("value",
            [](phylanx::ast::operand const& op)
            ->  phylanx::ast::operand_node_type const&
            {
                return op;
            },
            "access the current operand's value")
        .def(pybind11::self == pybind11::self)
        .def(pybind11::self != pybind11::self)
        .def("__str__", &phylanx::bindings::as_string<phylanx::ast::operand>)
        .def("__repr__", &phylanx::bindings::repr<phylanx::ast::operand>)
        .def(pybind11::pickle(
            &phylanx::bindings::pickle_helper<phylanx::ast::operand>,
            &phylanx::bindings::unpickle_helper<phylanx::ast::operand>));

    // phylanx::ast::unary_expr
    pybind11::class_<phylanx::ast::unary_expr>(
        ast, "unary_expr", "AST node representing an unary_expr")
        .def(pybind11::init<phylanx::ast::optoken, phylanx::ast::operand>(),
            "initialize unary_expr instance with an operator and an operand")
        .def_readonly("operator", &phylanx::ast::unary_expr::operator_,
            "the operator of the unary_expr")
        .def_readonly("operand", &phylanx::ast::unary_expr::operand_,
            "the operand of the unary_expr")
        .def(pybind11::self == pybind11::self)
        .def(pybind11::self != pybind11::self)
        .def("__str__", &phylanx::bindings::as_string<phylanx::ast::unary_expr>)
        .def("__repr__", &phylanx::bindings::repr<phylanx::ast::unary_expr>)
        .def(pybind11::pickle(
            &phylanx::bindings::pickle_helper<phylanx::ast::unary_expr>,
            &phylanx::bindings::unpickle_helper<phylanx::ast::unary_expr>));

    // phylanx::ast::operation
    pybind11::class_<phylanx::ast::operation>(
        ast, "operation", "AST node representing an operation")
        .def(pybind11::init<phylanx::ast::optoken, phylanx::ast::identifier>(),
            "initialize operation instance with an operator and an operand")
        .def(pybind11::init<phylanx::ast::optoken, phylanx::ast::operand>(),
            "initialize operation instance with an operator and an operand")
        .def_readonly("operator", &phylanx::ast::operation::operator_,
            "the operator of the operation")
        .def_readonly("operand", &phylanx::ast::operation::operand_,
            "the operand of the operation")
        .def(pybind11::self == pybind11::self)
        .def(pybind11::self != pybind11::self)
        .def("__str__", &phylanx::bindings::as_string<phylanx::ast::operation>)
        .def("__repr__", &phylanx::bindings::repr<phylanx::ast::operation>)
        .def(pybind11::pickle(
            &phylanx::bindings::pickle_helper<phylanx::ast::operation>,
            &phylanx::bindings::unpickle_helper<phylanx::ast::operation>));

    // phylanx::ast::expression
    pybind11::class_<phylanx::ast::expression>(
        ast, "expression", "AST node representing an expression")
        .def(pybind11::init<phylanx::ast::operand>(),
            "initialize expression instance with an operand")
        .def(pybind11::init<phylanx::ast::identifier>(),
            "initialize expression instance with an identifier")
        .def(pybind11::init<phylanx::ast::primary_expr>(),
            "initialize expression instance with a primary expression")
        .def(pybind11::init<phylanx::ast::primary_expr>(),
            "initialize expression instance with a unary expression")
        .def(pybind11::init<phylanx::ast::function_call>(),
            "initialize expression instance with a unary expression")
        .def(pybind11::init<bool>(),
            "initialize expression instance with a Boolean value")
        .def("append",
            [](phylanx::ast::expression& e, phylanx::ast::operation const& op)
            {
                e.rest.push_back(op);
            },
            "append a operand to this expression")
        .def_readonly("first", &phylanx::ast::expression::first,
            "the first operand of the expression")
        .def_readwrite("rest", &phylanx::ast::expression::rest,
            "the (optional) list of operations of the expression")
        .def(pybind11::self == pybind11::self)
        .def(pybind11::self != pybind11::self)
        .def("__str__", &phylanx::bindings::as_string<phylanx::ast::expression>)
        .def("__repr__", &phylanx::bindings::repr<phylanx::ast::expression>)
        .def(pybind11::pickle(
            &phylanx::bindings::pickle_helper<phylanx::ast::expression>,
            &phylanx::bindings::unpickle_helper<phylanx::ast::expression>));

    // phylanx::ast::function_call
    pybind11::class_<phylanx::ast::function_call>(
        ast, "function_call", "AST node representing a function_call")
        .def(pybind11::init<phylanx::ast::identifier>(),
            "initialize function_call instance with an identifier")
        .def_readonly("name", &phylanx::ast::function_call::function_name,
            "the name of the function to invoke")
        .def_readwrite("args", &phylanx::ast::function_call::args,
            "the (optional) list of arguments for the function")
        .def("append",
            [](phylanx::ast::function_call& e,
               phylanx::ast::expression const& expr)
            {
                e.args.push_back(expr);
            },
            "append a operand to this expression")
        .def(pybind11::self == pybind11::self)
        .def(pybind11::self != pybind11::self)
        .def("__str__", &phylanx::bindings::as_string<phylanx::ast::function_call>)
        .def("__repr__", &phylanx::bindings::repr<phylanx::ast::function_call>)
        .def(pybind11::pickle(
            &phylanx::bindings::pickle_helper<phylanx::ast::function_call>,
            &phylanx::bindings::unpickle_helper<phylanx::ast::function_call>));

    ///////////////////////////////////////////////////////////////////////////
    // phylanx::ast::generate_ast()
    ast.def("generate_ast",
        [](std::string const& code)
        {
            pybind11::gil_scoped_release release;       // release GIL
            return phylanx::ast::generate_ast(code);
        },
        "generate an AST from the given expression string");

    // phylanx::ast::traverse()
    ast.def("traverse", &phylanx::bindings::traverse<phylanx::ast::optoken>,
        "traverse the given AST optoken and call the provided function "
        "on each part of it");
    ast.def("traverse", &phylanx::bindings::traverse<phylanx::ast::identifier>,
        "traverse the given AST identifier and call the provided function "
        "on each part of it");
    ast.def("traverse", &phylanx::bindings::traverse<phylanx::ast::primary_expr>,
        "traverse the given AST primary_expr and call the provided function "
        "on each part of it");
    ast.def("traverse", &phylanx::bindings::traverse<phylanx::ast::operand>,
        "traverse the given AST operand and call the provided function "
        "on each part of it");
    ast.def("traverse", &phylanx::bindings::traverse<phylanx::ast::unary_expr>,
        "traverse the given AST unary_expr and call the provided function "
        "on each part of it");
    ast.def("traverse", &phylanx::bindings::traverse<phylanx::ast::expression>,
        "traverse the given AST expression and call the provided function "
        "on each part of it");
    ast.def("traverse", &phylanx::bindings::traverse<phylanx::ast::function_call>,
        "traverse the given AST expression and call the provided function "
        "on each part of it");

    ///////////////////////////////////////////////////////////////////////////
    // Compiler State
    pybind11::class_<phylanx::bindings::compiler_state>(m, "compiler_state")
        .def(pybind11::init<>());
}
