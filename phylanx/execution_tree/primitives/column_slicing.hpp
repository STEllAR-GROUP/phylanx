// Copyright (c) 2017 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_COLUMN_SLICING_HPP
#define PHYLANX_COLUMN_SLICING_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>

#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class column_slicing_operation
      : public base_primitive
      , public hpx::components::component_base<column_slicing_operation>
    {
    public:
        static match_pattern_type const match_data;

        column_slicing_operation() = default;

        column_slicing_operation(std::vector<primitive_argument_type>&& operands);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params) const override;
    };
}}}

#endif //PHYLANX_COLUMN_SLICING_HPP
