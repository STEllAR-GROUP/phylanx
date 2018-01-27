//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2017 Adrian Serio
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_IF_CONDITIONAL_OCT_06_2017_1124AM)
#define PHYLANX_PRIMITIVES_IF_CONDITIONAL_OCT_06_2017_1124AM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/include/components.hpp>

#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class HPX_COMPONENT_EXPORT if_conditional
        : public base_primitive
        , public hpx::components::component_base<if_conditional>
    {
    public:
        static match_pattern_type const match_data;

        if_conditional() = default;

        if_conditional(std::vector<primitive_argument_type>&& operand);

        hpx::future<primitive_result_type> eval(
            std::vector<primitive_argument_type> const& args) const override;
    };
}}}

#endif
