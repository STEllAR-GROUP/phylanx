//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_UNARY_NOT_OPERATION_OCT_10_2017_0310PM)
#define PHYLANX_PRIMITIVES_UNARY_NOT_OPERATION_OCT_10_2017_0310PM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>

#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class HPX_COMPONENT_EXPORT unary_not_operation
      : public base_primitive
      , public hpx::components::component_base<unary_not_operation>
    {
    public:
        static std::vector<match_pattern_type> const match_data;

        unary_not_operation() = default;

        unary_not_operation(std::vector<primitive_argument_type>&& operands);

        hpx::future<primitive_result_type> eval(
            std::vector<primitive_argument_type> const& params) const override;

    private:
        std::vector<primitive_argument_type> operands_;
    };
}}}

#endif


