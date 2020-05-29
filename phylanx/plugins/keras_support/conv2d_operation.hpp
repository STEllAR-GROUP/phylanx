// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_KERAS_SUPPORT_CONV2D_OPERATION)
#define PHYLANX_KERAS_SUPPORT_CONV2D_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/datastructures/optional.hpp>
#include <hpx/futures/future.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {
/// \brief returns 2D convoltion
/// \param x              a quatern (4d array)
/// \param kernel         a quatern, the filter
/// \param padding        Padding mode, either `valid` or `same`
/// \param strides        The step to apply convolution on height and width
/// \param dilation_rate  The rate to sample x in each step for height and
///                       width

    class conv2d_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<conv2d_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        conv2d_operation() = default;

        conv2d_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type conv2d_valid(ir::node_data<double>&& arg,
            ir::node_data<double>&& kernel) const;
        primitive_argument_type conv2d_valid(ir::node_data<double>&& arg,
            ir::node_data<double>&& kernel, std::int64_t stride_height,
            std::int64_t stride_width) const;
        primitive_argument_type conv2d_valid_dilation(
            ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
            std::int64_t dilation_height, std::int64_t dilation_width) const;

        primitive_argument_type conv2d_same(ir::node_data<double>&& arg,
            ir::node_data<double>&& kernel) const;
        primitive_argument_type conv2d_same(ir::node_data<double>&& arg,
            ir::node_data<double>&& kernel, std::int64_t stride_height,
            std::int64_t stride_width) const;
        primitive_argument_type conv2d_same_dilation(
            ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
            std::int64_t dilation_height, std::int64_t dilation_width) const;

        primitive_argument_type conv2d_any_pad(ir::node_data<double>&& arg,
            ir::node_data<double>&& kernel, std::string&& padding) const;
        primitive_argument_type conv2d_any_pad(ir::node_data<double>&& arg,
            ir::node_data<double>&& kernel, std::string&& padding,
            std::int64_t stride_height, std::int64_t stride_width) const;
        primitive_argument_type conv2d_any_pad_dilation(
            ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
            std::string&& padding, std::int64_t dilation_height,
            std::int64_t dilation_width) const;
    };

    inline primitive create_conv2d_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "conv2d", std::move(operands), name, codename);
    }
}}}

#endif
