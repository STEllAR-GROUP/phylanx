// Copyright (c) 2018-2019 Bita Hasheminezhad
// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_MATRIXOPS_FLIP_OPERATION)
#define PHYLANX_MATRIXOPS_FLIP_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>
#include <hpx/util/optional.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {
    /// \brief Reverses the order of elements in an array along the given axis
    /// \param a         The scalar, vector, matrix or tensor to flip
    /// \param axes      Optional. Flip is calculated along the provided axes
    ///                  and an array of the same shape is returned. If None,
    ///                  all axes will be reversed.
    class flip_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<flip_operation>
    {
    public:
        enum flip_mode
        {
            flip_mode_up_down,       // flipud
            flip_mode_left_right,    // fliplr
            flip_mode_axes
        };

    protected:
        using val_type = std::int64_t;
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static std::vector<match_pattern_type> const match_data;

        flip_operation() = default;

        flip_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        template <typename T>
        primitive_argument_type flip1d(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type flip1d(ir::node_data<T>&& arg,
            ir::range&& axes) const;

        template <typename T>
        primitive_argument_type flip2d_axis0(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type flip2d_axis1(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type flip2d_both_axes(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type flip2d(ir::node_data<T>&& arg,
            ir::range&& axes) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type flip3d_axis0(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type flip3d_axis1(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type flip3d_axis2(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type flip3d_axes_0_1(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type flip3d_axes_0_2(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type flip3d_axes_1_2(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type flip3d_all_axes(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type flip3d(ir::node_data<T>&& arg,
            ir::range&& axes) const;
#endif
        template <typename T>
        primitive_argument_type flipnd(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type flipnd(ir::node_data<T>&& arg,
            ir::range&& axes) const;
        primitive_argument_type flipnd_helper(
            primitive_argument_type&& arg) const;

        template <typename T>
        primitive_argument_type flipud(ir::node_data<T>&& arg) const;
        primitive_argument_type flipud_helper(
            primitive_argument_type&& arg) const;

        template <typename T>
        primitive_argument_type fliplr(ir::node_data<T>&& arg) const;
        primitive_argument_type fliplr_helper(
            primitive_argument_type&& arg) const;

    private:
        flip_mode mode_;
    };

    inline primitive create_flipud_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "flipud", std::move(operands), name, codename);
    }

    inline primitive create_fliplr_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "fliplr", std::move(operands), name, codename);
    }

    inline primitive create_flip_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "flip", std::move(operands), name, codename);
    }
}}}

#endif
