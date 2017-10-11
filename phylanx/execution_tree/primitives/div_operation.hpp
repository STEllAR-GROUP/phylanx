//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DIV_OPERATION_OCT_07_2017_0631PM)
#define PHYLANX_PRIMITIVES_DIV_OPERATION_OCT_07_2017_0631PM

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
    class HPX_COMPONENT_EXPORT div_operation
        : public base_primitive
        , public hpx::components::component_base<div_operation>
    {
    private:
        using operand_type = ir::node_data<double>;
        using operands_type = std::vector<operand_type>;

    public:
        static std::vector<match_pattern_type> const match_data;

        div_operation() = default;

        div_operation(std::vector<primitive_argument_type>&& operands);

        hpx::future<primitive_result_type> eval() const override;

    protected:
        ir::node_data<double> div0d(operands_type && ops) const;
        ir::node_data<double> div0d0d(operands_type && ops) const;
        ir::node_data<double> div0d1d(operands_type && ops) const;
        ir::node_data<double> div0d2d(operands_type && ops) const;

        ir::node_data<double> div1d(operands_type && ops) const;
        ir::node_data<double> div1d0d(operands_type && ops) const;
        ir::node_data<double> div1d1d(operands_type && ops) const;

        ir::node_data<double> div2d(operands_type && ops) const;
        ir::node_data<double> div2d0d(operands_type && ops) const;
        ir::node_data<double> div2d2d(operands_type && ops) const;

    private:
        std::vector<primitive_argument_type> operands_;
    };
}}}

#endif


