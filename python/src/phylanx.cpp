//  Copyright (c) 2017 Hartmut Kaiser
//  Copyright (c) 2018 R. Tohid
//  Copyright (c) 2018 Steven R. Brandt
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include <hpx/runtime/components/new.hpp>
#include <hpx/runtime/threads/run_as_hpx_thread.hpp>
#include <hpx/util/detail/pp/stringize.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// Pybind11 V2.3 changed the interface for the description strings
#if defined(PYBIND11_DESCR)
#define PHYLANX_PYBIND_DESCR PYBIND11_DESCR
#define PHYLANX_PYBIND_DESCR_NAME name()
#else
#define PHYLANX_PYBIND_DESCR constexpr auto
#define PHYLANX_PYBIND_DESCR_NAME name
#endif

// older versions of pybind11 don't support variant-like types
namespace pybind11 { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    // Expose phylanx::util::variant and phylanx::ast::parser::extended_variant

#if !(defined(_MSC_VER) && _MSC_VER >= 1912 && defined(_HAS_CXX17) && (_HAS_CXX17 != 0))
    template <typename... Ts>
    struct type_caster<phylanx::util::variant<Ts...>>
      : variant_caster<phylanx::util::variant<Ts...>>
    {
    };
#endif

    template <typename... Ts>
    struct type_caster<phylanx::ast::parser::extended_variant<Ts...>>
      : variant_caster<phylanx::ast::parser::extended_variant<Ts...>>
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    // Expose phylanx::util::recursive_wrapper
    template <typename T>
    PHYLANX_PYBIND_DESCR get_name()
    {
        return make_caster<T>::PHYLANX_PYBIND_DESCR_NAME;
    }

    // specialize get_name for vector<primitive_argument_type> to avoid infinite recursion
    template <>
    PHYLANX_PYBIND_DESCR
    get_name<std::vector<phylanx::execution_tree::primitive_argument_type>>()
    {
#if defined(_DEBUG)
        return _("List[_phylanxd.execution_tree.primitive_argument_type]");
#else
        return _("List[_phylanx.execution_tree.primitive_argument_type]");
#endif
    }

