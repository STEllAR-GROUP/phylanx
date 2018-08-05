//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/compiler/compiler.hpp>
#include <phylanx/execution_tree/compiler/compiler_component.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component.hpp>

#include <mutex>
#include <string>

///////////////////////////////////////////////////////////////////////////////
// Serialization support for the base_file actions
typedef phylanx::execution_tree::compiler::compiler_component
    compiler_component_type;

HPX_REGISTER_ACTION(compiler_component_type::compile_action,
    phylanx_compiler_compile_action)

typedef hpx::components::component<compiler_component_type>
    phylanx_compiler_component_type;
HPX_REGISTER_COMPONENT(phylanx_compiler_component_type)

//////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace compiler
{
    hpx::future<entry_point> compiler_component::compile(
        std::string const& name, ast::expression const& expr) const
    {
        std::lock_guard<mutex_type> l(mtx_);
        return hpx::make_ready_future(entry_point{});
    }
}}}
