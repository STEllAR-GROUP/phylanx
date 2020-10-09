// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_EXECUTION_TREE_COMPILER_COMPONENT_HPP)
#define PHYLANX_EXECUTION_TREE_COMPILER_COMPONENT_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/compile.hpp>
#include <phylanx/execution_tree/compiler/compiler.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/include/actions.hpp>
#include <hpx/include/components.hpp>
#include <hpx/include/future.hpp>
#include <hpx/include/util.hpp>
#include <hpx/async_base/launch_policy.hpp>

#include <cstddef>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    class compiler_component
      : public hpx::components::component_base<compiler_component>
    {
    public:
        PHYLANX_EXPORT compiler_component();

        // compile_action
        PHYLANX_EXPORT compiler::entry_point compile(std::string const& name,
            std::vector<ast::expression> expr);

        // define_variable action
        PHYLANX_EXPORT compiler::function define_variable(
            std::string const& codename, std::string const& name,
            primitive_argument_type body, bool define_globally);

        HPX_DEFINE_COMPONENT_ACTION(
            compiler_component, compile, compile_action);
        HPX_DEFINE_COMPONENT_ACTION(
            compiler_component, define_variable, define_variable_action);

    private:
        static std::string generate_unique_function_name();

        compiler::function_list snippets_;
        compiler::environment env_;
        compiler::expression_pattern_list const patterns_;
    };

    ///////////////////////////////////////////////////////////////////////////
    class physl_compiler
      : public hpx::components::client_base<physl_compiler, compiler_component>
    {
    public:
        using base_type =
            hpx::components::client_base<physl_compiler, compiler_component>;

        PHYLANX_EXPORT physl_compiler(hpx::id_type& id);
        PHYLANX_EXPORT physl_compiler(hpx::id_type&& id);
        PHYLANX_EXPORT physl_compiler(hpx::future<hpx::id_type>&& id);

        PHYLANX_EXPORT physl_compiler(
            hpx::future<hpx::id_type>&& id, std::string const& name);

        PHYLANX_EXPORT hpx::future<compiler::entry_point> compile(
            std::string const& name, std::vector<ast::expression> exprs);
        PHYLANX_EXPORT compiler::entry_point compile(hpx::launch::sync_policy,
            std::string const& name, std::vector<ast::expression> exprs);

        PHYLANX_EXPORT hpx::future<compiler::entry_point> compile(
            std::string const& name, std::string const& expr);
        PHYLANX_EXPORT compiler::entry_point compile(hpx::launch::sync_policy,
            std::string const& name, std::string const& expr);

        PHYLANX_EXPORT hpx::future<compiler::entry_point> compile(
            std::vector<ast::expression> exprs);
        PHYLANX_EXPORT compiler::entry_point compile(hpx::launch::sync_policy,
            std::vector<ast::expression> exprs);

        PHYLANX_EXPORT hpx::future<compiler::entry_point> compile(
            std::string const& expr);
        PHYLANX_EXPORT compiler::entry_point compile(hpx::launch::sync_policy,
            std::string const& expr);

        PHYLANX_EXPORT hpx::future<compiler::function> define_variable(
            std::string const& codename, std::string const& name,
            primitive_argument_type body, bool define_globally = false);
        PHYLANX_EXPORT compiler::function define_variable(
            hpx::launch::sync_policy, std::string const& codename,
            std::string const& name, primitive_argument_type body,
            bool define_globally = false);
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Create a new compiler on the specified locality and register it with AGAS.
    PHYLANX_EXPORT physl_compiler create_and_register_compiler(
        hpx::id_type const& locality = hpx::find_here());

    PHYLANX_EXPORT physl_compiler create_compiler(
        hpx::id_type const& locality = hpx::find_here());

    /// Access an existing (registered) compiler on the given locality.
    PHYLANX_EXPORT physl_compiler access_compiler(
        hpx::id_type const& locality = hpx::find_here());
}}

// Declaration of serialization support for the actions
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::compiler_component::compile_action,
    phylanx_compiler_compile_action);
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::compiler_component::define_variable_action,
    phylanx_compiler_define_variable_action);

#endif

