// Copyright (c) 2019 Shahrzad Shirzad
// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_CONCATENATE)
#define PHYLANX_PRIMITIVES_CONCATENATE

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {
    /// \brief Implementation of concatenate as a Phylanx primitive.
    /// Returns the joined sequence of arrays along an existing axis.
    /// This implementation is intended to behave like [NumPy implementation of concatenate]
    /// (https://docs.scipy.org/doc/numpy-1.15.0/reference/generated/numpy.concatenate.html).
    /// \param a1, a2, ... is sequence of array_like
    /// \param axis The axis along which the arrays will be joined
    class concatenate
      : public primitive_component_base
      , public std::enable_shared_from_this<concatenate>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type&& args, eval_context ctx) const override;

        hpx::future<primitive_argument_type> handle_concatenate(
            primitive_arguments_type const& operands,
            primitive_argument_type const& axis, primitive_arguments_type&& args,
            eval_context ctx) const;

    public:
        static match_pattern_type const match_data;

        concatenate() = default;

        concatenate(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type concatenate1d(
            primitive_arguments_type&& args, std::int64_t axis) const;
        primitive_argument_type concatenate2d(
            primitive_arguments_type&& args, std::int64_t axis) const;
        primitive_argument_type concatenate3d(
            primitive_arguments_type&& args, std::int64_t axis) const;
        primitive_argument_type concatenate_flatten(
            primitive_arguments_type&& args) const;

        template <typename T>
        primitive_argument_type concatenate_flatten_helper(
            primitive_arguments_type&& args) const;
        template <typename T>
        primitive_argument_type concatenate_flatten1d(
            primitive_arguments_type&& args) const;
        template <typename T>
        primitive_argument_type concatenate1d_helper(
            primitive_arguments_type&& args) const;

        template <typename T>
        primitive_argument_type concatenate_flatten2d(
            primitive_arguments_type&& args) const;
        template <typename T>
        primitive_argument_type concatenate2d_axis0(
            primitive_arguments_type&& args) const;
        template <typename T>
        primitive_argument_type concatenate2d_axis1(
            primitive_arguments_type&& args) const;
        template <typename T>
        primitive_argument_type concatenate2d_helper(
            primitive_arguments_type&& args, std::int64_t axis) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type concatenate_flatten3d(
            primitive_arguments_type&& args) const;
        template <typename T>
        primitive_argument_type concatenate3d_helper(
            primitive_arguments_type&& args, std::int64_t axis) const;
        template <typename T>
        primitive_argument_type concatenate3d_axis0(
            primitive_arguments_type&& args) const;
        template <typename T>
        primitive_argument_type concatenate3d_axis1(
            primitive_arguments_type&& args) const;
        template <typename T>
        primitive_argument_type concatenate3d_axis2(
            primitive_arguments_type&& args) const;
#endif
        std::size_t get_vec_size(primitive_arguments_type const& args) const;
        std::size_t get_matrix_size(primitive_arguments_type const& args) const;
        std::size_t get_tensor_size(primitive_arguments_type const& args) const;
    };

    inline primitive create_concatenate(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "concatenate", std::move(operands), name, codename);
    }
}}}

#endif
