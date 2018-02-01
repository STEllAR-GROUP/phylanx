//  Copyright (c) 2018 Hartmut Kaiser
//  Copyright (c) 2018 Shahrzad Shirzad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_EQUAL_ELEMENTWISE_JAN_31_2018_0212PM)
#define PHYLANX_PRIMITIVES_EQUAL_ELEMENTWISE_JAN_31_2018_0212PM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>

#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class HPX_COMPONENT_EXPORT equal_elementwise
      : public base_primitive
      , public hpx::components::component_base<equal_elementwise>
    {
    public:
        static match_pattern_type const match_data;

        equal_elementwise() = default;

        equal_elementwise(std::vector<primitive_argument_type>&& operands);

        hpx::future<primitive_result_type> eval(
            std::vector<primitive_argument_type> const& args) const override;
    };
}}}

#endif
