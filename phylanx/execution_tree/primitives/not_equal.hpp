// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_NOT_EQUAL_OCT_07_2017_0212PM)
#define PHYLANX_PRIMITIVES_NOT_EQUAL_OCT_07_2017_0212PM

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
    class not_equal
        : public primitive_component_base
        , public std::enable_shared_from_this<not_equal>
    {
    protected:
        using operand_type = ir::node_data<double>;
        using operands_type = std::vector<primitive_argument_type>;

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

    public:
        static match_pattern_type const match_data;

        not_equal() = default;

        not_equal(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;

    private:
        struct visit_not_equal;

        primitive_argument_type not_equal0d1d(
            operand_type&& lhs, operand_type&& rhs) const;
        primitive_argument_type not_equal0d2d(
            operand_type&& lhs, operand_type&& rhs) const;
        primitive_argument_type not_equal0d(
            operand_type&& lhs, operand_type&& rhs) const;
        primitive_argument_type not_equal1d0d(
            operand_type&& lhs, operand_type&& rhs) const;
        primitive_argument_type not_equal1d1d(
            operand_type&& lhs, operand_type&& rhs) const;
        primitive_argument_type not_equal1d2d(
            operand_type&& lhs, operand_type&& rhs) const;
        primitive_argument_type not_equal1d(
            operand_type&& lhs, operand_type&& rhs) const;
        primitive_argument_type not_equal2d0d(
            operand_type&& lhs, operand_type&& rhs) const;
        primitive_argument_type not_equal2d1d(
            operand_type&& lhs, operand_type&& rhs) const;
        primitive_argument_type not_equal2d2d(
            operand_type&& lhs, operand_type&& rhs) const;
        primitive_argument_type not_equal2d(
            operand_type&& lhs, operand_type&& rhs) const;
        primitive_argument_type not_equal_all(
            operand_type&& lhs, operand_type&& rhs) const;
    };

    PHYLANX_EXPORT primitive create_not_equal(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif


