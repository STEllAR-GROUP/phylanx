//  Copyright (c) 2018-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/generate_ast.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/compile.hpp>
#include <phylanx/execution_tree/compiler/compiler.hpp>
#include <phylanx/execution_tree/compiler_component.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component.hpp>

#include <hpx/async.hpp>
#include <hpx/modules/format.hpp>
#include <hpx/include/agas.hpp>
#include <hpx/include/components.hpp>

#include <atomic>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
// Serialization support for the base_file actions
typedef phylanx::execution_tree::compiler_component
    compiler_component_type;

HPX_REGISTER_ACTION(compiler_component_type::compile_action,
    phylanx_compiler_compile_action)
HPX_REGISTER_ACTION(compiler_component_type::define_variable_action,
    phylanx_compiler_define_variable_action)

typedef hpx::components::component<compiler_component_type>
    phylanx_compiler_component_type;
HPX_REGISTER_COMPONENT(phylanx_compiler_component_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    compiler_component::compiler_component()
      : env_(compiler::default_environment())
      , patterns_(compiler::generate_patterns())
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    compiler::entry_point compiler_component::compile(
        std::string const& name, std::vector<ast::expression> exprs)
    {
        return execution_tree::compile(name, generate_unique_function_name(),
            exprs, snippets_, env_, patterns_);
    }

    compiler::function compiler_component::define_variable(
        std::string const& codename, std::string const& name,
        primitive_argument_type body)
    {
        return execution_tree::define_variable(codename,
            compiler::primitive_name_parts{name, -1, 0, 0}, snippets_, env_,
            std::move(body));
    }

    std::string compiler_component::generate_unique_function_name()
    {
        static std::atomic<std::size_t> function_counter(0);
        return hpx::util::format("function_{}", ++function_counter);
    }

    ///////////////////////////////////////////////////////////////////////////
    physl_compiler::physl_compiler(hpx::id_type& id)
      : base_type(id)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    physl_compiler::physl_compiler(hpx::id_type&& id)
      : base_type(std::move(id))
    {
    }

    physl_compiler::physl_compiler(hpx::future<hpx::id_type>&& fid)
      : base_type(std::move(fid))
    {
    }

    physl_compiler::physl_compiler(
            hpx::future<hpx::id_type>&& fid, std::string const& name)
      : base_type(std::move(fid))
    {
        if (!name.empty())
        {
            this->base_type::register_as(name).get();
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<compiler::entry_point> physl_compiler::compile(
        std::vector<ast::expression> exprs)
    {
        using action_type = typename compiler_component::compile_action;
        return hpx::async(action_type(), this->base_type::get_id(), "<unknown>",
            std::move(exprs));
    }

    compiler::entry_point physl_compiler::compile(hpx::launch::sync_policy,
        std::vector<ast::expression> exprs)
    {
        return compile(std::move(exprs)).get();
    }

    hpx::future<compiler::entry_point> physl_compiler::compile(
        std::string const& name, std::vector<ast::expression> exprs)
    {
        using action_type = typename compiler_component::compile_action;
        return hpx::async(
            action_type(), this->base_type::get_id(), name, std::move(exprs));
    }

    compiler::entry_point physl_compiler::compile(hpx::launch::sync_policy,
        std::string const& name, std::vector<ast::expression> exprs)
    {
        return compile(name, std::move(exprs)).get();
    }

    hpx::future<compiler::entry_point> physl_compiler::compile(
        std::string const& name, std::string const& expr)
    {
        using action_type = typename compiler_component::compile_action;
        return hpx::async(action_type(), this->base_type::get_id(), name,
            ast::generate_ast(expr));
    }

    compiler::entry_point physl_compiler::compile(hpx::launch::sync_policy,
        std::string const& name, std::string const& expr)
    {
        return compile(name, expr).get();
    }

    hpx::future<compiler::entry_point> physl_compiler::compile(
        std::string const& expr)
    {
        using action_type = typename compiler_component::compile_action;
        return hpx::async(action_type(), this->base_type::get_id(), "<unknown>",
            ast::generate_ast(expr));
    }

    compiler::entry_point physl_compiler::compile(hpx::launch::sync_policy,
        std::string const& expr)
    {
        return compile(expr).get();
    }

    hpx::future<compiler::function> physl_compiler::define_variable(
        std::string const& codename, std::string const& name,
        primitive_argument_type body)
    {
        using action_type = typename compiler_component::define_variable_action;
        return hpx::async(action_type(), this->base_type::get_id(), codename,
            name, std::move(body));
    }

    compiler::function physl_compiler::define_variable(hpx::launch::sync_policy,
        std::string const& codename, std::string const& name,
        primitive_argument_type body)
    {
        return define_variable(codename, name, std::move(body)).get();
    }

    ///////////////////////////////////////////////////////////////////////////
    std::string compiler_instance_name(hpx::id_type const& locality)
    {
        return hpx::detail::name_from_basename("/phylanx/compiler/",
            hpx::naming::get_locality_id_from_id(locality));
    }

    physl_compiler create_and_register_compiler(hpx::id_type const& locality)
    {
        return physl_compiler{hpx::new_<compiler_component>(locality),
            compiler_instance_name(locality)};
    }

    physl_compiler access_compiler(hpx::id_type const& locality)
    {
        return physl_compiler{hpx::agas::on_symbol_namespace_event(
            compiler_instance_name(locality), false)};
    }

    physl_compiler create_compiler(hpx::id_type const& locality)
    {
        return physl_compiler{hpx::new_<compiler_component>(locality)};
    }
}}
