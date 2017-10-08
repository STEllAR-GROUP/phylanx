//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_GREATER_OCT_07_2017_0223PM)
#define PHYLANX_PRIMITIVES_GREATER_OCT_07_2017_0223PM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/include/components.hpp>

#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class HPX_COMPONENT_EXPORT greater
        : public base_primitive
        , public hpx::components::component_base<greater>
    {
    private:
        using operands_type = std::vector<primitive_result_type>;

    public:
        static match_pattern_type const match_data;

        greater() = default;

        greater(std::vector<primitive_argument_type>&& operands);

        hpx::future<primitive_result_type> eval() const override;

    protected:
        bool greater0d(ir::node_data<double> const& lhs,
            ir::node_data<double> const& rhs) const;
        bool greater1d(ir::node_data<double> const& lhs,
            ir::node_data<double> const& rhs) const;
        bool greater2d(ir::node_data<double> const& lhs,
            ir::node_data<double> const& rhs) const;

        bool greater1d1d(ir::node_data<double> const& lhs,
            ir::node_data<double> const& rhs) const;
        bool greater2d2d(ir::node_data<double> const& lhs,
            ir::node_data<double> const& rhs) const;

    public:
        bool greater_all(ir::node_data<double> const& lhs,
            ir::node_data<double> const& rhs) const;

    private:
        std::vector<primitive_argument_type> operands_;
    };
}}}

#endif


