//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include <string>
#include <strstream>
#include <vector>

// older versions of pybind11 don't support variant-like types
namespace pybind11 { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    // Expose phylanx::util::variant and phylanx::ast::parser::extended_variant
    template <typename... Ts>
    struct type_caster<phylanx::util::variant<Ts...>>
      : variant_caster<phylanx::util::variant<Ts...>>
    {
    };

    template <typename... Ts>
    struct type_caster<phylanx::ast::parser::extended_variant<Ts...>>
      : variant_caster<phylanx::ast::parser::extended_variant<Ts...>>
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    // Expose phylanx::util::recursive_wrapper
    template <typename T>
    class type_caster<phylanx::util::recursive_wrapper<T>>
      : public type_caster_base<T>
    {
    public:
        static handle cast(phylanx::util::recursive_wrapper<T> const& src,
            return_value_policy policy, handle parent)
        {
            return type_caster_base<T>::cast(&src.get(), policy, parent);
        }

        template <typename T_>
        using cast_op_type = phylanx::util::recursive_wrapper<T>;

        operator phylanx::util::recursive_wrapper<T>()
        {
            return phylanx::util::recursive_wrapper<T>(*((T*)this->value));
        }
    };
}}

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace bindings
{
    ///////////////////////////////////////////////////////////////////////////
    // support for the traverse API
    struct traverse_helper
    {
        template <typename Ast>
        bool on_enter(Ast const& ast) const
        {
            pybind11::dict d = func_.attr("__class__").attr("__dict__");
            if (d.contains("on_enter"))
            {
                pybind11::object ret =
                    d["on_enter"](func_, ast, *args_, **kwargs_);
                return ret.cast<bool>();
            }
            pybind11::object ret = func_(ast, *args_, **kwargs_);
            return ret.cast<bool>();
        }

        template <typename Ast>
        bool on_exit(Ast const& ast) const
        {
            pybind11::dict d = func_.attr("__class__").attr("__dict__");
            if (d.contains("on_exit"))
            {
                pybind11::object ret =
                    d["on_exit"](func_, ast, *args_, **kwargs_);
                return ret.cast<bool>();
            }
            return true;
        }

        pybind11::object& func_;
        pybind11::args& args_;
        pybind11::kwargs& kwargs_;
    };

    template <typename Ast>
    bool traverse(Ast const& ast, pybind11::object func, pybind11::args args,
        pybind11::kwargs kwargs)
    {
        return phylanx::ast::traverse(ast, traverse_helper{func, args, kwargs});
    }

    // serialization support
    template <typename Ast>
    std::vector<char> serialize(Ast const& ast)
    {
        return phylanx::util::serialize(ast);
    }

    ///////////////////////////////////////////////////////////////////////////
    // support for __str__
    template <typename Ast>
    std::string as_string(Ast const& ast)
    {
        std::stringstream strm;
        strm << ast;
        return strm.str();
    }

    ///////////////////////////////////////////////////////////////////////////
    // pickle support
    template <typename Ast>
    std::vector<char> pickle_helper(Ast const& ast)
    {
        return phylanx::util::serialize(ast);
    }

    template <typename Ast>
    Ast unpickle_helper(std::vector<char> data)
    {
        Ast ast;
        phylanx::util::detail::unserialize(data, ast);
        return ast;
    }
}}

