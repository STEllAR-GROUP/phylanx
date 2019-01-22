// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_MATRIXOPS_REPEAT_OPERATION)
#define PHYLANX_MATRIXOPS_REPEAT_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>
#include <hpx/util/optional.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {
/// \brief Returns repeated array which has the same shape as a, except
///        along the given axis.
/// \param a         The scalar, vector, or matrix to repeat
/// \param repeats   The number of repetitions for each element. repeats is
///                  broadcasted to fit the shape of the given axis.
/// \param axis      Optional. If provided, repeat is calculated along the
///                  provided axis.

    class repeat_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<repeat_operation>
    {
    protected:
        using val_type = std::int64_t;
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        repeat_operation() = default;

        repeat_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        bool validate_repetition(ir::node_data<val_type>&& rep) const;

        template <typename T>
        primitive_argument_type repeat0d0d(ir::node_data<T>&& arg,
            ir::node_data<val_type>&& rep) const;
        template <typename T>
        primitive_argument_type repeat0d1d(ir::node_data<T>&& arg,
            ir::node_data<val_type>&& rep) const;
        template <typename T>
        primitive_argument_type repeat0d(ir::node_data<T>&& arg,
            ir::node_data<val_type>&& rep,
            hpx::util::optional<val_type> axis) const;

        template <typename T>
        primitive_argument_type repeat1d0d(ir::node_data<T>&& arg,
            val_type&& rep) const;
        template <typename T>
        primitive_argument_type repeat1d1d(ir::node_data<T>&& arg,
            ir::node_data<val_type>&& rep) const;
        template <typename T>
        primitive_argument_type repeat1d(ir::node_data<T>&& arg,
            ir::node_data<val_type>&& rep,
            hpx::util::optional<val_type> axis) const;

        template <typename T>
        primitive_argument_type repeat2d0d_axis0(ir::node_data<T>&& arg,
            val_type&& rep) const;
        template <typename T>
        primitive_argument_type repeat2d1d_axis0(ir::node_data<T>&& arg,
            ir::node_data<val_type>&& rep) const;
        template <typename T>
        primitive_argument_type repeat2d_axis0(ir::node_data<T>&& arg,
            ir::node_data<val_type>&& rep) const;
        template <typename T>
        primitive_argument_type repeat2d0d_axis1(ir::node_data<T>&& arg,
            val_type&& rep) const;
        template <typename T>
        primitive_argument_type repeat2d1d_axis1(ir::node_data<T>&& arg,
            ir::node_data<val_type>&& rep) const;
        template <typename T>
        primitive_argument_type repeat2d_axis1(ir::node_data<T>&& arg,
            ir::node_data<val_type>&& rep) const;
        template <typename T>
        primitive_argument_type repeat2d0d_flatten(ir::node_data<T>&& arg,
            val_type&& rep) const;
        template <typename T>
        primitive_argument_type repeat2d1d_flatten(ir::node_data<T>&& arg,
            ir::node_data<val_type>&& rep) const;
        template <typename T>
        primitive_argument_type repeat2d(ir::node_data<T>&& arg,
            ir::node_data<val_type>&& rep,
            hpx::util::optional<val_type> axis) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type repeat3d0d_axis0(ir::node_data<T>&& arg,
            val_type&& rep) const;
        template <typename T>
        primitive_argument_type repeat3d1d_axis0(ir::node_data<T>&& arg,
            ir::node_data<val_type>&& rep) const;
        template <typename T>
        primitive_argument_type repeat3d_axis0(ir::node_data<T>&& arg,
            ir::node_data<val_type>&& rep) const;

        template <typename T>
        primitive_argument_type repeat3d0d_axis1(ir::node_data<T>&& arg,
            val_type&& rep) const;
        template <typename T>
        primitive_argument_type repeat3d1d_axis1(ir::node_data<T>&& arg,
            ir::node_data<val_type>&& rep) const;
        template <typename T>
        primitive_argument_type repeat3d_axis1(ir::node_data<T>&& arg,
            ir::node_data<val_type>&& rep) const;

        template <typename T>
        primitive_argument_type repeat3d0d_axis2(ir::node_data<T>&& arg,
            val_type&& rep) const;
        template <typename T>
        primitive_argument_type repeat3d1d_axis2(ir::node_data<T>&& arg,
            ir::node_data<val_type>&& rep) const;
        template <typename T>
        primitive_argument_type repeat3d_axis2(ir::node_data<T>&& arg,
            ir::node_data<val_type>&& rep) const;

        template <typename T>
        primitive_argument_type repeat3d0d_flatten(ir::node_data<T>&& arg,
            val_type&& rep) const;
        template <typename T>
        primitive_argument_type repeat3d1d_flatten(ir::node_data<T>&& arg,
            ir::node_data<val_type>&& rep) const;

        template <typename T>
        primitive_argument_type repeat3d(ir::node_data<T>&& arg,
            ir::node_data<val_type>&& rep,
            hpx::util::optional<val_type> axis) const;
#endif

        template <typename T>
        primitive_argument_type repeatnd(ir::node_data<T>&& arg,
            ir::node_data<val_type>&& rep,
            hpx::util::optional<val_type> axis) const;
    };

    inline primitive create_repeat_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "repeat", std::move(operands), name, codename);
    }
}}}

#endif
