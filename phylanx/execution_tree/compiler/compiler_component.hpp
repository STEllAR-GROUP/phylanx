// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_EXECUTION_TREE_COMPILER_COMPONENT_HPP)
#define PHYLANX_EXECUTION_TREE_COMPILER_COMPONENT_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/include/actions.hpp>
#include <hpx/include/components.hpp>
#include <hpx/include/future.hpp>
#include <hpx/include/util.hpp>

namespace phylanx { namespace execution_tree { namespace compiler
{
    ///////////////////////////////////////////////////////////////////////////
    class compiler_component
      : public hpx::components::component_base<compiler_component>
    {
    public:
        compiler_component() = default;

        // compile_action
        PHYLANX_EXPORT hpx::future<primitive_argument_type> compile() const;

        HPX_DEFINE_COMPONENT_ACTION(
            compiler_component, compile, compile_action);

    private:
        using mutex_type = hpx::lcos::local::spinlock;
        mutable mutex_type mtx_;
    };
}}}

// Declaration of serialization support for the local_file actions
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::compiler::compiler_component::compile_action,
    phylanx_compiler_compile_action);

#endif

