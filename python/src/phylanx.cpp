//  Copyright (c) 2017 Hartmut Kaiser
//  Copyright (c) 2018 R. Tohid
//  Copyright (c) 2018 Steven R. Brandt
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <hpx/runtime/components/new.hpp>
#include <hpx/runtime/threads/run_as_hpx_thread.hpp>

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <phylanx/execution_tree/primitives/file_read_csv.hpp>
#include <phylanx/execution_tree/primitives/slicing_operation.hpp>

// See http://pybind11.readthedocs.io/en/stable/advanced/cast/stl.html
PYBIND11_MAKE_OPAQUE(std::vector<phylanx::ast::operation>);
PYBIND11_MAKE_OPAQUE(std::vector<phylanx::ast::expression>);

// older versions of pybind11 don't support variant-like types
namespace pybind11 {
namespace detail {
#if !defined(_MSC_VER) || _MSC_VER < 1912
    ///////////////////////////////////////////////////////////////////////////
    // Expose phylanx::util::variant and phylanx::ast::parser::extended_variant
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
            return phylanx::util::recursive_wrapper<T>(*((T*) this->value));
        }
    };
}
}

///////////////////////////////////////////////////////////////////////////////
namespace phylanx {
namespace bindings {

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

    ///////////////////////////////////////////////////////////////////////////
    phylanx::execution_tree::primitive generate_tree(std::string const& expr,
        pybind11::dict const& dict)
    {
        std::vector<phylanx::execution_tree::primitive_argument_type> args;
        args.reserve(dict.size());

        for (auto const& item : dict)
        {
            std::string key = pybind11::str(item.first);
            phylanx::execution_tree::primitive value =
                item.second.cast<phylanx::execution_tree::primitive>();
            args.emplace_back(std::move(value));
        }

        phylanx::execution_tree::compiler::function_list snippets;
        auto f = phylanx::execution_tree::compile(expr, snippets);

        return phylanx::execution_tree::primitive_operand(f.arg_);
    }
}
}

///////////////////////////////////////////////////////////////////////////////
void init_hpx_runtime();
void stop_hpx_runtime();

const char* expression_compiler_help =
    "compile and evaluate a numerical expression in Phylanx lisp";
template <typename... Ts>
typename hpx::util::invoke_result<phylanx::execution_tree::primitive(Ts...),
    Ts...>::type
