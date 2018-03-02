// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_GREATER_OCT_07_2017_0223PM)
#define PHYLANX_PRIMITIVES_GREATER_OCT_07_2017_0223PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class greater
        : public primitive_component_base
        , public std::enable_shared_from_this<greater>
    {
    protected:
        using operand_type = ir::node_data<double>;
        using operands_type = std::vector<primitive_argument_type>;

        hpx::future<primitive_argument_type> greater::eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

    public:
        static match_pattern_type const match_data;

        greater() = default;

        greater(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;

    private:
        struct visit_greater;

        primitive_argument_type greater0d1d(
            operand_type&& lhs, operand_type&& rhs) const;
            primitive_argument_type greater0d2d(
                operand_type&& lhs, operand_type&& rhs) const;
            primitive_argument_type greater0d(
                operand_type&& lhs, operand_type&& rhs) const;
            primitive_argument_type greater1d0d(
                operand_type&& lhs, operand_type&& rhs) const;
            primitive_argument_type greater1d1d(
                operand_type&& lhs, operand_type&& rhs) const;
            primitive_argument_type greater1d2d(
                operand_type&& lhs, operand_type&& rhs) const;
            primitive_argument_type greater1d(
                operand_type&& lhs, operand_type&& rhs) const;
            primitive_argument_type greater2d0d(
                operand_type&& lhs, operand_type&& rhs) const;
            primitive_argument_type greater2d1d(
                operand_type&& lhs, operand_type&& rhs) const;
            primitive_argument_type greater2d2d(
                operand_type&& lhs, operand_type&& rhs) const;
            primitive_argument_type greater2d(
                operand_type&& lhs, operand_type&& rhs) const;
            primitive_argument_type greater_all(
                operand_type&& lhs, operand_type&& rhs) const;
    };

    PHYLANX_EXPORT primitive create_greater(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif


