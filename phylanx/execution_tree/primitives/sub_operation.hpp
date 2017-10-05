//  Copyright (c) 2017  Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_SUB_OPERATION_SEP_15_2017_1035AM)
#define PHYLANX_PRIMITIVES_SUB_OPERATION_SEP_15_2017_1035AM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/util/optional.hpp>
#include <phylanx/util/serialization/optional.hpp>

#include <hpx/include/components.hpp>

#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class HPX_COMPONENT_EXPORT sub_operation
        : public base_primitive
        , public hpx::components::component_base<sub_operation>
    {
    private:
        using operand_type = util::optional<ir::node_data<double>>;
        using operands_type = std::vector<operand_type>;

    public:
        sub_operation() = default;

        sub_operation(std::vector<primitive_argument_type>&& operands);

        hpx::future<operand_type> eval() const override;

    protected:
        ir::node_data<double> sub0d(operands_type const& ops) const;
        ir::node_data<double> sub1d(operands_type const& ops) const;
        ir::node_data<double> sub2d(operands_type const& ops) const;

        ir::node_data<double> sub1d1d(operands_type const& ops) const;
        ir::node_data<double> sub2d2d(operands_type const& ops) const;

    private:
        std::vector<primitive_argument_type> operands_;
    };
}}}

#endif


