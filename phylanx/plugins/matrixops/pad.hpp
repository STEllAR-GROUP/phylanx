// Copyright (c) 2019 Shahrzad Shirzad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_PAD)
#define PHYLANX_PRIMITIVES_PAD

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
    /// \brief Implementation of pad as a Phylanx primitive.
    /// Given an interval, values outside the interval are padped to the interval edges.
    /// This implementation is intended to behave like [NumPy implementation of pad]
    /// (https://docs.scipy.org/doc/numpy-1.15.0/reference/generated/numpy.pad.html).
    /// \param a It may be a scalar value, vector, or matrix
    /// \param mode Is a string, in case of 'constant' pads with a constant value
    /// \param constant_values(optional) Is an int, the values to set the padded values

    class pad
      : public primitive_component_base
      , public std::enable_shared_from_this<pad>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        pad() = default;

        pad(primitive_arguments_type&& operands, std::string const& name,
            std::string const& codename);

    private:
        primitive_argument_type get_array(
            primitive_arguments_type&& args, std::size_t&& ndim) const;
        primitive_argument_type get_array(
            primitive_arguments_type& args, std::size_t&& ndim) const;

        template <typename T>
        primitive_argument_type pad_1d(ir::node_data<T>&& arg,
            ir::node_data<std::int64_t>&& width, T&& value) const;
        template <typename T>
        primitive_argument_type pad_2d(ir::node_data<T>&& arg,
            ir::node_data<std::int64_t>&& width, T&& value) const;
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type pad_3d(ir::node_data<T>&& arg,
            ir::node_data<std::int64_t>&& width, T&& value) const;
#endif
        template <typename T>
        primitive_argument_type pad_helper(ir::node_data<T>&& arg,
            ir::node_data<std::int64_t>&& width,
            ir::node_data<T>&& values) const;
    };

    inline primitive create_pad(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "pad", std::move(operands), name, codename);
    }
}}}

#endif
