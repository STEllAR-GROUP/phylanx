// Copyright (c) 2017-2018 Hartmut Kaiser
// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Parsa Amini
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ADD_OPERATION_SEP_05_2017_1202PM)
#define PHYLANX_PRIMITIVES_ADD_OPERATION_SEP_05_2017_1202PM

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
    class add_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<add_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args) const;

        using arg_type = ir::node_data<double>;
        using args_type = std::vector<arg_type, arguments_allocator<arg_type>>;

    public:
        static match_pattern_type const match_data;

        add_operation() = default;

        add_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& args,
            eval_mode) const override;

    private:
        enum struct stretch_operand { neither, lhs, rhs };

        primitive_argument_type add0d0d(arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type add0d0d(args_type && args) const;
        primitive_argument_type add0d1d(arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type add0d2d(arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type add0d(arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type add0d(args_type && args) const;

        primitive_argument_type add1d0d(arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type add1d1d(arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type add1d1d(args_type && args) const;
        primitive_argument_type add1d2d(arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type add1d(arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type add1d(args_type && args) const;

        primitive_argument_type add2d0d(arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type add2d1d(arg_type&& lhs, arg_type&& rhs) const;
        stretch_operand get_stretch_dimension(
            std::size_t lhs, std::size_t rhs) const;
        primitive_argument_type add2d2d_no_stretch(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type add2d2d_lhs_both(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type add2d2d_rhs_both(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type add2d2d_lhs_row_rhs_col(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type add2d2d_lhs_row(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type add2d2d_lhs_col_rhs_row(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type add2d2d_rhs_row(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type add2d2d_lhs_col(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type add2d2d_rhs_col(
            arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type add2d2d(arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type add2d2d(args_type && args) const;
        primitive_argument_type add2d(arg_type&& lhs, arg_type&& rhs) const;
        primitive_argument_type add2d(args_type && args) const;

        primitive_argument_type handle_list_operands(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;
        primitive_argument_type handle_numeric_operands(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;

        primitive_argument_type handle_list_operands(
            primitive_arguments_type&& ops) const;
        primitive_argument_type handle_numeric_operands(
            primitive_arguments_type&& ops) const;

        void append_element(primitive_arguments_type& result,
            primitive_argument_type&& rhs) const;
    };

    inline primitive create_add_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "__add", std::move(operands), name, codename);
    }
}}}

#endif
