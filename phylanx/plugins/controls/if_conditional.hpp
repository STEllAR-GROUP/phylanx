//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2017 Adrian Serio
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_IF_CONDITIONAL_OCT_06_2017_1124AM)
#define PHYLANX_PRIMITIVES_IF_CONDITIONAL_OCT_06_2017_1124AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class if_conditional
      : public primitive_component_base
      , public std::enable_shared_from_this<if_conditional>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        if_conditional() = default;

        if_conditional(primitive_arguments_type&& operand,
            std::string const& name, std::string const& codename);
    };

    inline primitive create_if_conditional(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "if", std::move(operands), name, codename);
    }
}}}

#endif
