//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_WHILE_OPERATION_OCT_06_2017_1127AM)
#define PHYLANX_PRIMITIVES_WHILE_OPERATION_OCT_06_2017_1127AM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/include/components.hpp>

#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class HPX_COMPONENT_EXPORT while_operation
      : public base_primitive
      , public hpx::components::component_base<while_operation>
    {
        using operands_type = std::vector<primitive_result_type>;

    public:
        static std::vector<match_pattern_type> const match_data;

        while_operation() = default;

        while_operation(std::vector<primitive_argument_type>&& operands);

        hpx::future<primitive_result_type> eval() const override;

    private:
        std::vector<primitive_argument_type> operands_;
    };
}}}

#endif


