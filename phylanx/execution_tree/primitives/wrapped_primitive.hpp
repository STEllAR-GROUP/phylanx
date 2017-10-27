//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_WRAPPED_PRIMITIVE_OCT_22_2017_0111PM)
#define PHYLANX_PRIMITIVES_WRAPPED_PRIMITIVE_OCT_22_2017_0111PM

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/include/components.hpp>

#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class HPX_COMPONENT_EXPORT wrapped_primitive
      : public base_primitive
      , public hpx::components::locking_hook<
          hpx::components::component_base<wrapped_primitive>>
    {
    public:
        wrapped_primitive() = default;

        wrapped_primitive(std::string name);

        wrapped_primitive(primitive_argument_type target);
        wrapped_primitive(primitive_argument_type target, std::string name);

        wrapped_primitive(primitive_argument_type target,
            std::vector<primitive_argument_type>&& operands);
        wrapped_primitive(primitive_argument_type target,
            std::vector<primitive_argument_type>&& operands, std::string name);

        void set_target(primitive_argument_type target);

        hpx::future<primitive_result_type> eval(
            std::vector<primitive_argument_type> const& params) const override;

        HPX_DEFINE_COMPONENT_DIRECT_ACTION(
            wrapped_primitive, set_target, set_target_direct_action);

    private:
        primitive_argument_type target_;
        std::vector<primitive_argument_type> args_;
        std::string name_;
    };
}}}

// Declaration of serialization support for the actions
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::
        wrapped_primitive::set_target_direct_action,
    phylanx_wrapped_primitive_set_target_action);

namespace phylanx { namespace execution_tree
{
    struct wrapped_primitive : public primitive
    {
        wrapped_primitive(primitive_argument_type const& rhs)
          : primitive(primitive_operand(rhs))
        {}
        wrapped_primitive(primitive_argument_type && rhs)
          : primitive(primitive_operand(std::move(rhs)))
        {}

        PHYLANX_EXPORT hpx::future<void> set_target(
            primitive_argument_type && target);
        PHYLANX_EXPORT void set_target(hpx::launch::sync_policy,
            primitive_argument_type && target);
    };
}}

#endif

