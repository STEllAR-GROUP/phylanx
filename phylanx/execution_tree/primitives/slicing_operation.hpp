// Copyright (c) 2017 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_SLICING_OPERATION_1153_10242017_HPP
#define PHYLANX_SLICING_OPERATION_1153_10242017_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/util/optional.hpp>
#include <phylanx/util/serialization/optional.hpp>

#include <hpx/include/components.hpp>

#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class HPX_COMPONENT_EXPORT slicing_operation
      : public base_primitive
      , public hpx::components::component_base<slicing_operation>
    {
    public:
        static std::vector<match_pattern_type> const match_data;

        slicing_operation() = default;

        slicing_operation(std::vector<primitive_argument_type>&& operands);

        hpx::future<primitive_result_type> eval(
            std::vector<primitive_argument_type> const& params) const override;

    private:
        std::vector<primitive_argument_type> operands_;
    };
}}}

#endif //PHYLANX_SLICING_OPERATION_1153_10242017_HPP
