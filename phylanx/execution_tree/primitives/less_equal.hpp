//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_LESS_EQUAL_OCT_07_2017_0226PM)
#define PHYLANX_PRIMITIVES_LESS_EQUAL_OCT_07_2017_0226PM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/include/components.hpp>

#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class HPX_COMPONENT_EXPORT less_equal
        : public base_primitive
        , public hpx::components::component_base<less_equal>
    {
    private:
        using operand_type = util::optional<ir::node_data<double>>;
        using operands_type = std::vector<operand_type>;

    public:
        less_equal() = default;

        less_equal(std::vector<primitive_argument_type>&& operands);

        hpx::future<operand_type> eval() const override;

    protected:
        ir::node_data<double> less_equal0d(operands_type const& ops) const;
        ir::node_data<double> less_equal1d(operands_type const& ops) const;
        ir::node_data<double> less_equal2d(operands_type const& ops) const;

        ir::node_data<double> less_equal1d1d(operands_type const& ops) const;
        ir::node_data<double> less_equal2d2d(operands_type const& ops) const;

    private:
        std::vector<primitive_argument_type> operands_;
    };
}}}

#endif