///////////////////////////////////////////////////////////////////////////////
#if defined(_DEBUG)
PYBIND11_MODULE(phylanxd, m)
#else
PYBIND11_MODULE(phylanx, m)
#endif
{
    m.doc() = "Phylanx plugin module";

    ///////////////////////////////////////////////////////////////////////////
    // expose version functions
    m.def("major_version", &phylanx::major_version);
    m.def("minor_version", &phylanx::minor_version);
    m.def("subminor_version", &phylanx::subminor_version);
    m.def("full_version", &phylanx::full_version);
    m.def("full_version_as_string", &phylanx::full_version_as_string);

    ///////////////////////////////////////////////////////////////////////////
    // expose AST submodule
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
        .def(pybind11::pickle(
            &phylanx::bindings::pickle_helper<phylanx::ast::optoken>,
            &phylanx::bindings::unpickle_helper<phylanx::ast::optoken>));

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
        .def(pybind11::pickle(
            &phylanx::bindings::pickle_helper<phylanx::ast::identifier>,
            &phylanx::bindings::unpickle_helper<phylanx::ast::identifier>));

    // phylanx::ast::primary_expr
    pybind11::class_<phylanx::ast::primary_expr>(
            ast, "primary_expr", "AST node representing a primary_expr")
        .def(pybind11::init<bool>(),
            "initialize primary_expr instance with a boolean value")
        .def(pybind11::init<double>(),
            "initialize primary_expr instance with a floating point value")
        .def(pybind11::init<std::string const&>(),
            "initialize primary_expr instance with an identifier name")
        .def(pybind11::init<phylanx::ast::identifier>(),
            "initialize primary_expr instance with an value")
        .def(pybind11::init<phylanx::ast::expression>(),
            "initialize primary_expr instance with an expression value")
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
        .def(pybind11::pickle(
            &phylanx::bindings::pickle_helper<phylanx::ast::primary_expr>,
            &phylanx::bindings::unpickle_helper<phylanx::ast::primary_expr>));

    // phylanx::ast::operand
    pybind11::class_<phylanx::ast::operand>(
            ast, "operand", "AST node representing an operand")
        .def(pybind11::init<double>(),
            "initialize operand instance with a floating point value")
        .def(pybind11::init<std::string const&>(),
            "initialize operand instance with a primary_expr (identifier) name")
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
        .def(pybind11::pickle(
            &phylanx::bindings::pickle_helper<phylanx::ast::unary_expr>,
            &phylanx::bindings::unpickle_helper<phylanx::ast::unary_expr>));

    // phylanx::ast::operation
    pybind11::class_<phylanx::ast::operation>(
            ast, "operation", "AST node representing an operation")
        .def(pybind11::init<phylanx::ast::optoken, phylanx::ast::operand>(),
            "initialize operation instance with an operator and an operand")
        .def_readonly("operator", &phylanx::ast::operation::operator_,
            "the operator of the operation")
        .def_readonly("operand", &phylanx::ast::operation::operand_,
            "the operand of the operation")
        .def(pybind11::self == pybind11::self)
        .def(pybind11::self != pybind11::self)
        .def("__str__", &phylanx::bindings::as_string<phylanx::ast::operation>)
        .def(pybind11::pickle(
            &phylanx::bindings::pickle_helper<phylanx::ast::operation>,
            &phylanx::bindings::unpickle_helper<phylanx::ast::operation>));

    // phylanx::ast::expression
    pybind11::class_<phylanx::ast::expression>(
            ast, "expression", "AST node representing an expression")
        .def(pybind11::init<phylanx::ast::operand>(),
            "initialize operation instance with an operand")
        .def_readonly("first", &phylanx::ast::expression::first,
            "the first operand of the expression")
        .def_readwrite("rest", &phylanx::ast::expression::rest,
            "the (optional) list of operations of the expression")
        .def(pybind11::self == pybind11::self)
        .def(pybind11::self != pybind11::self)
        .def("__str__", &phylanx::bindings::as_string<phylanx::ast::expression>)
        .def(pybind11::pickle(
            &phylanx::bindings::pickle_helper<phylanx::ast::expression>,
            &phylanx::bindings::unpickle_helper<phylanx::ast::expression>));

    // phylanx::ast::generate_ast()
    ast.def("generate_ast", &phylanx::ast::generate_ast,
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
    ast.def("traverse", &phylanx::bindings::traverse<phylanx::ast::operation>,
        "traverse the given AST operation and call the provided function "
            "on each part of it");
    ast.def("traverse", &phylanx::bindings::traverse<phylanx::ast::expression>,
        "traverse the given AST expression and call the provided function "
            "on each part of it");

    ///////////////////////////////////////////////////////////////////////////
    // expose util submodule
    auto util = m.def_submodule("util");

    util.def("serialize", &phylanx::bindings::serialize<phylanx::ast::optoken>,
        "serialize an AST optoken object into a byte-stream");
    util.def("serialize", &phylanx::bindings::serialize<phylanx::ast::identifier>,
        "serialize an AST identifier object into a byte-stream");
    util.def("serialize", &phylanx::bindings::serialize<phylanx::ast::primary_expr>,
        "serialize an AST primary_expr object into a byte-stream");
    util.def("serialize", &phylanx::bindings::serialize<phylanx::ast::operand>,
        "serialize an AST operand object into a byte-stream");
    util.def("serialize", &phylanx::bindings::serialize<phylanx::ast::unary_expr>,
        "serialize an AST unary_expr object into a byte-stream");
    util.def("serialize", &phylanx::bindings::serialize<phylanx::ast::operation>,
        "serialize an AST operation object into a byte-stream");
    util.def("serialize", &phylanx::bindings::serialize<phylanx::ast::expression>,
        "serialize an AST expression object into a byte-stream");

    util.def("unserialize", &phylanx::util::unserialize,
        "un-serialize a byte-stream into an AST expression object");
}
