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

#include <cstdint>
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
    // support for compilers
    struct compiler_state
    {
        // keep module alive until this has been free'd
        pybind11::weakref m;

        phylanx::execution_tree::compiler::environment eval_env;
        phylanx::execution_tree::compiler::function_list eval_snippets;

        static pybind11::object import_phylanx()
        {
#if defined(_DEBUG)
            return pybind11::module::import("phylanx._phylanxd");
#else
            return pybind11::module::import("phylanx._phylanx");
#endif
        }

        compiler_state()
          : m(import_phylanx())
          , eval_env(phylanx::execution_tree::compiler::default_environment())
          , eval_snippets()
        {
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // support for the traverse API
    struct traverse_helper
    {
        template <typename Ast>
        bool on_enter(Ast const& ast) const
        {
            pybind11::gil_scoped_acquire acquire;       // acquire GIL

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
            pybind11::gil_scoped_acquire acquire;       // acquire GIL

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
        pybind11::gil_scoped_release release;       // release GIL
        return phylanx::ast::traverse(ast, traverse_helper{func, args, kwargs});
    }

    // serialization support
    template <typename Ast>
    std::vector<char> serialize(Ast const& ast)
    {
        pybind11::gil_scoped_release release;       // release GIL
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
        pybind11::gil_scoped_release release;       // release GIL
        return phylanx::util::serialize(ast);
    }

    template <typename Ast>
    Ast unpickle_helper(std::vector<char> data)
    {
        pybind11::gil_scoped_release release;       // release GIL

        Ast ast;
        phylanx::util::detail::unserialize(data, ast);
        return ast;
    }

    ///////////////////////////////////////////////////////////////////////////
    inline void expression_compiler(std::string const& file_name, std::string xexpr_str,
        compiler_state& c)
    {
        pybind11::gil_scoped_release release;       // release GIL

        return hpx::threads::run_as_hpx_thread(
            [&]() -> void
            {
                phylanx::execution_tree::compile(
                    file_name, phylanx::ast::generate_ast(xexpr_str), c.eval_snippets,
                    c.eval_env);
            });
    };

    ///////////////////////////////////////////////////////////////////////////
    inline phylanx::execution_tree::primitive_argument_type
    expression_evaluator(
        std::string xexpr_str, compiler_state& c, pybind11::args args)
    {
        pybind11::gil_scoped_release release;       // release GIL

        return hpx::threads::run_as_hpx_thread(
            [&]() -> phylanx::execution_tree::primitive_argument_type
            {
                pybind11::gil_scoped_acquire acquire;
                auto xexpr = phylanx::ast::generate_ast(xexpr_str);
                auto const& code_x = phylanx::execution_tree::compile(
                    xexpr, c.eval_snippets, c.eval_env);
                auto x = code_x.run();

                phylanx::execution_tree::primitive_arguments_type keep_alive;
                keep_alive.reserve(args.size());
                phylanx::execution_tree::primitive_arguments_type fargs;
                fargs.reserve(args.size());

                for (auto const& item : args)
                {
                    phylanx::execution_tree::primitive_argument_type value =
                        item.cast<
                            phylanx::execution_tree::primitive_argument_type>();
                    keep_alive.emplace_back(std::move(value));
                    fargs.emplace_back(extract_ref_value(keep_alive.back()));
                }

                pybind11::gil_scoped_release release;       // release GIL
                return x(std::move(fargs));
            });
    };
}}

#endif
