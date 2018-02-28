//  Copyright (c) 2017 Hartmut Kaiser
//  Copyright (c) 2018 R. Tohid
//  Copyright (c) 2018 Steven R. Brandt
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_BINDING_HELPERS_HPP)
#define PHYLANX_BINDING_HELPERS_HPP

#include <phylanx/phylanx.hpp>

#include <pybind11/pybind11.h>

#include <hpx/runtime/threads/run_as_hpx_thread.hpp>

#include <exception>
#include <sstream>
#include <string>
#include <vector>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace bindings
{
    ///////////////////////////////////////////////////////////////////////////
    void bind_ast(pybind11::module m);
    void bind_execution_tree(pybind11::module m);
    void bind_util(pybind11::module m);

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
    inline phylanx::execution_tree::primitive generate_tree(
        std::string const& expr, pybind11::dict const& dict)
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

    ///////////////////////////////////////////////////////////////////////////
    inline phylanx::execution_tree::primitive_argument_type
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
}}

#endif
