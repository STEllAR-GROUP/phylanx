//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_EQUAL_OCT_07_2017_0212PM)
#define PHYLANX_PRIMITIVES_EQUAL_OCT_07_2017_0212PM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/include/components.hpp>

#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class HPX_COMPONENT_EXPORT equal
        : public base_primitive
        , public hpx::components::component_base<equal>
    {
    private:
        using operand_type = ir::node_data<double>;
        using operands_type = std::vector<primitive_result_type>;

    public:
        static match_pattern_type const match_data;

        equal() = default;

        equal(std::vector<primitive_argument_type>&& operands);

        hpx::future<primitive_result_type> eval() const override;

    protected:
        bool equal0d(operand_type&& lhs, operand_type&& rhs) const;
        bool equal1d(operand_type&& lhs, operand_type&& rhs) const;
        bool equal2d(operand_type&& lhs, operand_type&& rhs) const;

        bool equal1d1d(operand_type&& lhs, operand_type&& rhs) const;
        bool equal2d2d(operand_type&& lhs, operand_type&& rhs) const;

    public:
        bool equal_all(operand_type&& lhs, operand_type&& rhs) const;

    private:
        std::vector<primitive_argument_type> operands_;
    };
}}}

#endif


