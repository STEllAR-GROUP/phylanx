// Copyright (c) 2018 Bibek Wagle
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_STACK_OPERATION_DEC_04_2018_1041AM)
#define PHYLANX_STACK_OPERATION_DEC_04_2018_1041AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
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
    class stack_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<stack_operation>
    {
    public:
        enum stacking_mode
        {
            stacking_mode_column_wise,  // hstack
            stacking_mode_row_wise,     // vstack
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
            stacking_mode_depth_wise,   // dstack
#endif
            stacking_mode_axis
        };

    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static std::vector<match_pattern_type> const match_data;

        stack_operation() = default;

        stack_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:

        template <typename T>
        hpx::future<primitive_argument_type> empty_helper() const;
        hpx::future<primitive_argument_type> empty() const;

        std::size_t get_vecsize(
            primitive_arguments_type const& args) const;

        primitive_argument_type stack0d(
            primitive_arguments_type&& args) const;
        primitive_argument_type stack1d(
            primitive_arguments_type&& args) const;
        primitive_argument_type stack2d(
            primitive_arguments_type&& args) const;
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        primitive_argument_type stack3d(
            primitive_arguments_type&& args) const;
#endif

        // support for hstack
        template <typename T>
        primitive_argument_type hstack0d1d_helper(
            primitive_arguments_type&& args) const;
        primitive_argument_type hstack0d1d(
            primitive_arguments_type&& args) const;

        template <typename T>
        primitive_argument_type hstack2d_helper(
            primitive_arguments_type&& args) const;
        primitive_argument_type hstack2d(
            primitive_arguments_type&& args) const;

        // support for vstack
        template <typename T>
        primitive_argument_type vstack0d_helper(
            primitive_arguments_type&& args) const;
        primitive_argument_type vstack0d(
            primitive_arguments_type&& args) const;

        template <typename T>
        primitive_argument_type vstack1d2d_helper(
            primitive_arguments_type&& args) const;
        primitive_argument_type vstack1d2d(
            primitive_arguments_type&& args) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type hstack3d_helper(
            primitive_arguments_type&& args) const;
        primitive_argument_type hstack3d(
            primitive_arguments_type&& args) const;

        template <typename T>
        primitive_argument_type vstack3d_helper(
            primitive_arguments_type&& args) const;
        primitive_argument_type vstack3d(
            primitive_arguments_type&& args) const;

        // support for dstack
        template <typename T>
        primitive_argument_type dstack0d_helper(
            primitive_arguments_type&& args) const;
        primitive_argument_type dstack0d(
            primitive_arguments_type&& args) const;

        template <typename T>
        primitive_argument_type dstack1d_helper(
            primitive_arguments_type&& args) const;
        primitive_argument_type dstack1d(
            primitive_arguments_type&& args) const;

        template <typename T>
        primitive_argument_type dstack2d3d_helper(
            primitive_arguments_type&& args) const;
        primitive_argument_type dstack2d3d(
            primitive_arguments_type&& args) const;
#endif

    private:
        node_data_type dtype_;
        stacking_mode mode_;
    };

    ///////////////////////////////////////////////////////////////////////////
    inline primitive create_hstack_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "hstack", std::move(operands), name, codename);
    }

    inline primitive create_vstack_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "vstack", std::move(operands), name, codename);
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    inline primitive create_dstack_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "dstack", std::move(operands), name, codename);
    }
#endif

    inline primitive create_stack_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "stack", std::move(operands), name, codename);
    }
}}}


#endif //PHYLANX_HSTACK_OPERATION_JAN13_1200_2018_H
