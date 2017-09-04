//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include <hpx/util/detail/pp/stringize.hpp>

#if defined(_DEBUG)
#define PHYLANX_MODULE_NAME phylanxd
#else
#define PHYLANX_MODULE_NAME phylanx
#endif

// older versions of pybind11 don't support variant-like types
namespace pybind11 { namespace detail
{
#if !defined(PYBIND11_HAS_VARIANT)
    ///////////////////////////////////////////////////////////////////////////
    // Expose phylanx::util::variant -- can be any `std::variant`-like container

    // Visit a variant and cast any found type to Python
    struct variant_caster_visitor
    {
        return_value_policy policy;
        handle parent;

        using result_type = handle; // required by boost::variant in C++11

        template <typename T>
        result_type operator()(T &&src) const
        {
            return make_caster<T>::cast(std::forward<T>(src), policy, parent);
        }
    };

    // Helper class which abstracts away variant's `visit` function.
    // `std::variant` and similar `namespace::variant` types which provide a
    // `namespace::visit()` function are handled here automatically using
    // argument-dependent lookup. Users can provide specializations for other
    // variant-like classes, e.g. `boost::variant` and `boost::apply_visitor`.
    template <template<typename...> class Variant>
    struct visit_helper
    {
        template <typename... Args>
        static auto call(Args &&...args)
        ->  decltype(visit(std::forward<Args>(args)...))
        {
            return visit(std::forward<Args>(args)...);
        }
    };

    // Generic variant caster
    template <typename Variant> struct variant_caster;

    template <template <typename...> class V, typename... Ts>
    struct variant_caster<V<Ts...>>
    {
        static_assert(sizeof...(Ts) > 0,
            "Variant must consist of at least one alternative.");

        template <typename U, typename... Us>
        bool load_alternative(handle src, bool convert, type_list<U, Us...>)
        {
            auto caster = make_caster<U>();
            if (caster.load(src, convert))
            {
                value = cast_op<U>(caster);
                return true;
            }
            return load_alternative(src, convert, type_list<Us...>{});
        }

        bool load_alternative(handle, bool, type_list<>) { return false; }

        bool load(handle src, bool convert)
        {
            // Do a first pass without conversions to improve constructor
            // resolution.
            // E.g. `py::int_(1).cast<variant<double, int>>()` needs to fill
            // the `int` slot of the variant. Without two-pass loading `double`
            // would be filled because it appears first and a conversion is
            // possible.
            if (convert && load_alternative(src, false, type_list<Ts...>{}))
                return true;
            return load_alternative(src, convert, type_list<Ts...>{});
        }

        template <typename Variant>
        static handle cast(
            Variant&& src, return_value_policy policy, handle parent)
        {
            return visit_helper<V>::call(
                variant_caster_visitor{policy, parent},
                std::forward<Variant>(src));
        }

        using Type = V<Ts...>;

        PYBIND11_TYPE_CASTER(Type,
            _("Union[") + detail::concat(make_caster<Ts>::name()...) + _("]"));
    };
#endif

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
PYBIND11_PLUGIN(PHYLANX_MODULE_NAME)
{
    pybind11::module m(
        HPX_PP_STRINGIZE(PHYLANX_MODULE_NAME),
        "Phylanx plugin module");

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
        .value("op_post_decr", phylanx::ast::optoken::op_post_decr);

    // phylanx::ast::identifier
    pybind11::class_<phylanx::ast::identifier>(
            ast, "identifier", "AST node representing an identifier")
        .def(pybind11::init<std::string const&>(),
            "initialize identifier instance with a name")
        .def_readonly("name", &phylanx::ast::identifier::name,
            "the name of the identifier")
        .def(pybind11::self == pybind11::self)
        .def(pybind11::self != pybind11::self);

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
        .def_property_readonly("value", &phylanx::ast::primary_expr::value,
            "access the current primary_expr's value")
        .def(pybind11::self == pybind11::self)
        .def(pybind11::self != pybind11::self);

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
        .def_property_readonly("value", &phylanx::ast::operand::value,
            "access the current operand's value")
        .def(pybind11::self == pybind11::self)
        .def(pybind11::self != pybind11::self);

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
        .def(pybind11::self != pybind11::self);

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
        .def(pybind11::self != pybind11::self);

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
        .def(pybind11::self != pybind11::self);

    ast.def("generate_ast", &phylanx::ast::generate_ast,
        "generate an AST from the given expression string");

    ///////////////////////////////////////////////////////////////////////////
    // expose util submodule
    auto util = m.def_submodule("util");

    util.def("serialize",
        [](phylanx::ast::identifier const& ast)
        {
            return phylanx::util::serialize(ast);
        },
        "serialize an AST expression object into a byte-stream");
    util.def("serialize",
        [](phylanx::ast::expression const& ast)
        {
            return phylanx::util::serialize(ast);
        },
        "serialize an AST expression object into a byte-stream");

    util.def("unserialize",
        [](std::vector<char> const& data) -> phylanx::ast::expression
        {
            phylanx::ast::expression ast;
            phylanx::util::unserialize(data, ast);
            return ast;
        },
        "un-serialize a byte-stream into an AST expression object");

    return m.ptr();
}