expression_compiler(std::string xexpr, Ts const&... ts)
{
    return hpx::threads::run_as_hpx_thread([&]() {
        using namespace phylanx::execution_tree;
        try
        {
            phylanx::execution_tree::compiler::function_list eval_snippets;
            auto x = phylanx::execution_tree::compile(
                xexpr, eval_snippets);    //, env);

            phylanx::execution_tree::primitive_argument_type result;
            result = x(ts...);
            int ndx = result.index();
            if (ndx == 4)
            {
                auto node_result =
                    phylanx::util::get<phylanx::ir::node_data<double>>(result);
                return primitive{
                    hpx::local_new<primitives::variable>(node_result)};
            }
            else if (ndx == 2)
            {
                auto node_result = phylanx::util::get<std::int64_t>(result);
                return primitive{
                    hpx::local_new<primitives::variable>(node_result)};
            }
        }
        catch (const std::exception& ex)
        {
            PyErr_SetString(PyExc_RuntimeError, ex.what());
        }
        catch (...)
        {
            PyErr_SetString(PyExc_RuntimeError, "Unknown exception");
        }
        std::int64_t node_result = 0;
        return primitive{hpx::local_new<primitives::variable>(node_result)};
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
        HPX_PP_STRINGIZE(PHYLANX_VERSION_MAJOR) "." HPX_PP_STRINGIZE(
            PHYLANX_VERSION_MINOR) "." HPX_PP_STRINGIZE(PHYLANX_VERSION_SUBMINOR));

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

    pybind11::enum_<phylanx::ast::optoken>(ast, "optoken",
        pybind11::arithmetic(),
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
        .value(
            "op_shift_left_assign", phylanx::ast::optoken::op_shift_left_assign)
        .value("op_shift_right_assign",
            phylanx::ast::optoken::op_shift_right_assign)
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
                -> phylanx::ast::expr_node_type const& { return pe; },
            "access the current primary_expr's value")
        .def(pybind11::self == pybind11::self)
        .def(pybind11::self != pybind11::self)
        .def("__str__",
            &phylanx::bindings::as_string<phylanx::ast::primary_expr>)
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
                -> phylanx::ast::operand_node_type const& { return op; },
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
            [](phylanx::ast::expression& e, phylanx::ast::operation const& op) {
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
        .def(pybind11::pickle(
            &phylanx::bindings::pickle_helper<phylanx::ast::expression>,
            &phylanx::bindings::unpickle_helper<phylanx::ast::expression>));

    // list of phylanx::ast::operations
    pybind11::class_<std::vector<phylanx::ast::operation>>(
        ast, "operation_list", "A list of operations")
        .def(pybind11::init<>())
        .def("pop_back", &std::vector<phylanx::ast::operation>::pop_back)
        .def("append",
            [](std::vector<phylanx::ast::operation>& v,
                phylanx::ast::operation const& f) { v.push_back(f); })
        .def("__len__",
            [](const std::vector<phylanx::ast::operation>& v) {
                return v.size();
            })
        .def("__iter__",
            [](std::vector<phylanx::ast::operation>& v) {
                return pybind11::make_iterator(v.begin(), v.end());
            },
            pybind11::keep_alive<0, 1>());

    // list of phylanx::ast::expressions
    pybind11::class_<std::vector<phylanx::ast::expression>>(
        ast, "expression_list", "A list of expressions")
        .def(pybind11::init<>())
        .def("pop_back", &std::vector<phylanx::ast::expression>::pop_back)
        .def("append",
            [](std::vector<phylanx::ast::expression>& v,
                phylanx::ast::expression const& f) { v.push_back(f); })
        .def("__len__",
            [](const std::vector<phylanx::ast::expression>& v) {
                return v.size();
            })
        .def("__iter__",
            [](std::vector<phylanx::ast::expression>& v) {
                return pybind11::make_iterator(v.begin(), v.end());
            },
            pybind11::keep_alive<0, 1>());

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
                phylanx::ast::expression const& expr) {
                e.args.push_back(expr);
            },
            "append a operand to this expression")
        .def(pybind11::self == pybind11::self)
        .def(pybind11::self != pybind11::self)
        .def("__str__",
            &phylanx::bindings::as_string<phylanx::ast::function_call>)
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
    ast.def("traverse",
        &phylanx::bindings::traverse<phylanx::ast::primary_expr>,
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
    ast.def("traverse",
        &phylanx::bindings::traverse<phylanx::ast::function_call>,
        "traverse the given AST expression and call the provided function "
        "on each part of it");

    ///////////////////////////////////////////////////////////////////////////
    // expose expression tree
    auto execution_tree = m.def_submodule("execution_tree");

    execution_tree.def("generate_tree", &phylanx::bindings::generate_tree,
        "generate expression tree from given expression");

    execution_tree.def("var",
        [](double d) {
            return hpx::threads::run_as_hpx_thread([&]() {
                using namespace phylanx::execution_tree;
                return primitive{hpx::local_new<primitives::variable>(
                    phylanx::ir::node_data<double>{d})};
            });
        },
        "create a new variable from a floating point value");

    execution_tree.def("linspace",
        [](double xmin, double xmax, std::size_t nx) {
            std::vector<double> v;
            if (nx < 1)
            {
                ;
            }
            else if (nx == 1)
            {
                v.push_back(xmin);
            }
            else
            {
                double dx = (xmax - xmin) / (nx - 1);
                for (std::size_t i = 0; i < nx; i++)
                {
                    double x = xmin + dx * i;
                    v.push_back(x);
                }
            }
            return hpx::threads::run_as_hpx_thread([&]() {
                using namespace phylanx::execution_tree;
                return primitive{hpx::local_new<primitives::variable>(
                    phylanx::ir::node_data<double>{v})};
            });
        },
        "create a new variable from a vector of floating point values");

    execution_tree.def("linearmatrix",
        [](std::size_t nx, std::size_t ny, double m0, double dx, double dy) {
            blaze::DynamicMatrix<double> m{nx, ny};
            for (int i = 0; i < nx; i++)
            {
                for (int j = 0; j < ny; j++)
                {
                    m(i, j) = m0 + dx * i + dy * j;
                }
            }
            return hpx::threads::run_as_hpx_thread([&]() {
                using namespace phylanx::execution_tree;
                return primitive{hpx::local_new<primitives::variable>(
                    phylanx::ir::node_data<double>{m})};
            });
        },
        "create a new variable from a new matrix from linear coefficients m0, "
        "dx, and dy");

    execution_tree.def("file_read_csv",
        [](std::string fname) {
            return hpx::threads::run_as_hpx_thread([&]() {
                using namespace phylanx::execution_tree;
                auto frc =
                    phylanx::execution_tree::primitives::file_read_csv{{fname}};
                std::vector<primitive_argument_type> args;
                primitive_argument_type result = frc.eval(args).get();
                return primitive{
                    hpx::local_new<primitives::variable>(result.variant())};
            });
        },
        "Read in the contents of a csv file");

    execution_tree.def("slice",
        [](phylanx::execution_tree::primitive const& obj, std::int64_t x0,
            std::int64_t xN, std::int64_t y0, std::int64_t yN) {
            return hpx::threads::run_as_hpx_thread([&]() {
                using namespace phylanx::execution_tree;
                auto frc =
                    phylanx::execution_tree::primitives::slicing_operation{
                        {obj, x0, xN, y0, yN}};
                std::vector<primitive_argument_type> args;
                primitive_argument_type result = frc.eval(args).get();
                return primitive{
                    hpx::local_new<primitives::variable>(result.variant())};
            });
        },
        "Take a slice of a 2d array");

    execution_tree.def("zeros",
        [](std::size_t nx) {
            blaze::DynamicVector<double> v;
            v.resize(nx);
            return hpx::threads::run_as_hpx_thread([&]() {
                using namespace phylanx::execution_tree;
                return primitive{hpx::local_new<primitives::variable>(
                    phylanx::ir::node_data<double>{v})};
            });
        },
        "create a new variable from a vector of zeros");

    execution_tree.def("zeros",
        [](std::size_t nx, std::size_t ny) {
            blaze::DynamicMatrix<double> m{nx, ny};
            m = 0.0;
            return hpx::threads::run_as_hpx_thread([&]() {
                using namespace phylanx::execution_tree;
                return primitive{hpx::local_new<primitives::variable>(
                    phylanx::ir::node_data<double>{m})};
            });
        },
        "create a new variable from a matrix of zeros");

    execution_tree.def(
        "eval", expression_compiler<>, expression_compiler_help);
    execution_tree.def(
        "eval", expression_compiler<double>, expression_compiler_help);
    execution_tree.def("eval",
        expression_compiler<phylanx::execution_tree::primitive const&>,
        expression_compiler_help);
    execution_tree.def("eval",
        expression_compiler<phylanx::execution_tree::primitive const&,
            phylanx::execution_tree::primitive const&>,
        expression_compiler_help);
    execution_tree.def("eval",
        expression_compiler<phylanx::execution_tree::primitive const&, double>,
        expression_compiler_help);
    execution_tree.def("eval",
        expression_compiler<double, phylanx::execution_tree::primitive const&>,
        expression_compiler_help);
    execution_tree.def("eval",
        expression_compiler<double, double>,
        expression_compiler_help);
    execution_tree.def("eval",
        expression_compiler<phylanx::execution_tree::primitive const&,
            phylanx::execution_tree::primitive const&,
            phylanx::execution_tree::primitive const&>,
        expression_compiler_help);
    execution_tree.def("eval",
        expression_compiler<double, phylanx::execution_tree::primitive const&,
            phylanx::execution_tree::primitive const&>,
        expression_compiler_help);
    execution_tree.def("eval",
        expression_compiler<phylanx::execution_tree::primitive const&, double,
            phylanx::execution_tree::primitive const&>,
        expression_compiler_help);
    execution_tree.def("eval",
        expression_compiler<phylanx::execution_tree::primitive const&,
            phylanx::execution_tree::primitive const&, double>,
        expression_compiler_help);
    execution_tree.def("eval",
        expression_compiler<double, double,
            phylanx::execution_tree::primitive const&>,
        expression_compiler_help);
    execution_tree.def("eval",
        expression_compiler<double, phylanx::execution_tree::primitive const&,
            double>,
        expression_compiler_help);
    execution_tree.def("eval",
        expression_compiler<phylanx::execution_tree::primitive const&, double,
            double>,
        expression_compiler_help);
    execution_tree.def("eval",
        expression_compiler<double, double, double>,
        expression_compiler_help);

    execution_tree.def("add",
        [](phylanx::execution_tree::primitive const& obj1,
            phylanx::execution_tree::primitive const& obj2) {
            return hpx::threads::run_as_hpx_thread([&]() {
                using namespace phylanx::execution_tree;
                auto addition = primitives::add_operation{{obj1, obj2}};
                std::vector<primitive_argument_type> args;
                primitive_argument_type result = addition.eval(args).get();
                return primitive{
                    hpx::local_new<primitives::variable>(result.variant())};
            });

        },
        "Addition of any two phylanx objects.");

    execution_tree.def("and",
        [](phylanx::execution_tree::primitive const& obj1,
            phylanx::execution_tree::primitive const& obj2) {
            return hpx::threads::run_as_hpx_thread([&]() {
                using namespace phylanx::execution_tree;
                auto and_op = primitives::and_operation{{obj1, obj2}};
                std::vector<primitive_argument_type> args;
                primitive_argument_type result = and_op.eval(args).get();
                return primitive{
                    hpx::local_new<primitives::variable>(result.variant())};
            });
        },
        "Logical and operation.");

    execution_tree.def("cross",
        [](phylanx::execution_tree::primitive const& obj1,
            phylanx::execution_tree::primitive const& obj2) {
            return hpx::threads::run_as_hpx_thread([&]() {
                using namespace phylanx::execution_tree;
                auto cross = primitives::cross_operation{{obj1, obj2}};
                std::vector<primitive_argument_type> args;
                primitive_argument_type result = cross.eval(args).get();
                return primitive{
                    hpx::local_new<primitives::variable>(result.variant())};
            });
        },
        "Cross product of two vectors.");

    execution_tree.def("det",
        [](phylanx::execution_tree::primitive const& obj) {
            return hpx::threads::run_as_hpx_thread([&]() {
                using namespace phylanx::execution_tree;
                auto det = primitives::determinant{{obj}};
                std::vector<primitive_argument_type> args;
                primitive_argument_type result = det.eval(args).get();
                return primitive{
                    hpx::local_new<primitives::variable>(result.variant())};
            });
        },
        "Compute the determinant of an array.");

    execution_tree.def("div",
        [](phylanx::execution_tree::primitive const& obj1,
            phylanx::execution_tree::primitive const& obj2) {
            return hpx::threads::run_as_hpx_thread([&]() {
                using namespace phylanx::execution_tree;
                auto division = primitives::div_operation{{obj1, obj2}};
                std::vector<primitive_argument_type> args;
                primitive_argument_type result = division.eval(args).get();
                return primitive{
                    hpx::local_new<primitives::variable>(result.variant())};
            });
        },
        "Compute division.");

    execution_tree.def("dot",
        [](phylanx::execution_tree::primitive const& obj1,
            phylanx::execution_tree::primitive const& obj2) {
            return hpx::threads::run_as_hpx_thread([&]() {
                using namespace phylanx::execution_tree;
                auto dot_product = primitives::dot_operation{{obj1, obj2}};
                std::vector<primitive_argument_type> args;
                primitive_argument_type result = dot_product.eval(args).get();
                return primitive{
                    hpx::local_new<primitives::variable>(result.variant())};
            });
        },
        "Dot product.");

    execution_tree.def("exp",
        [](phylanx::execution_tree::primitive const& obj) {
            return hpx::threads::run_as_hpx_thread([&]() {
                using namespace phylanx::execution_tree;
                auto exponential = primitives::exponential_operation{{obj}};
                std::vector<primitive_argument_type> args;
                primitive_argument_type result = exponential.eval(args).get();
                return primitive{
                    hpx::local_new<primitives::variable>(result.variant())};
            });
        },
        "Calculate the exponentials of all elements of the array.");

    execution_tree.def("inv",
        [](phylanx::execution_tree::primitive const& obj) {
            return hpx::threads::run_as_hpx_thread([&]() {
                using namespace phylanx::execution_tree;
                auto inverse = primitives::inverse_operation{{obj}};
                std::vector<primitive_argument_type> args;
                primitive_argument_type result = inverse.eval(args).get();
                return primitive{
                    hpx::local_new<primitives::variable>(result.variant())};
            });
        },
        "Compute the inverse of an array.");

    execution_tree.def("multiply",
        [](phylanx::execution_tree::primitive const& obj1,
            phylanx::execution_tree::primitive const& obj2) {
            return hpx::threads::run_as_hpx_thread([&]() {
                using namespace phylanx::execution_tree;
                auto multiplication = primitives::mul_operation{{obj1, obj2}};
                std::vector<primitive_argument_type> args;
                primitive_argument_type result =
                    multiplication.eval(args).get();
                return primitive{
                    hpx::local_new<primitives::variable>(result.variant())};
            });
        },
        "Multiplication of any two phylanx objects.");

    execution_tree.def("power",
        [](phylanx::execution_tree::primitive const& obj1,
            phylanx::execution_tree::primitive const& obj2) {
            return hpx::threads::run_as_hpx_thread([&]() {
                using namespace phylanx::execution_tree;
                auto power = primitives::power_operation{{obj1, obj2}};
                std::vector<primitive_argument_type> args;
                primitive_argument_type result = power.eval(args).get();
                return primitive{
                    hpx::local_new<primitives::variable>(result.variant())};
            });
        },
        "Raise elements of an array to the power from the second argument.");

    execution_tree.def("subtract",
        [](phylanx::execution_tree::primitive const& obj1,
            phylanx::execution_tree::primitive const& obj2) {
            return hpx::threads::run_as_hpx_thread([&]() {
                using namespace phylanx::execution_tree;
                auto subtraction = primitives::sub_operation{{obj1, obj2}};
                std::vector<primitive_argument_type> args;
                primitive_argument_type result = subtraction.eval(args).get();
                return primitive{
                    hpx::local_new<primitives::variable>(result.variant())};
            });
        },
        "Element-wise subtraction.");

    execution_tree.def("transpose",
        [](phylanx::execution_tree::primitive const& obj) {
            return hpx::threads::run_as_hpx_thread([&]() {
                using namespace phylanx::execution_tree;
                auto transpose = primitives::transpose_operation{{obj}};
                std::vector<primitive_argument_type> args;
                primitive_argument_type result = transpose.eval(args).get();
                return primitive{
                    hpx::local_new<primitives::variable>(result.variant())};
            });
        },
        "Transpose of a matrix.");

    pybind11::class_<phylanx::execution_tree::primitive>(execution_tree,
        "primitive", "type representing an arbitrary execution tree")
        .def("eval",
            [](phylanx::execution_tree::primitive const& p) {
                return hpx::threads::run_as_hpx_thread([&]() {
                    using namespace phylanx::execution_tree;
                    return numeric_operand(p, {}).get()[0];
                });
            },
            "evaluate execution tree")
        .def("num_dimensions",
            [](phylanx::execution_tree::primitive const& p) {
                return hpx::threads::run_as_hpx_thread([&]() {
                    using namespace phylanx::execution_tree;
                    auto n = numeric_operand(p, {}).get();
                    return n.num_dimensions();
                });
            },
            "get the number of dimensions")
        .def("get",
            [](phylanx::execution_tree::primitive const& p, int index) {
                return hpx::threads::run_as_hpx_thread([&]() {
                    using namespace phylanx::execution_tree;
                    auto n = numeric_operand(p, {}).get();
                    return n[index];
                });
            },
            "Get the value at the specified index")
        .def("get",
            [](phylanx::execution_tree::primitive const& p, int index1,
                int index2) {
                return hpx::threads::run_as_hpx_thread([&]() {
                    using namespace phylanx::execution_tree;
                    auto n = numeric_operand(p, {}).get();
                    phylanx::ir::node_data<double>::dimensions_type indicies{
                        std::size_t(index1), std::size_t(index2)};
                    return n[indicies];
                });
            },
            "Get the value specified by the x,y index pair")
        .def("dimension",
            [](phylanx::execution_tree::primitive const& p, int index) {
                return hpx::threads::run_as_hpx_thread([&]() {
                    using namespace phylanx::execution_tree;
                    auto n = numeric_operand(p, {}).get();
                    return n.dimension(index);
                });
            },
            "Get the size of the given dimension")
        .def("assign",
            [](phylanx::execution_tree::primitive p, double d) {
                hpx::threads::run_as_hpx_thread([&]() {
                    p.store(
                        hpx::launch::sync, phylanx::ir::node_data<double>{d});
                });
            },
            "assign another value to variable");

    ///////////////////////////////////////////////////////////////////////////
    // expose util submodule
    auto util = m.def_submodule("util");

    util.def("serialize", &phylanx::bindings::serialize<phylanx::ast::optoken>,
        "serialize an AST optoken object into a byte-stream");
    util.def("serialize",
        &phylanx::bindings::serialize<phylanx::ast::identifier>,
        "serialize an AST identifier object into a byte-stream");
    util.def("serialize",
        &phylanx::bindings::serialize<phylanx::ast::primary_expr>,
        "serialize an AST primary_expr object into a byte-stream");
    util.def("serialize", &phylanx::bindings::serialize<phylanx::ast::operand>,
        "serialize an AST operand object into a byte-stream");
    util.def("serialize",
        &phylanx::bindings::serialize<phylanx::ast::unary_expr>,
        "serialize an AST unary_expr object into a byte-stream");
    util.def("serialize",
        &phylanx::bindings::serialize<phylanx::ast::operation>,
        "serialize an AST operation object into a byte-stream");
    util.def("serialize",
        &phylanx::bindings::serialize<phylanx::ast::expression>,
        "serialize an AST expression object into a byte-stream");
    util.def("serialize",
        &phylanx::bindings::serialize<phylanx::ast::function_call>,
        "serialize an AST expression object into a byte-stream");

    util.def("unserialize", &phylanx::util::unserialize,
        "un-serialize a byte-stream into an AST expression object");
}
