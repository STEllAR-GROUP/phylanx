//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_UNARY_MINUS_OPERATION_OCT_10_2017_0248PM)
#define PHYLANX_PRIMITIVES_UNARY_MINUS_OPERATION_OCT_10_2017_0248PM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/optional.hpp>
#include <phylanx/util/serialization/optional.hpp>

#include <hpx/include/components.hpp>

#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class HPX_COMPONENT_EXPORT unary_minus_operation
        : public base_primitive
        , public hpx::components::component_base<unary_minus_operation>
    {
    private:
        using operand_type = ir::node_data<double>;
        using operands_type = std::vector<operand_type>;

    public:
        static std::vector<match_pattern_type> const match_data;

        unary_minus_operation() = default;

        unary_minus_operation(std::vector<primitive_argument_type>&& operands);

        hpx::future<primitive_result_type> eval() const override;

    protected:
        ir::node_data<double> neg0d(operands_type && ops) const;
        ir::node_data<double> negxd(operands_type && ops) const;

    private:
        std::vector<primitive_argument_type> operands_;
    };
}}}

#endif


