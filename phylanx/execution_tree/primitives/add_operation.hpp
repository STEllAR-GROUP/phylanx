//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ADD_OPERATION_SEP_05_2017_1202PM)
#define PHYLANX_PRIMITIVES_ADD_OPERATION_SEP_05_2017_1202PM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/optional.hpp>
#include <phylanx/util/serialization/optional.hpp>

#include <hpx/include/components.hpp>

#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class HPX_COMPONENT_EXPORT add_operation
        : public base_primitive
        , public hpx::components::component_base<add_operation>
    {
    private:
        using operand_type = ir::node_data<double>;
        using operands_type = std::vector<operand_type>;

    public:
        static std::vector<match_pattern_type> const match_data;

        add_operation() = default;

        add_operation(std::vector<primitive_argument_type>&& operands);

        hpx::future<primitive_result_type> eval() const override;

    protected:
        ir::node_data<double> add0d(operands_type && ops) const;
        ir::node_data<double> add0d0d(operands_type && ops) const;
        ir::node_data<double> add0d1d(operands_type && ops) const;
        ir::node_data<double> add0d2d(operands_type && ops) const;

        ir::node_data<double> add1d(operands_type && ops) const;
        ir::node_data<double> add1d0d(operands_type && ops) const;
        ir::node_data<double> add1d1d(operands_type && ops) const;

        ir::node_data<double> add2d(operands_type && ops) const;
        ir::node_data<double> add2d0d(operands_type && ops) const;
        ir::node_data<double> add2d2d(operands_type && ops) const;

    private:
        std::vector<primitive_argument_type> operands_;
    };
}}}

#endif


