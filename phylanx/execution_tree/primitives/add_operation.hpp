//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ADD_OPERATION_SEP_05_2017_1202PM)
#define PHYLANX_PRIMITIVES_ADD_OPERATION_SEP_05_2017_1202PM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/include/components.hpp>

#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class HPX_COMPONENT_EXPORT add_operation
        : public base_primitive
        , public hpx::components::component_base<add_operation>
    {
    private:
        using operands_type = std::vector<ir::node_data<double>>;

    public:
        add_operation() = default;

        add_operation(std::vector<primitive_argument_type>&& operands);

        hpx::future<ir::node_data<double>> eval() const override;

    protected:
        ir::node_data<double> add0d(operands_type const& ops) const;
        ir::node_data<double> add1d(operands_type const& ops) const;
        ir::node_data<double> add2d(operands_type const& ops) const;

        ir::node_data<double> add1d1d(operands_type const& ops) const;
        ir::node_data<double> add2d2d(operands_type const& ops) const;

    private:
        std::vector<primitive_argument_type> operands_;
    };
}}}

#endif


