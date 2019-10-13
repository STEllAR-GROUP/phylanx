//  Copyright (c) 2017-2019 Hartmut Kaiser
//  Copyright (c) 2018 R. Tohid
//  Copyright (c) 2018 Steven R. Brandt
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_BINDING_HELPERS_HPP)
#define PHYLANX_BINDING_HELPERS_HPP

#include <phylanx/phylanx.hpp>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include <hpx/runtime/threads/run_as_hpx_thread.hpp>

#include <cstdint>
#include <exception>
#include <list>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

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
        phylanx::execution_tree::eval_context eval_ctx;

        // name of the main source compiled file
        std::string codename_;

        // data related to measurement status
        bool enable_measurements;
        std::vector<std::string> primitive_instances;

        static pybind11::object import_phylanx()
        {
#if defined(_DEBUG)
            return pybind11::module::import("phylanx._phylanxd");
#else
            return pybind11::module::import("phylanx._phylanx");
#endif
        }

        compiler_state(std::string codename)
          : m(import_phylanx())
          , eval_env(phylanx::execution_tree::compiler::default_environment())
          , eval_snippets()
          , codename_(std::move(codename))
          , enable_measurements(false)
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
        pybind11::gil_scoped_release release;       // release GIL
        return hpx::threads::run_as_hpx_thread(
            [&]() -> std::string
            {
                std::stringstream strm;
                strm << value;
                return strm.str();
            });
    }

    // support for __repr__
    template <typename T>
    std::string repr(T const& value)
    {
        pybind11::gil_scoped_release release;       // release GIL
        return hpx::threads::run_as_hpx_thread(
            [&]() -> std::string
            {
                std::stringstream strm;
                strm << phylanx::util::repr << value << phylanx::util::norepr;
                return strm.str();
            });
    }

    ///////////////////////////////////////////////////////////////////////////
    // pickle support
    template <typename Ast>
    std::vector<char> pickle_helper(Ast const& ast)
    {
        pybind11::gil_scoped_release release;       // release GIL
        return hpx::threads::run_as_hpx_thread(
            [&]() -> std::vector<char>
            {
                return phylanx::util::serialize(ast);
            });
    }

    template <typename Ast>
    Ast unpickle_helper(std::vector<char> const& data)
    {
        pybind11::gil_scoped_release release;       // release GIL
        return hpx::threads::run_as_hpx_thread(
            [&]() -> Ast
            {
                Ast ast;
                phylanx::util::detail::unserialize(data, ast);
                return ast;
            });
    }

    ///////////////////////////////////////////////////////////////////////////
    // compile expression
    std::string expression_compiler(compiler_state& state,
        std::string const& file_name, std::string const& func_name,
        std::string const& xexpr_str);

    // evaluate compiled expression
    phylanx::execution_tree::primitive_argument_type expression_evaluator(
        compiler_state& state, std::string const& file_name,
        std::string const& xexpr_str, pybind11::args args,
        pybind11::kwargs kwargs);

    // extract pre-compiled code for given function name
    phylanx::execution_tree::primitive code_for(
        phylanx::bindings::compiler_state& state,
        std::string const& file_name, std::string const& func_name);

    // extract pre-compiled code for given function name with bound given
    // arguments
    phylanx::execution_tree::primitive bound_code_for(
        phylanx::bindings::compiler_state& state,
        std::string const& file_name, std::string const& func_name,
        pybind11::args args);

    ///////////////////////////////////////////////////////////////////////////
    // initialize measurements for tree evaluations
    std::vector<std::string> enable_measurements(
        compiler_state& c, bool reset_counters);

    // retrieve performance data from all active performance counters
    std::string retrieve_counter_data(compiler_state& c);

    // retrieve tree topology in DOT format for given expression
    std::list<std::string> retrieve_tree_topology(compiler_state& state,
        std::string const& file_name, std::string const& xexpr_str);

    // retrieve tree topology in DOT format for given expression
    std::string retrieve_dot_tree_topology(compiler_state& state,
        std::string const& file_name, std::string const& xexpr_str);

    // retrieve tree topology in Newick format for given expression
    std::string retrieve_newick_tree_topology(compiler_state& state,
        std::string const& file_name, std::string const& xexpr_str);

    // extract the dtype of the given variable/expression
    pybind11::dtype extract_dtype(
        phylanx::execution_tree::primitive_argument_type const& p);
}}

#endif
