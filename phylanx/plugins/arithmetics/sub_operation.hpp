// Copyright (c) 2017 Bibek Wagle
// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Parsa Amini
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_SUB_OPERATION_SEP_15_2017_1035AM)
#define PHYLANX_PRIMITIVES_SUB_OPERATION_SEP_15_2017_1035AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class sub_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<sub_operation>
    {
    protected:
        using arg_type = ir::node_data<double>;
        using args_type = std::vector<arg_type>;

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

    public:
        static match_pattern_type const match_data;

        sub_operation() = default;

        sub_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;

    private:
        enum struct stretch_operand { neither, lhs, rhs };

        primitive_argument_type sub0d0d(args_type && ops) const;
        primitive_argument_type sub0d0d(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type sub0d1d(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type sub0d2d(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type sub0d(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type sub0d(args_type && ops) const;

        primitive_argument_type sub1d0d(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type sub1d1d(args_type&& ops) const;
        primitive_argument_type sub1d1d(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type sub1d2d(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type sub1d(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type sub1d(args_type && ops) const;

        primitive_argument_type sub2d0d(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type sub2d1d(arg_type&& lhs, arg_type&& rhs) const;
        stretch_operand get_stretch_dimension(
            std::size_t lhs, std::size_t rhs) const;
        primitive_argument_type sub2d2d_no_stretch(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type sub2d2d_lhs_both(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type sub2d2d_rhs_both(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type sub2d2d_lhs_row_rhs_col(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type sub2d2d_lhs_row(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type sub2d2d_lhs_col_rhs_row(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type sub2d2d_rhs_row(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type sub2d2d_lhs_col(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type sub2d2d_rhs_col(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type sub2d2d(args_type&& ops) const;
        primitive_argument_type sub2d2d(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type sub2d(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type sub2d(args_type && ops) const;
    };

    inline primitive create_sub_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "__sub", std::move(operands), name, codename);
    }
}}}

#endif
