// Copyright (c) 2019 Shahrzad Shirzad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_RELU)
#define PHYLANX_PRIMITIVES_RELU

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

namespace phylanx { namespace execution_tree { namespace primitives
{
    /// \brief Implementation of relu as a Phylanx primitive.
    /// Given an array, values greater than a threshold are clipped between 0 and a
    /// given maximum value, and values smaller than the threshold are mapped into
    /// a constant alpha times the difference between the value and the threshold.
    /// \param x It may be a scalar value, vector, or matrix
    /// \param alpha Slope of negative region, scalar default is 0.0
    /// \param max_value Saturation threshold, scalar
    /// \param threshold Threshold for thresholded activations, scalar default is 0.0

    class relu_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<relu_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        relu_operation() = default;

        relu_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        template <typename T>
        primitive_argument_type relu0d(ir::node_data<T>&& arg, double alpha,
            T max_value, double threshold) const;

        template <typename T>
        primitive_argument_type relu1d(ir::node_data<T>&& arg, double alpha,
            T max_value, double threshold) const;

        template <typename T>
        primitive_argument_type relu2d(ir::node_data<T>&& arg, double alpha,
            T max_value, double threshold) const;
        template <typename T>
        primitive_argument_type relu3d(ir::node_data<T>&& arg, double alpha,
            T max_value, double threshold) const;

        template <typename T>
        primitive_argument_type relu_helper(ir::node_data<T>&& arg,
            double alpha, T max_value, double threshold) const;
    };

    inline primitive create_relu_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "relu", std::move(operands), name, codename);
    }
}}}

#endif
