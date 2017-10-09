//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DETERMINANT_OCT_09_2017_0158PM)
#define PHYLANX_PRIMITIVES_DETERMINANT_OCT_09_2017_0158PM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>

#include <utility>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class HPX_COMPONENT_EXPORT determinant
        : public base_primitive
        , public hpx::components::component_base<determinant>
    {
    private:
        using operand_type = ir::node_data<double>;
        using operands_type = std::vector<operand_type>;

        std::vector<primitive_argument_type> operands_;

    public:
        static match_pattern_type const match_data;

        determinant() = default;

        determinant(
            std::vector<primitive_argument_type>&& operands);

        hpx::future<primitive_result_type> eval() const override;

    protected:
        ir::node_data<double> determinant0d(operands_type && ops) const;
        ir::node_data<double> determinantxd(operands_type && ops) const;
    };
}}}

#endif
