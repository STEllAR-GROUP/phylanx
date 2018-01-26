// Copyright (c) 2017 Bibek Wagle
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_FOR_OPERATION_HPP
#define PHYLANX_FOR_OPERATION_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/include/components.hpp>

#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class HPX_COMPONENT_EXPORT for_operation
      : public base_primitive
      , public hpx::components::component_base<for_operation>
    {
    public:
        static match_pattern_type const match_data;

        for_operation() = default;

        for_operation(std::vector<primitive_argument_type>&& operands);

        hpx::future<primitive_result_type> eval(
            std::vector<primitive_argument_type> const& args) const override;
    };
}}}

#endif //PHYLANX_FOR_OPERATION_HPP