    template <>
    PHYLANX_PYBIND_DESCR
    get_name<phylanx::execution_tree::primitive_argument_type>()
    {
#if defined(_DEBUG)
        return _("_phylanxd.execution_tree.primitive_argument_type");
#else
        return _("_phylanx.execution_tree.primitive_argument_type");
#endif
    }


    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    class type_caster<phylanx::ir::node_data<T>>
    {
        bool load0d(handle src, bool convert)
        {
            auto caster = make_caster<T>();
            if (caster.load(src, convert))
            {
                value = cast_op<T>(caster);
                return true;
            }
            return false;
        }

        bool load1d(handle src, bool convert)
        {
            auto caster = make_caster<std::vector<T>>();
            if (caster.load(src, convert))
            {
                value = cast_op<std::vector<T>>(caster);
                return true;
            }
            return false;
        }

        bool load2d(handle src, bool convert)
        {
            auto caster = make_caster<std::vector<std::vector<T>>>();
            if (caster.load(src, convert))
            {
                value = cast_op<std::vector<std::vector<T>>>(caster);
                return true;
            }
            return false;
        }

    public:
        bool load(handle src, bool convert)
        {
            return load0d(src, convert) || load1d(src, convert) ||
                load2d(src, convert);
        }

        template <typename NodeData>
        static handle cast(
            NodeData&& src, return_value_policy policy, handle parent)
        {
            switch (src.index())
            {
            case 0:                     // T
                return make_caster<T>::cast(
                    std::forward<NodeData>(src).scalar(), policy, parent);

            case 1: HPX_FALLTHROUGH;    // blaze::DynamicVector<T>
            case 3:                     // blaze::CustomVector<T>
                return make_caster<std::vector<T>>::cast(
                    std::forward<NodeData>(src).as_vector(), policy, parent);

            case 2: HPX_FALLTHROUGH;    // blaze::DynamicMatrix<T>
            case 4:                     // blaze::CustomMatrix<T>
                return make_caster<std::vector<std::vector<T>>>::cast(
                    std::forward<NodeData>(src).as_matrix(), policy, parent);

            default:
                break;
            }
            return handle();
        }

        using Type = phylanx::ir::node_data<T>;
        PYBIND11_TYPE_CASTER(Type, _("node_data[") + get_name<T>() + _("]"));
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    class type_caster<phylanx::util::recursive_wrapper<T>>
    {
    private:
        using caster_t = make_caster<T>;
        caster_t subcaster;

        using subcaster_cast_op_type =
            typename caster_t::template cast_op_type<T>;

    public:
        bool load(handle src, bool convert)
        {
            return subcaster.load(src, convert);
        }

        static PHYLANX_PYBIND_DESCR PHYLANX_PYBIND_DESCR_NAME =
            _("recursive_wrapper[") + get_name<T>() + _("]");

        static handle cast(phylanx::util::recursive_wrapper<T> const& src,
            return_value_policy policy, handle parent)
        {
            // It is definitely wrong to take ownership of this pointer,
            // so mask that rvp
            if (policy == return_value_policy::take_ownership ||
                policy == return_value_policy::automatic)
            {
                policy = return_value_policy::automatic_reference;
            }
            return caster_t::cast(&src.get(), policy, parent);
        }
        static handle cast(phylanx::util::recursive_wrapper<T> && src,
            return_value_policy policy, handle parent)
        {
            // It is definitely wrong to take ownership of this pointer,
            // so mask that rvp
            if (policy == return_value_policy::take_ownership ||
                policy == return_value_policy::automatic)
            {
                policy = return_value_policy::automatic_reference;
            }
            return caster_t::cast(&src.get(), policy, parent);
        }

        template <typename T_>
        using cast_op_type = phylanx::util::recursive_wrapper<T>;

        operator phylanx::util::recursive_wrapper<T>()
        {
            return phylanx::util::recursive_wrapper<T>(
                subcaster.operator subcaster_cast_op_type&());
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename Derived, typename Variant>
    struct variant_caster_helper;

    template <typename Derived, template <typename...> class V, typename... Ts>
    struct variant_caster_helper<Derived, V<Ts...>>
    {
        PYBIND11_TYPE_CASTER(Derived,
            _("Union[") + detail::concat(make_caster<Ts>::name...) + _("]"));

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

        template <typename U, typename... Us>
        bool load_alternative(handle src, bool convert,
            type_list<phylanx::util::recursive_wrapper<U>, Us...>)
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
            // Do a first pass without conversions to improve constructor resolution.
            // E.g. `py::int_(1).cast<variant<double, int>>()` needs to fill the `int`
            // slot of the variant. Without two-pass loading `double` would be filled
            // because it appears first and a conversion is possible.
            if (convert && load_alternative(src, false, type_list<Ts...>{}))
                return true;
            return load_alternative(src, convert, type_list<Ts...>{});
        }

        template <typename Derived_>
        static handle cast(
            Derived_&& src, return_value_policy policy, handle parent)
        {
            return visit_helper<V>::call(variant_caster_visitor{policy, parent},
                                         std::forward<Derived_>(src));
        }
    };

    template <>
    class type_caster<phylanx::execution_tree::primitive_argument_type>
    {
    private:
        using caster_t = variant_caster_helper<
            phylanx::execution_tree::primitive_argument_type,
            phylanx::execution_tree::argument_value_type>;

    public:
        static PHYLANX_PYBIND_DESCR PHYLANX_PYBIND_DESCR_NAME =
            get_name<phylanx::execution_tree::primitive_argument_type>();

        bool load(handle src, bool convert)
        {
            return subcaster.load(src, convert);
        }

        template <typename T>
        static handle cast(T&& src, return_value_policy policy, handle parent)
        {
            return caster_t::cast(std::forward<T>(src), policy, parent);
        }

        template <typename T>
        using cast_op_type = typename caster_t::template cast_op_type<T>;

        operator phylanx::execution_tree::primitive_argument_type*()
        {
            return subcaster.operator
                phylanx::execution_tree::primitive_argument_type*();
        }
        operator phylanx::execution_tree::primitive_argument_type&()
        {
            return subcaster.operator
                phylanx::execution_tree::primitive_argument_type&();
        }
        operator phylanx::execution_tree::primitive_argument_type&&() &&
        {
            return std::move(subcaster).operator
                phylanx::execution_tree::primitive_argument_type &&();
        }

    private:
        caster_t subcaster;
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
            auto d =
                func_.attr("__class__").attr("__dict__").cast<pybind11::dict>();
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
            auto d =
                func_.attr("__class__").attr("__dict__").cast<pybind11::dict>();
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
    template <typename T>
    std::string as_string(T const& value)
    {
        std::stringstream strm;
        strm << value;
        return strm.str();
    }

    // support for __repr__
    template <typename T>
    std::string repr(T const& value)
    {
        std::stringstream strm;
        strm << phylanx::util::repr << value << phylanx::util::norepr;
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

    ///////////////////////////////////////////////////////////////////////////
    phylanx::execution_tree::primitive generate_tree(std::string const& expr,
        pybind11::dict const& dict)
    {
        std::vector<phylanx::execution_tree::primitive_argument_type> args;
        args.reserve(dict.size());

        for (auto const& item : dict)
        {
            phylanx::execution_tree::primitive value =
                item.second.cast<phylanx::execution_tree::primitive>();
            args.emplace_back(std::move(value));
        }

        phylanx::execution_tree::compiler::function_list snippets;
        auto f = phylanx::execution_tree::compile(expr, snippets);

        return phylanx::execution_tree::primitive_operand(f.arg_);
    }
}}

///////////////////////////////////////////////////////////////////////////////
void init_hpx_runtime();
void stop_hpx_runtime();

char const* const expression_compiler_help =
    "compile and evaluate a numerical expression in Phylanx lisp";

phylanx::execution_tree::primitive_argument_type
expression_compiler(std::string xexpr, pybind11::args args)
{
    namespace et = phylanx::execution_tree;
    return hpx::threads::run_as_hpx_thread(
        [&]() -> et::primitive_argument_type
        {
            try
            {
                et::compiler::function_list eval_snippets;
                auto x = et::compile(xexpr, eval_snippets);

                std::vector<phylanx::execution_tree::primitive_argument_type>
                    fargs;
                fargs.reserve(args.size());

                for (auto const& item : args)
                {
                    phylanx::execution_tree::primitive_argument_type value =
                        item.cast<
                            phylanx::execution_tree::primitive_argument_type>();
                    fargs.emplace_back(std::move(value));
                }

                return x(std::move(fargs));
            }
            catch (std::exception const& ex)
            {
                PyErr_SetString(PyExc_RuntimeError, ex.what());
            }
            catch (...)
            {
                PyErr_SetString(PyExc_RuntimeError, "Unknown exception");
            }
            return {};
    });
};

///////////////////////////////////////////////////////////////////////////////
#if defined(_DEBUG)
PYBIND11_MODULE(_phylanxd, m)
#else
PYBIND11_MODULE(_phylanx, m)
#endif
{
    m.doc() = "Phylanx plugin module";

    m.attr("__version__") = pybind11::str(
        HPX_PP_STRINGIZE(PHYLANX_VERSION_MAJOR) "."
        HPX_PP_STRINGIZE(PHYLANX_VERSION_MINOR) "."
        HPX_PP_STRINGIZE(PHYLANX_VERSION_SUBMINOR) "."
        PHYLANX_HAVE_GIT_COMMIT);

    ///////////////////////////////////////////////////////////////////////////
    // expose version functions
    m.def("major_version", &phylanx::major_version);
    m.def("minor_version", &phylanx::minor_version);
    m.def("subminor_version", &phylanx::subminor_version);
    m.def("full_version", &phylanx::full_version);
    m.def("full_version_as_string", &phylanx::full_version_as_string);

    m.def("init_hpx_runtime", &init_hpx_runtime);
    m.def("stop_hpx_runtime", &stop_hpx_runtime);

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
        [](std::string const& code) {
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
    // expose execution tree
    auto execution_tree = m.def_submodule("execution_tree");

    execution_tree.def("generate_tree", &phylanx::bindings::generate_tree,
        "generate expression tree from given expression");

    execution_tree.def("var",
        [](double d) {
            return hpx::threads::run_as_hpx_thread(
                [&]()
                {
                    using namespace phylanx::execution_tree;
                    return create_primitive_component(hpx::find_here(),
                        "variable", primitive_argument_type{d});
                });
        },
        "create a new variable from a floating point value");
    execution_tree.def("var",
        [](const std::string& d) {
            return hpx::threads::run_as_hpx_thread(
                [&]()
                {
                    using namespace phylanx::execution_tree;
                    return create_primitive_component(hpx::find_here(),
                        "variable", primitive_argument_type{d});
                });
        },
        "create a new variable from a string");
    execution_tree.def("var",
        [](const std::vector<double>& d) {
            return hpx::threads::run_as_hpx_thread(
                [&]()
                {
                    using namespace phylanx::execution_tree;
                    return create_primitive_component(hpx::find_here(),
                        "variable", primitive_argument_type{
                            phylanx::ir::node_data<double>{d}});
                });
        },
        "create a new variable from a vector floating point values");
    execution_tree.def("var",
        [](const std::vector<std::vector<double>>& d) {
            return hpx::threads::run_as_hpx_thread(
                [&]()
                {
                    using namespace phylanx::execution_tree;
                    return create_primitive_component(hpx::find_here(),
                        "variable", primitive_argument_type{
                            phylanx::ir::node_data<double>{d}});
                });
        },
        "create a new variable from a matrix floating point values");

    execution_tree.def("eval", expression_compiler, expression_compiler_help);

    pybind11::class_<phylanx::execution_tree::primitive>(execution_tree,
        "primitive", "type representing an arbitrary execution tree")
        .def(pybind11::init<>())
        .def("eval", [](phylanx::execution_tree::primitive const& p)
            {
                return hpx::threads::run_as_hpx_thread(
                    [&]() {
                        using namespace phylanx::execution_tree;
                        return numeric_operand(primitive_argument_type{p}, {})
                            .get();
                    });
            },
            "evaluate execution tree")
        .def("assign", [](phylanx::execution_tree::primitive p, double d)
            {
                hpx::threads::run_as_hpx_thread(
                    [&]() {
                        using namespace phylanx::execution_tree;
                        p.store(hpx::launch::sync, primitive_argument_type{d});
                    });
            },
            "assign another value to variable")
        .def("__str__",
            &phylanx::bindings::as_string<phylanx::execution_tree::primitive>)
        .def("__repr__",
            &phylanx::bindings::repr<phylanx::execution_tree::primitive>)
    ;

    ///////////////////////////////////////////////////////////////////////////
    // expose util submodule
    auto util = m.def_submodule("util");

    util.def("serialize", &phylanx::bindings::serialize<phylanx::ast::nil>,
        "serialize an AST optoken object into a byte-stream");
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
    util.def("serialize", &phylanx::bindings::serialize<phylanx::ast::function_call>,
        "serialize an AST expression object into a byte-stream");

    util.def("serialize", &phylanx::bindings::serialize<phylanx::ir::node_data<double>>,
        "serialize a node_data<double> expression object into a byte-stream");
    util.def("serialize", &phylanx::bindings::serialize<phylanx::ir::node_data<bool>>,
        "serialize a node_data<bool> expression object into a byte-stream");

    util.def("unserialize", &phylanx::util::unserialize,
        "un-serialize a byte-stream into a Phylanx object");
}
