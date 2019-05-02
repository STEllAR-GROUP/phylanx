// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_KERAS_SUPPORT_SEPARABLE_separable_conv1d_operation)
#define PHYLANX_KERAS_SUPPORT_SEPARABLE_separable_conv1d_operation

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>
#include <hpx/util/optional.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {
/// \brief returns 1D separable convoltion
/// \param x                 a stream of 3d data: batch, input_length,
///                          in_channels
/// \param depthwise_kernel  a 3d filter: filter_length, depth_in_channels,
///                          depth_out_channels
/// \param pointwise_kernel  a 3d filter: 1, point_in_channels,
///                          point_out_channels
/// \param padding           Padding mode, either `valid` or`same`
/// \param strides           The step to apply convolution
/// \param dilation_rate     The rate to sample x in each step

    class separable_conv1d_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<separable_conv1d_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        separable_conv1d_operation() = default;

        separable_conv1d_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type sep_conv1d_valid(ir::node_data<double>&& arg,
            ir::node_data<double>&& depth_kernel,
            ir::node_data<double>&& point_kernel) const;

        primitive_argument_type sep_conv1d_any_pad(ir::node_data<double>&& arg,
            ir::node_data<double>&& depth_kernel,
            ir::node_data<double>&& point_kernel, std::string&& padding) const;
        //primitive_argument_type conv1d_any_pad(ir::node_data<double>&& arg,
        //    ir::node_data<double>&& kernel, std::string&& padding,
        //    std::int64_t strides) const;
        //primitive_argument_type conv1d_any_pad_dilation(
        //    ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        //    std::string&& padding, std::int64_t dilation_rate) const;
    };

    inline primitive create_separable_conv1d_operation(
        hpx::id_type const& locality, primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "separable_conv1d", std::move(operands), name, codename);
    }
}}}

#endif
